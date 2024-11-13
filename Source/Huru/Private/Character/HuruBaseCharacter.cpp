// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/HuruBaseCharacter.h"
#include "Character/HuruCharacterMovementComponent.h"
#include "Character/Animation/HuruPlayerCameraBehavior.h"
#include "Character/Animation/HuruCharacterAnimInstance.h"
#include "Character/Component/HuruDebugComponent.h"
//Engine
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

const FName NAME_FP_Camera(TEXT("FP_Camera"));
const FName NAME_RotationAmount(TEXT("RotationAmount"));
const FName NAME_YawOffset(TEXT("YawOffset"));
const FName NAME_Pelvis(TEXT("Pelvis")); // Socket
const FName NAME_pelvis(TEXT("pelvis")); // Bone
const FName NAME_root(TEXT("root"));
const FName NAME_spine_05(TEXT("spine_05"));


AHuruBaseCharacter::AHuruBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UHuruCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = 0;
	bReplicates = true;
	SetReplicatingMovement(true);
}

void AHuruBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 네트워크 게임이면 곡선 이동 비활성화 (Standalone 모드가 아니면 최적화 활성화)
	bEnableNetworkOptimizations = !IsNetMode(NM_Standalone);

	// 캐릭터 블루프린트(CharacterBP) 후에 메쉬와 애니메이션 블루프린트(AnimBP)가 
	// 업데이트되도록 설정해서 최신 값을 받도록 보장
	GetMesh()->AddTickPrerequisiteActor(this);
	
	// 이동 모델 설정
	SetMovementModel();

	// 초기 원하는 값으로 상태를 강제로 업데이트
	ForceUpdateCharacterState();

	if (Stance == EHuruStance::Standing)
	{
		// 스탠스가 서 있는 상태면 쭈그린 자세를 풀어줌
		UnCrouch();
	}
	else if (Stance == EHuruStance::Crouching)
	{
		// 스탠스가 쭈그린 상태면 쭈그린 자세로 변경
		Crouch();
	}

	// 기본 회전 값을 설정
	TargetRotation = GetActorRotation();
	// 마지막 속도 회전값을 타겟 회전값으로 설정
	LastVelocityRotation = TargetRotation;
	// 마지막 이동 입력 회전값을 타겟 회전값으로 설정
	LastMovementInputRotation = TargetRotation;

	if (GetMesh()->GetAnimInstance() && GetLocalRole() == ROLE_SimulatedProxy)
	{
		// 메쉬에 애니메이션 인스턴스가 있고 로컬 역할이 Simulated Proxy라면, 
		// 루트 모션을 무시하도록 설정
		GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	// 캐릭터 이동 설정을 목표 이동 설정 값으로 설정
	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());

	HuruDebugComponent = FindComponentByClass<UHuruDebugComponent>();
}

void AHuruBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 필요한 값들을 설정함
	SetEssentialValues(DeltaTime);

	if (MovementState == EHuruMovementState::Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation(DeltaTime);
	}
	else if (MovementState == EHuruMovementState::InAir)
	{
		UpdateInAirRotation(DeltaTime);
	}
	else if (MovementState == EHuruMovementState::Ragdoll)
	{
		RagdollUpdate(DeltaTime);
	}

	// 반복적으로 사용될 값을 캐싱
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = AimingRotation.Yaw;
}

void AHuruBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MyCharacterMovementComponent = Cast<UHuruCharacterMovementComponent>(Super::GetMovementComponent());
}

void AHuruBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHuruBaseCharacter, TargetRagdollLocation);
	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, ReplicatedCurrentAcceleration, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, ReplicatedControlRotation, COND_SkipOwner);

	DOREPLIFETIME(AHuruBaseCharacter, DesiredGait);
	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, DesiredStance, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, DesiredRotationMode, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, RotationMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, OverlayState, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHuruBaseCharacter, ViewMode, COND_SkipOwner);
}

void AHuruBaseCharacter::RagdollStart()
{
	// 델리게이트를 통해 랙돌 상태 변경 알림
	if (RagdollStateChangedDelegate.IsBound())
	{
		RagdollStateChangedDelegate.Broadcast(true);
	}

	// 네트워크 상황에서 클라이언트의 움직임 오류 체크 비활성화
	MyCharacterMovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = 1;

	// 전용 서버에서의 메쉬 애니메이션 최적화 설정
	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		DefVisBasedTickOp = GetMesh()->VisibilityBasedAnimTickOption;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	// 타겟 위치 설정
	TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
	ServerRagdollPull = 0;

	// 업데이트 속도 최적화 비활성화
	bPreRagdollURO = GetMesh()->bEnableUpdateRateOptimizations;
	GetMesh()->bEnableUpdateRateOptimizations = false;

	// 움직임 모드 및 상태 설정
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	SetMovementState(EHuruMovementState::Ragdoll);

	// 콜리전 설정
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_Pelvis, true, true);

	// 활성 애니메이션 멈추기
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
	}
	
	// 자율 틱 포즈 활성화
	GetMesh()->bOnlyAllowAutonomousTickPose = true;

	// 이동 복제 비활성화
	SetReplicateMovement(false);
}

