// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/HuruBaseCharacter.h"
#include "Character/HuruCharacterMovementComponent.h"
#include "Character/Animation/Notify/HuruPlayerCameraBehavior.h"
//Engine
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

const FName NAME_Pelvis(TEXT("Pelvis"));


AHuruBaseCharacter::AHuruBaseCharacter(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

}

void AHuruBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHuruBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	MyCharacterMovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = true;

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

void AHuruBaseCharacter::SetMovementState(EHuruMovementState NewState, bool bForce)
{
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
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

void AHuruBaseCharacter::Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	// Roll: Simply play a Root Motion Montage.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	Server_PlayMontage(Montage, PlayRate);
}

void AHuruBaseCharacter::SetRightShoulder(bool bNewRightShoulder)
{
	bRightShoulder = bNewRightShoulder;
	if (CameraBehavior)
	{
		CameraBehavior->bRightShoulder = bRightShoulder;
	}
}

void AHuruBaseCharacter::OnRep_RotationMode(EHuruRotationMode PrevRotMode)
{
}

void AHuruBaseCharacter::OnRep_ViewMode(EHuruViewMode PrevViewMode)
{
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
	if (GetMovementState() == EHuruMovementState::Ragdoll)
	{
		ReplicatedRagdollStart(); // Ragdoll 시작을 서버에 알림
	}
}

void AHuruBaseCharacter::VelocityDirectionAction_Implementation()
{
}

void AHuruBaseCharacter::LookingDirectionAction_Implementation()
{
}