void AHuruBaseCharacter::Server_SetMeshLocationDuringRagdoll_Implementation(FVector MeshLocation)
{
	TargetRagdollLocation = MeshLocation;
}

void AHuruBaseCharacter::SetActorLocationAndTargetRotation(FVector NewLocation, FRotator NewRotation)
{
	SetActorLocationAndRotation(NewLocation, NewRotation);
	TargetRotation = NewRotation;
}

void AHuruBaseCharacter::SetMovementState(EHuruMovementState NewState, bool bForce)
{
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void AHuruBaseCharacter::SetMovementAction(EHuruMovementAction NewAction, bool bForce)
{
	if (bForce || MovementAction != NewAction)
	{
		const EHuruMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void AHuruBaseCharacter::SetStance(EHuruStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const EHuruStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void AHuruBaseCharacter::SetGait(EHuruGait NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		const EHuruGait Prev = Gait;
		Gait = NewGait;
		OnGaitChanged(Prev);
	}
}

void AHuruBaseCharacter::SetRotationMode(EHuruRotationMode NewRotationMode, bool bForce)
{

	if (bForce || RotationMode != NewRotationMode)
	{
		const EHuruRotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode, bForce);
		}
	}
}

void AHuruBaseCharacter::Server_SetRotationMode_Implementation(EHuruRotationMode NewRotationMode, bool bForce)
{
	SetRotationMode(NewRotationMode, bForce);
}

void AHuruBaseCharacter::SetDesiredStance(EHuruStance NewStance)
{
	// 캐릭터의 원하는 자세(stance)를 설정
	DesiredStance = NewStance;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredStance(NewStance);
	}
}

void AHuruBaseCharacter::Server_SetDesiredStance_Implementation(EHuruStance NewStance)
{
	// 서버에서 실행되는 SetDesiredStance 함수의 실제 구현
	SetDesiredStance(NewStance);
}

void AHuruBaseCharacter::SetDesiredGait(const EHuruGait NewGait)
{
	// 캐릭터의 원하는 보행 방식(gait)을 설정
	DesiredGait = NewGait;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredGait(NewGait);
	}
}

void AHuruBaseCharacter::SetViewMode(EHuruViewMode NewViewMode, bool bForce)
{
	if (bForce || ViewMode != NewViewMode)
	{
		const EHuruViewMode Prev = ViewMode;
		ViewMode = NewViewMode;
		OnViewModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetViewMode(NewViewMode, bForce);
		}
	}
}

void AHuruBaseCharacter::Server_SetViewMode_Implementation(EHuruViewMode NewViewMode, bool bForce)
{
	SetViewMode(NewViewMode, bForce);
}

void AHuruBaseCharacter::Server_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	ForceNetUpdate();
	Multicast_PlayMontage(Montage, PlayRate);
}

void AHuruBaseCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (GetMesh()->GetAnimInstance() && !IsLocallyControlled())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}
}

void AHuruBaseCharacter::ReplicatedRagdollStart()
{
	if (HasAuthority())
	{
		Multicast_RagdollStart();
	}
	else
	{
		Server_RagdollStart();
	}
}

void AHuruBaseCharacter::Server_RagdollStart_Implementation()
{
	Multicast_RagdollStart();
}

void AHuruBaseCharacter::Multicast_RagdollStart_Implementation()
{
	RagdollStart();
}

FHuruMovementSettings AHuruBaseCharacter::GetTargetMovementSettings() const
{
	if (RotationMode == EHuruRotationMode::VelocityDirection)
	{
		if (Stance == EHuruStance::Standing)
		{
			return MovementData.VelocityDirection.Standing;
		}
		if (Stance == EHuruStance::Crouching)
		{
			return MovementData.VelocityDirection.Crouching;
		}
	}
	else if (RotationMode == EHuruRotationMode::LookingDirection)
	{
		if (Stance == EHuruStance::Standing)
		{
			return MovementData.LookingDirection.Standing;
		}
		if (Stance == EHuruStance::Crouching)
		{
			return MovementData.LookingDirection.Crouching;
		}
	}
	else if (RotationMode == EHuruRotationMode::Aiming)
	{
		if (Stance == EHuruStance::Standing)
		{
			return MovementData.Aiming.Standing;
		}
		if (Stance == EHuruStance::Crouching)
		{
			return MovementData.Aiming.Crouching;
		}
	}

	// 속도 방향 서있는 상태 기본값
	return MovementData.VelocityDirection.Standing;
}

EHuruGait AHuruBaseCharacter::GetAllowedGait() const
{
	// 허용된 보행 방식을 계산. 이는 캐릭터가 현재 들어갈 수 있는 최대 보행 방식을 나타내며, 
	// 원하는 보행 방식, 회전 모드, 자세 등에 따라 결정될 수 있음. 예를 들어, 
	// 실내에서 캐릭터를 걷는 상태로 강제할 수 있음.

	if (Stance == EHuruStance::Standing)
	{
		if (RotationMode != EHuruRotationMode::Aiming)
		{
			if (DesiredGait == EHuruGait::Sprinting)
			{
				return CanSprint() ? EHuruGait::Sprinting : EHuruGait::Running;
			}
			return DesiredGait;
		}
	}

	// 무릎 자세와 조준 회전 모드는 동일한 동작을 가짐

	if (DesiredGait == EHuruGait::Sprinting)
	{
		return EHuruGait::Running;
	}

	return DesiredGait;
}

EHuruGait AHuruBaseCharacter::GetActualGait(EHuruGait AllowedGait)
{
	// 실제 보행 상태를 가져옴. 이는 캐릭터의 실제 움직임에 의해 계산되며,
	// 따라서 원하는 보행 상태나 허용된 보행 상태와 다를 수 있음.
	// 예를 들어, 허용된 보행 상태가 걷기로 변경되더라도,
	// 캐릭터가 걷는 속도로 감속할 때까지 실제 보행 상태는 여전히 달리기로 유지됨.

	const float LocWalkSpeed = MyCharacterMovementComponent->CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = MyCharacterMovementComponent->CurrentMovementSettings.RunSpeed;

	if (Speed > LocRunSpeed + 10.0f)
	{
		if (AllowedGait == EHuruGait::Sprinting)
		{
			return EHuruGait::Sprinting;
		}
		return EHuruGait::Running;
	}

	if (Speed >= LocWalkSpeed + 10.0f)
	{
		return EHuruGait::Running;
	}

	return EHuruGait::Walking;
}

bool AHuruBaseCharacter::CanSprint() const
{
	// 캐릭터가 현재 회전 모드와 현재 가속도(input) 회전을 기반으로 스프린트할 수 있는지 결정함.
	// 캐릭터가 조준 회전 모드에 있을 경우, 카메라를 기준으로 정면을 향하고 있으며
	// 모든 이동 입력이 있을 때만 스프린트를 허용함 (+ 또는 - 50도).

	if (!bHasMovementInput || RotationMode == EHuruRotationMode::Aiming)
	{
		return false;
	}

	const bool bValidInputAmount = MovementInputAmount > 0.9f;

	if (RotationMode == EHuruRotationMode::VelocityDirection)
	{
		return bValidInputAmount;
	}

	if (RotationMode == EHuruRotationMode::LookingDirection)
	{
		const FRotator AccRot = ReplicatedCurrentAcceleration.ToOrientationRotator();
		FRotator Delta = AccRot - AimingRotation;
		Delta.Normalize();

		return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
	}

	return false;
}

FVector AHuruBaseCharacter::GetMovementInput() const
{
	return ReplicatedCurrentAcceleration;
}

void AHuruBaseCharacter::OnBreakfall_Implementation()
{
	Replicated_PlayMontage(GetRollAnimation(), 1.35);
}

void AHuruBaseCharacter::Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	// Roll: Simply play a Root Motion Montage.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	Server_PlayMontage(Montage, PlayRate);
}

float AHuruBaseCharacter::GetAnimCurveValue(FName CurveName) const
{
	if (GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}

	return 0.0f;
}

void AHuruBaseCharacter::SetRightShoulder(bool bNewRightShoulder)
{
	bRightShoulder = bNewRightShoulder;
	if (CameraBehavior)
	{
		CameraBehavior->bRightShoulder = bRightShoulder;
	}
}

ECollisionChannel AHuruBaseCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	TraceOrigin = GetActorLocation();
	TraceRadius = 10.0f;
	return ECC_Visibility;
}

FTransform AHuruBaseCharacter::GetThirdPersonPivotTarget()
{
	return GetActorTransform();
}

FVector AHuruBaseCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(NAME_FP_Camera);
}

void AHuruBaseCharacter::GetCameraParameters(float& TPFOVOut, float& FPFOVOut, bool& bRightShoulderOut) const
{
	TPFOVOut = ThirdPersonFOV;
	FPFOVOut = FirstPersonFOV;
	bRightShoulderOut = bRightShoulder;
}

void AHuruBaseCharacter::OnOverlayStateChanged(const EHuruOverlayState PreviousState)
{
}

void AHuruBaseCharacter::SetOverlayOverrideState(int32 NewState)
{
	OverlayOverrideState = NewState;
}

void AHuruBaseCharacter::SetOverlayState(EHuruOverlayState NewState, bool bForce)
{
	if (bForce || OverlayState != NewState)
	{
		const EHuruOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetOverlayState(NewState, bForce);
		}
	}
}

void AHuruBaseCharacter::Server_SetOverlayState_Implementation(EHuruOverlayState NewState, bool bForce)
{
	SetOverlayState(NewState, bForce);
}

void AHuruBaseCharacter::SetGroundedEntryState(EHuruGroundedEntryState NewState)
{
	GroundedEntryState = NewState;
}

void AHuruBaseCharacter::EventOnLanded()
{
	const float VelZ = FMath::Abs(GetCharacterMovement()->Velocity.Z);

	if (bRagdollOnLand && VelZ > RagdollOnLandVelocity)
	{
		ReplicatedRagdollStart();
	}
	else if (bBreakfallOnLand && bHasMovementInput && VelZ >= BreakfallOnLandVelocity)
	{
		OnBreakfall();
	}
	else
	{
		GetCharacterMovement()->BrakingFrictionFactor = bHasMovementInput ? 0.5f : 3.0f;

		// 0.5초 후에 제동 마찰 계수를 0으로 초기화
		GetWorldTimerManager().SetTimer(OnLandedFrictionResetTimer, this,
										&AHuruBaseCharacter::OnLandFrictionReset, 0.5f, false);
	}
}

void AHuruBaseCharacter::Multicast_OnLanded_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnLanded();
	}
}

void AHuruBaseCharacter::EventOnJumped()
{
	// 속도가 100 이상일 경우, 새로운 공중 회전값을 속도 회전값으로 설정함
	InAirRotation = Speed > 100.0f ? LastVelocityRotation : GetActorRotation();

	OnJumpedDelegate.Broadcast();
}

void AHuruBaseCharacter::Multicast_OnJumped_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnJumped();
	}
}

void AHuruBaseCharacter::SetDesiredRotationMode(EHuruRotationMode NewRotMode)
{
	DesiredRotationMode = NewRotMode;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredRotationMode(NewRotMode);
	}
}

void AHuruBaseCharacter::Server_SetDesiredRotationMode_Implementation(EHuruRotationMode NewRotMode)
{
	SetDesiredRotationMode(NewRotMode);
}

void AHuruBaseCharacter::OnRep_RotationMode(EHuruRotationMode PrevRotMode)
{
	OnRotationModeChanged(PrevRotMode);
}

void AHuruBaseCharacter::OnRep_ViewMode(EHuruViewMode PrevViewMode)
{
	OnViewModeChanged(PrevViewMode);
}

void AHuruBaseCharacter::OnRep_OverlayState(EHuruOverlayState PrevOverlayState)
{
	OnOverlayStateChanged(PrevOverlayState);
}

void AHuruBaseCharacter::RagdollUpdate(float DeltaTime)
{
	GetMesh()->bOnlyAllowAutonomousTickPose = false;

	// 마지막 랙돌 속도를 설정함
	const FVector NewRagdollVel = GetMesh()->GetPhysicsLinearVelocity(NAME_root);
	LastRagdollVelocity = (NewRagdollVel != FVector::ZeroVector || IsLocallyControlled())
							  ? NewRagdollVel
							  : LastRagdollVelocity / 2;

	// 랙돌 속도를 사용하여 물리 애니메이션을 위한 랙돌 관절 강도를 스케일링함
	const float SpringValue = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 25000.0f},
																LastRagdollVelocity.Size());
	GetMesh()->SetAllMotorsAngularDriveParams(SpringValue, 0.0f, 0.0f, false);

	// -4000보다 빠르게 떨어지면 중력을 비활성화하여 지속적인 가속을 방지함
	// 랙돌이 바닥을 통과하는 것을 방지함.
	const bool bEnableGrav = LastRagdollVelocity.Z > -4000.0f;
	GetMesh()->SetEnableGravity(bEnableGrav);

	// 랙돌을 따라가기 위해 액터 위치를 업데이트함.
	SetActorLocationDuringRagdoll(DeltaTime);
}

void AHuruBaseCharacter::SetActorLocationDuringRagdoll(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		// 골반을 타겟 위치로 설정함.
		TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
		if (!HasAuthority())
		{
			Server_SetMeshLocationDuringRagdoll(TargetRagdollLocation);
		}
	}

	// 랙돌이 위를 향하고 있는지 아래를 향하고 있는지 판단하고, 그에 따라 타겟 회전을 설정함.
	const FRotator PelvisRot = GetMesh()->GetSocketRotation(NAME_Pelvis);

	if (bReversedPelvis) {
		bRagdollFaceUp = PelvisRot.Roll > 0.0f;
	} else
	{
		bRagdollFaceUp = PelvisRot.Roll < 0.0f;
	}


	const FRotator TargetRagdollRotation(0.0f, bRagdollFaceUp ? PelvisRot.Yaw - 180.0f : PelvisRot.Yaw, 0.0f);

	// 목표 위치에서 아래쪽으로 트레이스를 수행하여 목표 위치를 오프셋함.
	// 이는 캐릭터의 하단이 바닥을 통과하지 않도록 하여, 래그돌이 바닥에 누워 있을 때를 방지함.
	const FVector TraceVect(TargetRagdollLocation.X, TargetRagdollLocation.Y,
	                        TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceVect,
	                                                  ECC_Visibility, Params);

	if (HuruDebugComponent && HuruDebugComponent->GetShowTraces())
	{
		UHuruDebugComponent::DrawDebugLineTraceSingle(World,
													 TargetRagdollLocation,
													 TraceVect,
													 EDrawDebugTrace::Type::ForOneFrame,
													 bHit,
													 HitResult,
													 FLinearColor::Red,
													 FLinearColor::Green,
													 1.0f);
	}
	
	bRagdollOnGround = HitResult.IsValidBlockingHit();
	FVector NewRagdollLoc = TargetRagdollLocation;

	if (bRagdollOnGround)
	{
		const float ImpactDistZ = FMath::Abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z);
		NewRagdollLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ImpactDistZ + 2.0f;
	}
	if (!IsLocallyControlled())
	{
		ServerRagdollPull = FMath::FInterpTo(ServerRagdollPull, 750.0f, DeltaTime, 0.6f);
		float RagdollSpeed = FVector(LastRagdollVelocity.X, LastRagdollVelocity.Y, 0).Size();
		FName RagdollSocketPullName = RagdollSpeed > 300 ? NAME_spine_05 : NAME_pelvis;
		GetMesh()->AddForce(
			(TargetRagdollLocation - GetMesh()->GetSocketLocation(RagdollSocketPullName)) * ServerRagdollPull,
			RagdollSocketPullName, true);
	}
	SetActorLocationAndTargetRotation(bRagdollOnGround ? NewRagdollLoc : TargetRagdollLocation, TargetRagdollRotation);
}

void AHuruBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// 캐릭터 이동 모드 변화를 사용해 이동 상태를 올바른 값으로 설정함
	// 이는 사용자 정의 이동 상태를 가질 수 있게 하면서도 기본 캐릭터 이동 컴포넌트의 기능을 계속 사용할 수 있게 해줌

	if (GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking)
	{
		SetMovementState(EHuruMovementState::Grounded);
	}
 	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		SetMovementState(EHuruMovementState::InAir);
	}
}

void AHuruBaseCharacter::OnMovementStateChanged(EHuruMovementState PreviousState)
{
	if (MovementState == EHuruMovementState::InAir)
	{
		if (MovementAction == EHuruMovementAction::None)
		{
			// 캐릭터가 공중에 들어가면, 공중 회전을 설정하고, 만약 쭈그려 있으면 쭈그린 자세를 풀기
			InAirRotation = GetActorRotation();
			if (Stance == EHuruStance::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementAction == EHuruMovementAction::Rolling)
		{
			// 캐릭터가 현재 구르는 중이라면, 랙돌을 활성화
			ReplicatedRagdollStart();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementState = MovementState;
	}
}

void AHuruBaseCharacter::OnMovementActionChanged(EHuruMovementAction PreviousAction)
{
	// 캐릭터가 구르기를 수행할 때 쭈그리게 만든다
	if (MovementAction == EHuruMovementAction::Rolling)
	{
		Crouch();
	}

	if (PreviousAction == EHuruMovementAction::Rolling)
	{
		if (DesiredStance == EHuruStance::Standing)
		{
			UnCrouch();
		}
		else if (DesiredStance == EHuruStance::Crouching)
		{
			Crouch();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementAction = MovementAction;
	}
}

void AHuruBaseCharacter::OnStanceChanged(EHuruStance PreviousStance)
{
	if (CameraBehavior)
	{
		CameraBehavior->Stance = Stance;
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void AHuruBaseCharacter::OnRotationModeChanged(EHuruRotationMode PreviousRotationMode)
{
	if (RotationMode == EHuruRotationMode::VelocityDirection && ViewMode == EHuruViewMode::FirstPerson)
	{
		// 새로운 회전 모드가 속도 방향이고 캐릭터가 1인칭인 경우, 뷰 모드를 3인칭으로 설정함
		SetViewMode(EHuruViewMode::ThirdPerson);
	}

	if (CameraBehavior)
	{
		CameraBehavior->SetRotationMode(RotationMode);
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void AHuruBaseCharacter::OnGaitChanged(EHuruGait PreviousGait)
{
	if (CameraBehavior)
	{
		CameraBehavior->Gait = Gait;
	}
}

void AHuruBaseCharacter::OnViewModeChanged(EHuruViewMode PreviousViewMode)
{
	if (ViewMode == EHuruViewMode::ThirdPerson)
	{
		if (RotationMode == EHuruRotationMode::VelocityDirection || RotationMode == EHuruRotationMode::LookingDirection)
		{
			// 3인칭일 경우, 회전 모드를 원하는 모드로 다시 설정
			SetRotationMode(DesiredRotationMode);
		}
	}
	else if (ViewMode == EHuruViewMode::FirstPerson && RotationMode == EHuruRotationMode::VelocityDirection)
	{
		// 1인칭일 경우, 현재 속도 방향 모드에 있다면 회전 모드를 시선 방향으로 설정
		SetRotationMode(EHuruRotationMode::LookingDirection);
	}

	if (CameraBehavior)
	{
		CameraBehavior->ViewMode = ViewMode;
	}
}

void AHuruBaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EHuruStance::Crouching);
}

void AHuruBaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EHuruStance::Standing);
}

void AHuruBaseCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (IsLocallyControlled())
	{
		EventOnJumped();
	}
	if (HasAuthority())
	{
		Multicast_OnJumped();
	}
}

void AHuruBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (IsLocallyControlled())
	{
		EventOnLanded();
	}
	if (HasAuthority())
	{
		Multicast_OnLanded();
	}
}

void AHuruBaseCharacter::OnLandFrictionReset()
{
	// 제동 마찰력을 초기화한다
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
}

void AHuruBaseCharacter::SetEssentialValues(float DeltaTime)
{
	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		ReplicatedCurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
		ReplicatedControlRotation = GetControlRotation();
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	}
	else
	{
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration() != 0
			                       ? GetCharacterMovement()->GetMaxAcceleration()
			                       : EasedMaxAcceleration / 2;
	}

	// AimingRotation을 현재의 컨트롤 회전값으로 보간하여 캐릭터 회전 동작을 부드럽게 만듦. InterpSpeed 값을 줄이면 속도는 느려지지만 더 부드러운 움직임을 제공함
	AimingRotation = FMath::RInterpTo(AimingRotation, ReplicatedControlRotation, DeltaTime, 30);

	// 이 값들은 캡슐(캐릭터의 충돌 모양체)이 어떻게 움직이고 있으며, 어떻게 움직이기를 원하는지를 나타냄
	// 데이터 기반 애니메이션 시스템에서 필수적이며, 시스템 전반의 다양한 기능에서 사용됨
	// 따라서 이 값들을 한 곳에서 관리하는 것이 가장 쉽다고 생각함

	const FVector CurrentVel = GetVelocity();

	// 가속도의 양을 설정함
	const FVector NewAcceleration = (CurrentVel - PreviousVelocity) / DeltaTime;
	Acceleration = NewAcceleration.IsNearlyZero() || IsLocallyControlled() ? NewAcceleration : Acceleration / 2;
	
	// 캐릭터의 속도를 기준으로 이동 중인지 판단. 속도는 수평(x, y) 속도의 길이로 계산되므로 수직 이동은 고려하지 않음. 
	// 캐릭터가 이동 중이면 마지막 속도 회전을 업데이트. 이 값은 캐릭터가 정지한 이후에도 마지막 이동 방향을 알 수 있어 유용할 수 있음.
	Speed = CurrentVel.Size2D();
	bIsMoving = Speed > 1.0f;
	if (bIsMoving)
	{
		LastVelocityRotation = CurrentVel.ToOrientationRotator();
	}

	// 캐릭터가 움직임 입력을 받고 있는지 판단하기 위해 이동 입력 양을 확인함.
	// 이동 입력 양은 현재 가속도를 최대 가속도로 나누어 0에서 1 사이의 범위를 가짐. 
	// 1은 최대 입력 양, 0은 입력 없음을 의미함. 
	// 캐릭터가 움직임 입력이 있으면 마지막 이동 입력 회전을 업데이트함.
	MovementInputAmount = ReplicatedCurrentAcceleration.Size() / EasedMaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = ReplicatedCurrentAcceleration.ToOrientationRotator();
	}

	// 현재와 이전 Aim Yaw 값을 비교하여 Aim Yaw 속도를 설정하고, Delta Seconds로 나눔.
	// 이는 카메라가 좌우로 회전하는 속도를 나타냄.
	AimYawRate = FMath::Abs((AimingRotation.Yaw - PreviousAimYaw) / DeltaTime);
}

void AHuruBaseCharacter::UpdateCharacterMovement()
{
	// 허용된 보행 방식을 설정
	const EHuruGait AllowedGait = GetAllowedGait();

	// 실제 보행 방식을 결정. 현재 보행 방식과 다르면 새로운 보행 방식 이벤트를 설정.
	const EHuruGait ActualGait = GetActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}

	// 현재 허용된 보행 방식에 따라 설정된 속도로 캐릭터의 최대 이동 속도를 업데이트.
	MyCharacterMovementComponent->SetAllowedGait(AllowedGait);
}

void AHuruBaseCharacter::UpdateGroundedRotation(float DeltaTime)
{
	if (MovementAction == EHuruMovementAction::None)
	{
		const bool bCanUpdateMovingRot = ((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !HasAnyRootMotion();
		if (bCanUpdateMovingRot)
		{
			const float GroundedRotationRate = CalculateGroundedRotationRate();
			if (RotationMode == EHuruRotationMode::VelocityDirection)
			{
				// 속도 방향 회전
				SmoothCharacterRotation({0.0f, LastVelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate, DeltaTime);
			}
			else if (RotationMode == EHuruRotationMode::LookingDirection)
			{
				// 바라보는 방향 회전
				float YawValue;
				if (Gait == EHuruGait::Sprinting)
				{
					YawValue = LastVelocityRotation.Yaw;
				}
				else
				{
					// 걷거나 뛰거나
					const float YawOffsetCurveVal = GetAnimCurveValue(NAME_YawOffset);
					YawValue = AimingRotation.Yaw + YawOffsetCurveVal;
				}
				SmoothCharacterRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
			}
			else if (RotationMode == EHuruRotationMode::Aiming)
			{
				const float ControlYaw = AimingRotation.Yaw;
				SmoothCharacterRotation({0.0f, ControlYaw, 0.0f}, 1000.0f, 20.0f, DeltaTime);
			}
		}
		else
		{
			// 움직이지 않을 때

			if ((ViewMode == EHuruViewMode::ThirdPerson && RotationMode == EHuruRotationMode::Aiming) ||
				ViewMode == EHuruViewMode::FirstPerson)
			{
				LimitRotation(-100.0f, 100.0f, 20.0f, DeltaTime);
			}

			// Turn In Place 애니메이션에서 RotationAmount 커브를 적용함.
			// RotationAmount 커브는 매 프레임마다 적용될 회전량을 정의하며,
			// 30fps로 애니메이션된 애니메이션에 대해 계산됨.

			const float RotAmountCurve = GetAnimCurveValue(NAME_RotationAmount);

			if (FMath::Abs(RotAmountCurve) > 0.001f)
			{
				if (GetLocalRole() == ROLE_AutonomousProxy)
				{
					TargetRotation.Yaw = UKismetMathLibrary::NormalizeAxis(
						TargetRotation.Yaw + (RotAmountCurve * (DeltaTime / (1.0f / 30.0f))));
					SetActorRotation(TargetRotation);
				}
				else
				{
					AddActorWorldRotation({0, RotAmountCurve * (DeltaTime / (1.0f / 30.0f)), 0});
				}
				TargetRotation = GetActorRotation();
			}
		}
	}
	else if (MovementAction == EHuruMovementAction::Rolling)
	{
		// Rolling 회전 (네트워크 게임에서는 허용되지 않음)
		if (!bEnableNetworkOptimizations && bHasMovementInput)
		{
			SmoothCharacterRotation({0.0f, LastMovementInputRotation.Yaw, 0.0f}, 0.0f, 2.0f, DeltaTime);
		}
	}

	// 다른 동작들은 무시됨
}

void AHuruBaseCharacter::UpdateInAirRotation(float DeltaTime)
{
	if (RotationMode == EHuruRotationMode::VelocityDirection || RotationMode == EHuruRotationMode::LookingDirection)
	{
		// 속도 / 바라보는 방향 회전
		SmoothCharacterRotation({0.0f, InAirRotation.Yaw, 0.0f}, 0.0f, 5.0f, DeltaTime);
	}
	else if (RotationMode == EHuruRotationMode::Aiming)
	{
		// 조준 회전
		SmoothCharacterRotation({0.0f, AimingRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		InAirRotation = GetActorRotation();
	}
}

void AHuruBaseCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime)
{
	// 부드러운 회전 동작을 위해 목표 회전을 보간한다.
	TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetActorRotation( FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

float AHuruBaseCharacter::CalculateGroundedRotationRate() const
{
	// 현재 회전 속도 곡선을 사용하여 회전 속도를 계산한다.
	// 곡선을 매핑된 속도와 함께 사용하면 각 속도에 대한 회전 속도를 높이기 위한 높은 제어 수준을 제공한다.
	// 카메라가 빠르게 회전할 경우 속도를 증가시켜 더 반응성이 뛰어난 회전을 만든다.

	const float MappedSpeedVal = MyCharacterMovementComponent->GetMappedSpeed();
	const float CurveVal =
		MyCharacterMovementComponent->CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
	const float ClampedAimYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {1.0f, 3.0f}, AimYawRate);
	return CurveVal * ClampedAimYawRate;
}

void AHuruBaseCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	// 캐릭터가 특정 각도를 넘어 회전하지 않도록 방지한다.
	FRotator Delta = AimingRotation - GetActorRotation();
	Delta.Normalize();
	const float RangeVal = Delta.Yaw;

	if (RangeVal < AimYawMin || RangeVal > AimYawMax)
	{
		const float ControlRotYaw = AimingRotation.Yaw;
		const float TargetYaw = ControlRotYaw + (RangeVal > 0.0f ? AimYawMin : AimYawMax);
		SmoothCharacterRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

void AHuruBaseCharacter::SetMovementModel()
{
	const FString ContextString = GetFullName();
	FHuruMovementStateSettings* OutRow =
		MovementModel.DataTable->FindRow<FHuruMovementStateSettings>(MovementModel.RowName, ContextString);
	check(OutRow);
	MovementData = *OutRow;
}

void AHuruBaseCharacter::ForceUpdateCharacterState()
{
	SetGait(DesiredGait, true);
	SetStance(DesiredStance, true);
	SetRotationMode(DesiredRotationMode, true);
	SetViewMode(ViewMode, true);
	SetOverlayState(OverlayState, true);
	SetMovementState(MovementState, true);
	SetMovementAction(MovementAction, true);
}

void AHuruBaseCharacter::Server_SetDesiredGait_Implementation(EHuruGait NewGait)
{
	// 서버에서 실행되는 SetDesiredGait 함수의 실제 구현
	SetDesiredGait(NewGait);
}

void AHuruBaseCharacter::ForwardMovementAction_Implementation(float Value)
{
	if (MovementState == EHuruMovementState::Grounded || MovementState == EHuruMovementState::InAir)
	{
		// 기본 카메라의 상대적 이동 동작
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetForwardVector(DirRotator), Value);
	}
}

void AHuruBaseCharacter::RightMovementAction_Implementation(float Value)
{
	if (MovementState == EHuruMovementState::Grounded || MovementState == EHuruMovementState::InAir)
	{
		// 기본 카메라의 상대적 이동 동작
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetRightVector(DirRotator), Value);
	}
}

void AHuruBaseCharacter::CameraUpAction_Implementation(float Value)
{
	AddControllerPitchInput(LookUpDownRate * Value);
}

void AHuruBaseCharacter::CameraRightAction_Implementation(float Value)
{
	AddControllerYawInput(LookLeftRightRate * Value);
}

void AHuruBaseCharacter::JumpAction_Implementation(bool bValue)
{
	if (bValue)
	{
		// 점프 액션: "점프 액션"을 눌러서 웅크리고 있을 때는 일어나고, 서 있을 때는 점프함
		if (JumpPressedDelegate.IsBound())
		{
			JumpPressedDelegate.Broadcast();
		}

		if (MovementAction == EHuruMovementAction::None)
		{
			if (MovementState == EHuruMovementState::Grounded)
			{
				if (Stance == EHuruStance::Standing)
				{
					Jump();
				}
				else if (Stance == EHuruStance::Crouching)
				{
					UnCrouch();
				}
			}
		}
	}
	else
	{
		StopJumping();
	}
}

void AHuruBaseCharacter::SprintAction_Implementation(bool bValue)
{
	if (bValue)
	{
		SetDesiredGait(EHuruGait::Sprinting);
	}
	else
	{
		SetDesiredGait(EHuruGait::Running);
	}
}

void AHuruBaseCharacter::AimAction_Implementation(bool bValue)
{
	if (bValue)
	{
		// AimAction: "AimAction"을 누르고 있으면 조준 모드로 들어가고, 놓으면 원하는 회전 모드로 돌아감
		SetRotationMode(EHuruRotationMode::Aiming);
	}
	else
	{
		if (ViewMode == EHuruViewMode::ThirdPerson)
		{
			SetRotationMode(DesiredRotationMode);
		}
		else if (ViewMode == EHuruViewMode::FirstPerson)
		{
			SetRotationMode(EHuruRotationMode::LookingDirection);
		}
	}
}

void AHuruBaseCharacter::CameraTapAction_Implementation()
{
	if (ViewMode == EHuruViewMode::FirstPerson)
	{
		// 1인칭 모드에서는 바꾸지 않는다
		return;
	}

	// 어깨 방향 바꾸기
	SetRightShoulder(!bRightShoulder);
}

void AHuruBaseCharacter::CameraHeldAction_Implementation()
{
	// 카메라 모드 바꾸기
	if (ViewMode == EHuruViewMode::FirstPerson)
	{
		SetViewMode(EHuruViewMode::ThirdPerson);
	}
	else if (ViewMode == EHuruViewMode::ThirdPerson)
	{
		SetViewMode(EHuruViewMode::FirstPerson);
	}
}

void AHuruBaseCharacter::StanceAction_Implementation()
{
	// 스탠스 액션: '스탠스 액션'을 눌러 서기/무릎 꿇기를 전환하고, 더블 탭하면 구르기

	if (MovementAction != EHuruMovementAction::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World);

	const float PrevStanceInputTime = LastStanceInputTime;
	LastStanceInputTime = World->GetTimeSeconds();

	if (LastStanceInputTime - PrevStanceInputTime <= RollDoubleTapTimeout)
	{
		// Roll
		Replicated_PlayMontage(GetRollAnimation(), 1.15f);

		if (Stance == EHuruStance::Standing)
		{
			SetDesiredStance(EHuruStance::Crouching);
		}
		else if (Stance == EHuruStance::Crouching)
		{
			SetDesiredStance(EHuruStance::Standing);
		}
		return;
	}

	if (MovementState == EHuruMovementState::Grounded)
	{
		if (Stance == EHuruStance::Standing)
		{
			SetDesiredStance(EHuruStance::Crouching);
			Crouch();
		}
		else if (Stance == EHuruStance::Crouching)
		{
			SetDesiredStance(EHuruStance::Standing);
			UnCrouch();
		}
	}
}

void AHuruBaseCharacter::WalkAction_Implementation()
{
	if (DesiredGait == EHuruGait::Walking)
	{
		SetDesiredGait(EHuruGait::Running);
	}
	else if (DesiredGait == EHuruGait::Running)
	{
		SetDesiredGait(EHuruGait::Walking);
	}
}

void AHuruBaseCharacter::RagdollAction_Implementation()
{
	if (GetMovementState() != EHuruMovementState::Ragdoll)
	{
		ReplicatedRagdollStart(); // Ragdoll 시작을 서버에 알림
	}
}

void AHuruBaseCharacter::VelocityDirectionAction_Implementation()
{
	// 회전 모드 선택: 기본 회전 모드를 속도 방향이나 보는 방향으로 바꿈
	// 조준 해제하면 캐릭터가 다시 이 모드로 돌아감
	SetDesiredRotationMode(EHuruRotationMode::VelocityDirection);
	SetRotationMode(EHuruRotationMode::VelocityDirection);
}

void AHuruBaseCharacter::LookingDirectionAction_Implementation()
{
	SetDesiredRotationMode(EHuruRotationMode::LookingDirection);
	SetRotationMode(EHuruRotationMode::LookingDirection);
}




