// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/Animation/HuruCharacterAnimInstance.h"
#include "Character/HuruBaseCharacter.h"
#include "Library/HuruMathLibrary.h"
//Engine
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"

static const FName NAME_BasePose_CLF(TEXT("BasePose_CLF"));
static const FName NAME_BasePose_N(TEXT("BasePose_N"));
static const FName NAME_Enable_FootIK_R(TEXT("Enable_FootIK_R"));
static const FName NAME_Enable_FootIK_L(TEXT("Enable_FootIK_L"));
static const FName NAME_Enable_HandIK_L(TEXT("Enable_HandIK_L"));
static const FName NAME_Enable_HandIK_R(TEXT("Enable_HandIK_R"));
static const FName NAME_Enable_Transition(TEXT("Enable_Transition"));
static const FName NAME_FootLock_L(TEXT("FootLock_L"));
static const FName NAME_FootLock_R(TEXT("FootLock_R"));
static const FName NAME_Grounded___Slot(TEXT("Grounded Slot"));
static const FName NAME_Layering_Arm_L(TEXT("Layering_Arm_L"));
static const FName NAME_Layering_Arm_L_Add(TEXT("Layering_Arm_L_Add"));
static const FName NAME_Layering_Arm_L_LS(TEXT("Layering_Arm_L_LS"));
static const FName NAME_Layering_Arm_R(TEXT("Layering_Arm_R"));
static const FName NAME_Layering_Arm_R_Add(TEXT("Layering_Arm_R_Add"));
static const FName NAME_Layering_Arm_R_LS(TEXT("Layering_Arm_R_LS"));
static const FName NAME_Layering_Hand_L(TEXT("Layering_Hand_L"));
static const FName NAME_Layering_Hand_R(TEXT("Layering_Hand_R"));
static const FName NAME_Layering_Head_Add(TEXT("Layering_Head_Add"));
static const FName NAME_Layering_Spine_Add(TEXT("Layering_Spine_Add"));
static const FName NAME_Mask_AimOffset(TEXT("Mask_AimOffset"));
static const FName NAME_Mask_LandPrediction(TEXT("Mask_LandPrediction"));
static const FName NAME__ALSCharacterAnimInstance__RotationAmount(TEXT("RotationAmount"));
static const FName NAME_VB___foot_target_l(TEXT("VB foot_target_l"));
static const FName NAME_VB___foot_target_r(TEXT("VB foot_target_r"));
static const FName NAME_W_Gait(TEXT("W_Gait"));
static const FName NAME__ALSCharacterAnimInstance__root(TEXT("root"));

void UHuruCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AHuruBaseCharacter>(TryGetPawnOwner());
	if (Character)
	{
		Character->OnJumpedDelegate.AddUniqueDynamic(this, &UHuruCharacterAnimInstance::OnJumped);
	}
}

void UHuruCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character || DeltaSeconds == 0.0f)
	{
		return;
	}

	// 캐릭터 정보의 나머지 부분을 업데이트함. 다른 정보는 캐릭터 클래스 안에서 설정될 때 애니메이션 블루프린트에 반영됨
	CharacterInformation.MovementInputAmount = Character->GetMovementInputAmount();
	CharacterInformation.bHasMovementInput = Character->HasMovementInput();
	CharacterInformation.bIsMoving = Character->IsMoving();
	CharacterInformation.Acceleration = Character->GetAcceleration();
	CharacterInformation.AimYawRate = Character->GetAimYawRate();
	CharacterInformation.Speed = Character->GetSpeed();
	CharacterInformation.Velocity = Character->GetCharacterMovement()->Velocity;
	CharacterInformation.MovementInput = Character->GetMovementInput();
	CharacterInformation.AimingRotation = Character->GetAimingRotation();
	CharacterInformation.CharacterActorRotation = Character->GetActorRotation();
	CharacterInformation.ViewMode = Character->GetViewMode();
	CharacterInformation.PrevMovementState = Character->GetPrevMovementState();
	MovementState = Character->GetMovementState();
	MovementAction = Character->GetMovementAction();
	Stance = Character->GetStance();
	RotationMode = Character->GetRotationMode();
	Gait = Character->GetGait();
	GroundedEntryState = Character->GetGroundedEntryState();

	UpdateAimingValues(DeltaSeconds);
	UpdateLayerValues();
	UpdateFootIK(DeltaSeconds);
	
	if (MovementState.Grounded())
	{
		// 움직이는지 체크하고, IsMoving와 HasMovementInput가 참이거나 Speed가 150보다 크면 이동 애니메이션을 활성화함.
		const bool bPrevShouldMove = Grounded.bShouldMove;
		Grounded.bShouldMove = ShouldMoveCheck();

		if (bPrevShouldMove == false && Grounded.bShouldMove)
		{
			// 움직이기 시작할 때 수행함.
			TurnInPlaceValues.ElapsedDelayTime = 0.0f;
			Grounded.bRotateL = false;
			Grounded.bRotateR = false;
		}

		if (Grounded.bShouldMove)
		{
			// 움직이는 동안 수행함.
			UpdateMovementValues(DeltaSeconds);
			UpdateRotationValues();
		}
		else
		{
			// 움직이지 않을 때 수행함.
			if (CanRotateInPlace())
			{
				RotateInPlaceCheck();
			}
			else
			{
				Grounded.bRotateL = false;
				Grounded.bRotateR = false;
			}
			if (CanTurnInPlace())
			{
				TurnInPlaceCheck(DeltaSeconds);
			}
			else
			{
				TurnInPlaceValues.ElapsedDelayTime = 0.0f;
			}
			if (CanDynamicTransition())
			{
				DynamicTransitionCheck();
			}
		}
	}
	else if (MovementState.InAir())
	{
		// 공중에 떠 있을때 수행함.
		UpdateInAirValues(DeltaSeconds);
	}
	else if (MovementState.Ragdoll())
	{
		// 랙돌인 상태에서 수행함.
		UpdateRagdollValues();
	}
}

void UHuruCharacterAnimInstance::PlayTransition(const FHuruDynamicMontageParams& Parameters)
{
	PlaySlotAnimationAsDynamicMontage(
		Parameters.Animation,
		NAME_Grounded___Slot,
		Parameters.BlendInTime,
		Parameters.BlendOutTime,
		Parameters.PlayRate,
		1,
		0.0f,
		Parameters.StartTime);
}

void UHuruCharacterAnimInstance::PlayTransitionChecked(const FHuruDynamicMontageParams& Parameters)
{
	if (Stance.Standing() && !Grounded.bShouldMove)
	{
		PlayTransition(Parameters);
	}
}

void UHuruCharacterAnimInstance::PlayDynamicTransition(float ReTriggerDelay, FHuruDynamicMontageParams Parameters)
{
	if (bCanPlayDynamicTransition)
	{
		bCanPlayDynamicTransition = false;

		// 동적 추가 전환 애니메이션 재생함.
		PlayTransition(Parameters);

		UWorld* World = GetWorld();
		check(World);
		World->GetTimerManager().SetTimer(
			PlayDynamicTransitionTimer,
			this,
			&UHuruCharacterAnimInstance::PlayDynamicTransitionDelay,
			ReTriggerDelay, false);
	}
}

void UHuruCharacterAnimInstance::OnJumped()
{
	InAir.bJumped = true;
	InAir.JumpPlayRate = FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, 600.0f}, {1.2f, 1.5f}, CharacterInformation.Speed);

	GetWorld()->GetTimerManager().SetTimer(
		OnJumpedTimer,
		this,
		&UHuruCharacterAnimInstance::OnJumpedDelay,
		0.1f,
		false);
}

void UHuruCharacterAnimInstance::OnPivot()
{
	Grounded.bPivot = CharacterInformation.Speed < Config.TriggerPivotSpeedLimit;
	GetWorld()->GetTimerManager().SetTimer(
		OnPivotTimer,
		this,
		&UHuruCharacterAnimInstance::OnPivotDelay,
		0.1f,
		false);
}

bool UHuruCharacterAnimInstance::ShouldMoveCheck() const
{
	return (CharacterInformation.bIsMoving && CharacterInformation.bHasMovementInput) ||
		CharacterInformation.Speed > 150.0f;
}

bool UHuruCharacterAnimInstance::CanRotateInPlace() const
{
	return RotationMode.Aiming() ||
		CharacterInformation.ViewMode == EHuruViewMode::FirstPerson;
}

bool UHuruCharacterAnimInstance::CanTurnInPlace() const
{
	return RotationMode.LookingDirection() &&
		CharacterInformation.ViewMode == EHuruViewMode::ThirdPerson &&
		GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

bool UHuruCharacterAnimInstance::CanDynamicTransition() const
{
	return GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

void UHuruCharacterAnimInstance::PlayDynamicTransitionDelay()
{
	bCanPlayDynamicTransition = true;
}

void UHuruCharacterAnimInstance::OnJumpedDelay()
{
	InAir.bJumped = false;
}

void UHuruCharacterAnimInstance::OnPivotDelay()
{
	Grounded.bPivot = false;
}

void UHuruCharacterAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	// 조준 회전 값을 보간(interpolate)하여 부드러운 조준 회전 변화를 달성함.
	//회전을 계산하기 전에 회전을 보간하면 액터 회전의 변화에 영향을 받지 않아, 빠른 액터 회전 변화에도 느린 조준 회전 변화를 유지할 수 있음.
	AimingValues.SmoothedAimingRotation = FMath::RInterpTo(AimingValues.SmoothedAimingRotation,
	                                                       CharacterInformation.AimingRotation, DeltaSeconds,
	                                                       Config.SmoothedAimingRotationInterpSpeed);

	// 조준 각도와 부드러운 조준 각도를 계산하기 위해, 조준 회전과 액터 회전 간의 차이를 가져옴.
	FRotator Delta = CharacterInformation.AimingRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	AimingValues.AimingAngle.X = Delta.Yaw;
	AimingValues.AimingAngle.Y = Delta.Pitch;

	Delta = AimingValues.SmoothedAimingRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	SmoothedAimingAngle.X = Delta.Yaw;
	SmoothedAimingAngle.Y = Delta.Pitch;

	if (!RotationMode.VelocityDirection())
	{
		// 조준 피치 각도를 1에서 0의 범위로 제한하여 수직 조준 스윕에 사용함.
		AimingValues.AimSweepTime = FMath::GetMappedRangeValueClamped<float, float>({-90.0f, 90.0f}, {1.0f, 0.0f},
		                                                              AimingValues.AimingAngle.Y);

		// 카메라 방향을 유지하기 위해 필요한 척추 회전 양을 얻기 위해 조준 요각을 척추와 골반 본의 수로 나누어 사용함.
		AimingValues.SpineRotation.Roll = 0.0f;
		AimingValues.SpineRotation.Pitch = 0.0f;
		AimingValues.SpineRotation.Yaw = AimingValues.AimingAngle.X / 4.0f;
	}
	else if (CharacterInformation.bHasMovementInput)
	{
		// 이동 입력 회전과 액터 회전의 차이를 구하고 이를 0-1 범위로 매핑함.
		// 이 값은 캐릭터가 이동 입력 방향을 바라보도록 하기 위해 조준 오프셋 동작에서 사용됨.
		Delta = CharacterInformation.MovementInput.ToOrientationRotator() - CharacterInformation.CharacterActorRotation;
		Delta.Normalize();
		const float InterpTarget = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f}, Delta.Yaw);

		AimingValues.InputYawOffsetTime = FMath::FInterpTo(AimingValues.InputYawOffsetTime, InterpTarget,
		                                                   DeltaSeconds, Config.InputYawOffsetInterpSpeed);
	}

	// 조준 요 각도를 3개의 개별 요 타임으로 분리함.
	// 이 3개의 값은 캐릭터 주위를 완전히 회전할 때 조준 오프셋의 혼합을 개선하기 위해 조준 오프셋 동작에서 사용됨.
	// 이를 통해 조준이 반응성을 유지하면서도 왼쪽에서 오른쪽 또는 오른쪽에서 왼쪽으로 부드럽게 혼합될 수 있도록 함.
	AimingValues.LeftYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 0.0f},
	                                                             FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.RightYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 1.0f},
	                                                              FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.ForwardYawTime = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f},
	                                                                SmoothedAimingAngle.X);
}

void UHuruCharacterAnimInstance::UpdateLayerValues()
{
	// 조준 오프셋 마스크의 반대를 가져와 조준 오프셋 가중치를 가져옴.
	LayerBlendingValues.EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(NAME_Mask_AimOffset));
	// 기본 포즈 가중치 설정.
	LayerBlendingValues.BasePose_N = GetCurveValue(NAME_BasePose_N);
	LayerBlendingValues.BasePose_CLF = GetCurveValue(NAME_BasePose_CLF);
	// 각 신체 부위에 대한 추가 가중치 설정.
	LayerBlendingValues.Spine_Add = GetCurveValue(NAME_Layering_Spine_Add);
	LayerBlendingValues.Head_Add = GetCurveValue(NAME_Layering_Head_Add);
	LayerBlendingValues.Arm_L_Add = GetCurveValue(NAME_Layering_Arm_L_Add);
	LayerBlendingValues.Arm_R_Add = GetCurveValue(NAME_Layering_Arm_R_Add);
	// 손 오버라이드 가중치 설정.
	LayerBlendingValues.Hand_R = GetCurveValue(NAME_Layering_Hand_R);
	LayerBlendingValues.Hand_L = GetCurveValue(NAME_Layering_Hand_L);
	// 손 IK 가중치를 혼합하고 설정하여 팔 레이어에서 허용된 경우에만 가중치가 적용되도록 함.
	LayerBlendingValues.EnableHandIK_L = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_L),
													 GetCurveValue(NAME_Layering_Arm_L));
	LayerBlendingValues.EnableHandIK_R = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_R),
													 GetCurveValue(NAME_Layering_Arm_R));
	// 팔이 메쉬 공간에서 혼합되어야 할지 로컬 공간에서 혼합되어야 할지를 설정함.
	// 메쉬 공간 가중치는 로컬 공간(LS) 곡선이 완전히 가중치가 적용되지 않는 한 항상 1이 됨.
	LayerBlendingValues.Arm_L_LS = GetCurveValue(NAME_Layering_Arm_L_LS);
	LayerBlendingValues.Arm_L_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_L_LS));
	LayerBlendingValues.Arm_R_LS = GetCurveValue(NAME_Layering_Arm_R_LS);
	LayerBlendingValues.Arm_R_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_R_LS));
}

void UHuruCharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	// Update Foot Locking values.
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_L, NAME_FootLock_L,
				   IkFootL_BoneName, FootIKValues.FootLock_L_Alpha, FootIKValues.UseFootLockCurve_L,
				   FootIKValues.FootLock_L_Location, FootIKValues.FootLock_L_Rotation);
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_R, NAME_FootLock_R,
				   IkFootR_BoneName, FootIKValues.FootLock_R_Alpha, FootIKValues.UseFootLockCurve_R,
				   FootIKValues.FootLock_R_Location, FootIKValues.FootLock_R_Rotation);

	if (MovementState.InAir())
	{
		// Reset IK Offsets if In Air
		SetPelvisIKOffset(DeltaSeconds, FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets(DeltaSeconds);
	}
	else if (!MovementState.Ragdoll())
	{
		// Update all Foot Lock and Foot Offset values when not In Air
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_L, IkFootL_BoneName, NAME__ALSCharacterAnimInstance__root,
					   FootOffsetLTarget,
					   FootIKValues.FootOffset_L_Location, FootIKValues.FootOffset_L_Rotation);
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_R, IkFootR_BoneName, NAME__ALSCharacterAnimInstance__root,
					   FootOffsetRTarget,
					   FootIKValues.FootOffset_R_Location, FootIKValues.FootOffset_R_Rotation);
		SetPelvisIKOffset(DeltaSeconds, FootOffsetLTarget, FootOffsetRTarget);
	}
}

void UHuruCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	// 속도 블렌드를 보간하고 설정함.
	const FHuruVelocityBlend& TargetBlend = CalculateVelocityBlend();
	VelocityBlend.F = FMath::FInterpTo(VelocityBlend.F, TargetBlend.F, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.B = FMath::FInterpTo(VelocityBlend.B, TargetBlend.B, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.L = FMath::FInterpTo(VelocityBlend.L, TargetBlend.L, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.R = FMath::FInterpTo(VelocityBlend.R, TargetBlend.R, DeltaSeconds, Config.VelocityBlendInterpSpeed);

	// 대각선 스케일 양을 설정함.
	Grounded.DiagonalScaleAmount = CalculateDiagonalScaleAmount();

	// 상대 가속도 양을 설정하고, 기울기 양을 보간(interp)함.
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, RelativeAccelerationAmount.Y, DeltaSeconds,
									 Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, RelativeAccelerationAmount.X, DeltaSeconds,
									 Config.GroundedLeanInterpSpeed);

	// 걷기와 달리기 블렌드를 설정함.
	Grounded.WalkRunBlend = CalculateWalkRunBlend();

	// 보폭 블렌드를 설정함.
	Grounded.StrideBlend = CalculateStrideBlend();

	// 서있는 상태와 웅크린 상태의 재생 속도를 설정함.
	Grounded.StandingPlayRate = CalculateStandingPlayRate();
	Grounded.CrouchingPlayRate = CalculateCrouchingPlayRate(); 
}

void UHuruCharacterAnimInstance::UpdateRotationValues()
{
	// 이동 방향을 설정함.
	MovementDirection = CalculateMovementDirection();

	// Yaw 오프셋을 설정함. 이 값들은 AnimGraph의 "YawOffset" 커브에 영향을 주며,
	// 캐릭터의 회전 방향을 더 자연스럽게 보이도록 오프셋을 적용하는 데 사용됨. 
	// 커브를 통해 각 이동 방향에서 오프셋이 어떻게 작동할지 세밀하게 조절할 수 있음.
	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.AimingRotation;
	Delta.Normalize();
	const FVector& FBOffset = YawOffset_FB->GetVectorValue(Delta.Yaw);
	Grounded.FYaw = FBOffset.X;
	Grounded.BYaw = FBOffset.Y;
	const FVector& LROffset = YawOffset_LR->GetVectorValue(Delta.Yaw);
	Grounded.LYaw = LROffset.X;
	Grounded.RYaw = LROffset.Y;
}

void UHuruCharacterAnimInstance::UpdateInAirValues(float DeltaSeconds)
{
	// 낙하 속도를 업데이트함. 공중에 있을 때만 이 값을 설정하면 착지 강도를 AnimGraph에서 사용할 수 있음.
	// 그렇지 않으면, 착지 시 Z 속도가 0으로 돌아감.
	InAir.FallSpeed = CharacterInformation.Velocity.Z;

	// 착지 예측 가중치를 설정함.
	InAir.LandPrediction = CalculateLandPrediction();

	// 공중에서의 기울기(Lean) 값을 보간(Interp)하고 설정함.
	const FHuruLeanAmount& InAirLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, InAirLeanAmount.LR, DeltaSeconds, Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, InAirLeanAmount.FB, DeltaSeconds, Config.GroundedLeanInterpSpeed);
}

void UHuruCharacterAnimInstance::UpdateRagdollValues()
{
	// 속도 길이에 따라 난동(Flail) 속도를 조정함. 랙돌이 빠르게 움직일수록 캐릭터가 더 빠르게 난동을 부림.
	const float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(NAME__ALSCharacterAnimInstance__root).Size();
	FlailRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

void UHuruCharacterAnimInstance::SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	float& CurFootLockAlpha, bool& UseFootLockCurve, FVector& CurFootLockLoc, FRotator& CurFootLockRot)
{
	if (GetCurveValue(EnableFootIKCurve) <= 0.0f)
	{
		return;
	}

	// Step 1: 로컬 발잠금 커브 값을 설정함
	float FootLockCurveVal;

	if (UseFootLockCurve)
	{
		UseFootLockCurve = FMath::Abs(GetCurveValue(NAME__ALSCharacterAnimInstance__RotationAmount)) <= 0.001f ||
			Character->GetLocalRole() != ROLE_AutonomousProxy;
		FootLockCurveVal = GetCurveValue(FootLockCurve) * (1.f / GetSkelMeshComponent()->AnimUpdateRateParams->UpdateRate);
	}
	else
	{
		UseFootLockCurve = GetCurveValue(FootLockCurve) >= 0.99f;
		FootLockCurveVal = 0.0f;
	}

	// Step 2: 새 값이 현재 값보다 작거나 1일 때만 발잠금 알파를 업데이트함
	// 이렇게 하면 발이 잠긴 위치에서만 블렌드 아웃되거나 새로운 위치로 잠길 수 있으며, 절대로 블렌드 인되지 않도록 함
	if (FootLockCurveVal >= 0.99f || FootLockCurveVal < CurFootLockAlpha)
	{
		CurFootLockAlpha = FootLockCurveVal;
	}

	// Step 3: 발 잠금 커브가 1이면, 새로운 잠금 위치와 회전을 컴포넌트 공간에서 타겟으로 저장함
	if (CurFootLockAlpha >= 0.99f)
	{
		const FTransform& OwnerTransform =
			GetOwningComponent()->GetSocketTransform(IKFootBone, RTS_Component);
		CurFootLockLoc = OwnerTransform.GetLocation();
		CurFootLockRot = OwnerTransform.Rotator();
	}

	// Step 4: 발 잠금 알파에 가중치가 있으면, 캡슐이 움직이는 동안 발이 고정된 위치에 있도록 발 잠금 오프셋을 업데이트함.
	if (CurFootLockAlpha > 0.0f)
	{
		SetFootLockOffsets(DeltaSeconds, CurFootLockLoc, CurFootLockRot);
	}
}

void UHuruCharacterAnimInstance::SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot)
{
	FRotator RotationDifference = FRotator::ZeroRotator;
	// 발이 땅에 고정된 상태를 유지하기 위해, 현재 회전과 마지막으로 업데이트된 회전 사이의 차이를 사용하여 발이 얼마나 회전해야 하는지 계산
	if (Character->GetCharacterMovement()->IsMovingOnGround())
	{
		RotationDifference = CharacterInformation.CharacterActorRotation - Character->GetCharacterMovement()->
			GetLastUpdateRotation();
		RotationDifference.Normalize();
	}

	// 발이 땅에 고정된 상태를 유지하기 위해, 메쉬 회전에 대해 프레임 간에 이동한 거리를 계산하여 발이 얼마나 오프셋되어야 하는지 찾음
	const FVector& LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(
		CharacterInformation.Velocity * DeltaSeconds);

	// 발이 컴포넌트 공간에서 고정된 상태를 유지하기 위해 현재 로컬 위치에서 위치 차이를 빼고, 회전 차이만큼 회전시킴
	LocalLoc = (LocalLoc - LocationDifference).RotateAngleAxis(RotationDifference.Yaw, FVector::DownVector);

	// 현재 로컬 회전에서 회전 차이를 빼서 새로운 로컬 회전을 구함
	FRotator Delta = LocalRot - RotationDifference;
	Delta.Normalize();
	LocalRot = Delta;
}

void UHuruCharacterAnimInstance::SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget)
{
	// 평균 발 IK 가중치를 찾아서 골반 알파를 계산한다. 알파가 0이면 오프셋을 지움
	FootIKValues.PelvisAlpha =
		(GetCurveValue(NAME_Enable_FootIK_L) + GetCurveValue(NAME_Enable_FootIK_R)) / 2.0f;

	if (FootIKValues.PelvisAlpha > 0.0f)
	{
		// Step 1: 새로운 골반 타겟을 가장 낮은 발 오프셋으로 설정함
		const FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;

		// Step 2: 현재 골반 오프셋을 새로운 타겟 값으로 보간(interp)함
		// 새로운 타겟이 현재 값보다 위에 있는지 아래에 있는지에 따라 서로 다른 속도로 보간함
		const float InterpSpeed = PelvisTarget.Z > FootIKValues.PelvisOffset.Z ? 10.0f : 15.0f;
		FootIKValues.PelvisOffset =
			FMath::VInterpTo(FootIKValues.PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
	}
	else
	{
		FootIKValues.PelvisOffset = FVector::ZeroVector;
	}
}

void UHuruCharacterAnimInstance::ResetIKOffsets(float DeltaSeconds)
{
	// 발 IK 오프셋을 0으로 보간(interp)함
	FootIKValues.FootOffset_L_Location = FMath::VInterpTo(FootIKValues.FootOffset_L_Location,
														  FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Location = FMath::VInterpTo(FootIKValues.FootOffset_R_Location,
														  FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_L_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_L_Rotation,
														  FRotator::ZeroRotator, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_R_Rotation,
														  FRotator::ZeroRotator, DeltaSeconds, 15.0f);
}

void UHuruCharacterAnimInstance::SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset)
{
	// Foot IK 커브에 가중치가 있을 경우에만 Foot IK 오프셋 값을 업데이트한다. 만약 가중치가 0이라면, 오프셋 값을 지움
	if (GetCurveValue(EnableFootIKCurve) <= 0)
	{
		CurLocationOffset = FVector::ZeroVector;
		CurRotationOffset = FRotator::ZeroRotator;
		return;
	}

	// Step 1: 발 위치에서 아래로 트레이스를 하여 지형을 찾음
	// 만약 표면이 걷기 가능한 경우, 충격 위치와 노멀을 저장
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	FVector IKFootFloorLoc = OwnerComp->GetSocketLocation(IKFootBone);
	IKFootFloorLoc.Z = OwnerComp->GetSocketLocation(RootBone).Z;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	const FVector TraceStart = IKFootFloorLoc + FVector(0.0, 0.0, Config.IK_TraceDistanceAboveFoot);
	const FVector TraceEnd = IKFootFloorLoc - FVector(0.0, 0.0, Config.IK_TraceDistanceBelowFoot);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult,
	                                                  TraceStart,
	                                                  TraceEnd,
	                                                  ECC_Visibility, Params);
	
	FRotator TargetRotOffset = FRotator::ZeroRotator;
	if (Character->GetCharacterMovement()->IsWalkable(HitResult))
	{
		FVector ImpactPoint = HitResult.ImpactPoint;
		FVector ImpactNormal = HitResult.ImpactNormal;

		// Step 1.1: 임팩트 지점과 예상(평평한) 바닥 위치 간의 위치 차이를 찾음
		// 이 값들은 발 높이에 노멀을 곱한 값으로 오프셋되어, 경사진 표면에서 더 나은 동작을 얻음
		CurLocationTarget = (ImpactPoint + ImpactNormal * Config.FootHeight) -
			(IKFootFloorLoc + FVector(0, 0, Config.FootHeight));

		// Step 1.2: 임팩트 노멀의 아탄젠트 값을 사용하여 회전 오프셋을 계산함
		TargetRotOffset.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.X, ImpactNormal.Z));
		TargetRotOffset.Roll = FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z));
	}

	// Step 2: 현재 위치 오프셋을 새로운 목표 값으로 보간함
	// 새로운 목표가 현재 값보다 위에 있는지 아래에 있는지에 따라 서로 다른 속도로 보간함
	const float InterpSpeed = CurLocationOffset.Z > CurLocationTarget.Z ? 30.f : 15.0f;
	CurLocationOffset = FMath::VInterpTo(CurLocationOffset, CurLocationTarget, DeltaSeconds, InterpSpeed);

	// Step 3: 현재 회전 오프셋을 새로운 목표 값으로 보간함
	CurRotationOffset = FMath::RInterpTo(CurRotationOffset, TargetRotOffset, DeltaSeconds, 30.0f);
}

void UHuruCharacterAnimInstance::RotateInPlaceCheck()
{
	// Step 1: 에이밍 각도가 임계값을 초과하는지 확인하여 캐릭터가 왼쪽 또는 오른쪽으로 회전해야 하는지 확인함.
	Grounded.bRotateL = AimingValues.AimingAngle.X < RotateInPlace.RotateMinThreshold;
	Grounded.bRotateR = AimingValues.AimingAngle.X > RotateInPlace.RotateMaxThreshold;

	// Step 2: 캐릭터가 회전해야 하는 경우, 회전 속도를 Aim Yaw Rate에 맞춰 설정함.
	// 이렇게 하면 카메라를 더 빨리 움직일 때 캐릭터가 더 빠르게 회전하게 됨.
	if (Grounded.bRotateL || Grounded.bRotateR)
	{
		Grounded.RotateRate = FMath::GetMappedRangeValueClamped<float, float>(
			{RotateInPlace.AimYawRateMinRange, RotateInPlace.AimYawRateMaxRange},
			{RotateInPlace.MinPlayRate, RotateInPlace.MaxPlayRate},
			CharacterInformation.AimYawRate);
	}
}

void UHuruCharacterAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
{
	// Step 1: 에이밍 각도가 Turn Check 최소 각도 밖에 있고, Aim Yaw Rate가 Aim Yaw Rate Limit보다 낮은지 확인함.
	// 그렇다면, Elapsed Delay Time을 카운트 시작함. 그렇지 않으면, Elapsed Delay Time을 초기화함.
	// 이렇게 하면, 제자리에 돌기 전에 일정 시간 동안 조건이 유지되도록 보장함.
	if (FMath::Abs(AimingValues.AimingAngle.X) <= TurnInPlaceValues.TurnCheckMinAngle ||
		CharacterInformation.AimYawRate >= TurnInPlaceValues.AimYawRateLimit)
	{
		TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		return;
	}

	TurnInPlaceValues.ElapsedDelayTime += DeltaSeconds;
	const float ClampedAimAngle = FMath::GetMappedRangeValueClamped<float,float>
	({TurnInPlaceValues.TurnCheckMinAngle, 180.0f},
		{TurnInPlaceValues.MinAngleDelay,TurnInPlaceValues.MaxAngleDelay},
		AimingValues.AimingAngle.X);

	// Step 2: Elapsed Delay 시간이 설정된 지연 시간을 초과하는지 확인 (회전 각도 범위에 매핑됨). 
	// 그렇다면, 제자리 회전을 트리거함.
	if (TurnInPlaceValues.ElapsedDelayTime > ClampedAimAngle)
	{
		FRotator TurnInPlaceYawRot = CharacterInformation.AimingRotation;
		TurnInPlaceYawRot.Roll = 0.0f;
		TurnInPlaceYawRot.Pitch = 0.0f;
		TurnInPlace(TurnInPlaceYawRot, 1.0f, 0.0f, false);
	}
}

void UHuruCharacterAnimInstance::DynamicTransitionCheck()
{
	// 각 발을 확인하여 IK_Foot 본과 원하는/목표 위치 (가상 본을 통해 결정됨) 사이의 위치 차이가 임계값을 초과하는지 확인.
	// 초과할 경우, 해당 발에 애드티브 전환 애니메이션을 재생함.
	// 현재 설정된 전환은 두 발 전환 애니메이션의 후반부를 재생하므로 단일 발만 움직임.
	// IK_Foot 본만 잠길 수 있기 때문에 별도의 가상 본을 통해 잠길 때 시스템이 원하는 위치를 알 수 있도록 함.
	FTransform SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootL_BoneName, RTS_Component);
	FTransform SocketTransformB = GetOwningComponent()->GetSocketTransform(
		NAME_VB___foot_target_l, RTS_Component);
	float Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FHuruDynamicMontageParams Params;
		Params.Animation = TransitionAnim_R;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}

	SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootR_BoneName, RTS_Component);
	SocketTransformB = GetOwningComponent()->GetSocketTransform(NAME_VB___foot_target_r, RTS_Component);
	Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FHuruDynamicMontageParams Params;
		Params.Animation = TransitionAnim_L;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}
}

FHuruVelocityBlend UHuruCharacterAnimInstance::CalculateVelocityBlend() const
{
	// 속도 블렌드를 계산함. 이 값은 각 방향에서 액터의 속도 양을 나타내며 (대각선이 각 방향에서 0.5가 되도록 정규화됨),
	// 표준 블렌드 스페이스보다 더 나은 방향 블렌딩을 생성하기 위해 BlendMulti 노드에서 사용됨.
	const FVector LocRelativeVelocityDir =
		CharacterInformation.CharacterActorRotation.UnrotateVector(CharacterInformation.Velocity.GetSafeNormal(0.1f));
	const float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) +
		FMath::Abs(LocRelativeVelocityDir.Z);
	const FVector RelativeDir = LocRelativeVelocityDir / Sum;
	FHuruVelocityBlend Result;
	Result.F = FMath::Clamp(RelativeDir.X, 0.0f, 1.0f);
	Result.B = FMath::Abs(FMath::Clamp(RelativeDir.X, -1.0f, 0.0f));
	Result.L = FMath::Abs(FMath::Clamp(RelativeDir.Y, -1.0f, 0.0f));
	Result.R = FMath::Clamp(RelativeDir.Y, 0.0f, 1.0f);
	return Result;
}

void UHuruCharacterAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent)
{
	// Step 1: Set Turn Angle
	FRotator Delta = TargetRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	const float TurnAngle = Delta.Yaw;

	// Step 2: Choose Turn Asset based on the Turn Angle and Stance
	FHuruTurnInPlaceAsset TargetTurnAsset;
	if (Stance.Standing())
	{
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.N_TurnIP_L90
								  : TurnInPlaceValues.N_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.N_TurnIP_L180
								  : TurnInPlaceValues.N_TurnIP_R180;
		}
	}
	else
	{
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.CLF_TurnIP_L90
								  : TurnInPlaceValues.CLF_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.CLF_TurnIP_L180
								  : TurnInPlaceValues.CLF_TurnIP_R180;
		}
	}

	// Step 3: If the Target Turn Animation is not playing or set to be overriden, play the turn animation as a dynamic montage.
	if (!OverrideCurrent && IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		return;
	}
	PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName, 0.2f, 0.2f,
									  TargetTurnAsset.PlayRate * PlayRateScale, 1, 0.0f, StartTime);

	// Step 4: Scale the rotation amount (gets scaled in AnimGraph) to compensate for turn angle (If Allowed) and play rate.
	if (TargetTurnAsset.ScaleTurnAngle)
	{
		Grounded.RotationScale = (TurnAngle / TargetTurnAsset.AnimatedAngle) * TargetTurnAsset.PlayRate * PlayRateScale;
	}
	else
	{
		Grounded.RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
	}
}

FVector UHuruCharacterAnimInstance::CalculateRelativeAccelerationAmount() const
{
	// 상대 가속도 양을 계산함. 이 값은 액터 회전에 대한 현재 가속도/감속도의 양을 나타냄.
	// -1은 최대 제동 감속도를 의미하고, 1은 캐릭터 이동 컴포넌트의 최대 가속도를 의미하도록 -1에서 1의 범위로 정규화됨.
	if (FVector::DotProduct(CharacterInformation.Acceleration, CharacterInformation.Velocity) > 0.0f)
	{
		const float MaxAcc = Character->GetCharacterMovement()->GetMaxAcceleration();
		return CharacterInformation.CharacterActorRotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
	}

	const float MaxBrakingDec = Character->GetCharacterMovement()->GetMaxBrakingDeceleration();
	return
		CharacterInformation.CharacterActorRotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

float UHuruCharacterAnimInstance::CalculateStrideBlend() const
{
	// 스트라이드 블렌드를 계산함. 이 값은 블렌드 스페이스 내에서 발이 이동하는 거리를 조절하는 데 사용됨.
	// 캐릭터가 다양한 이동 속도로 걷거나 뛸 수 있게 해줌.
	// 걷기 애니메이션이나 뛰기 애니메이션을 독립적으로 블렌드하면서 애니메이션 속도를 이동 속도와 맞추게 해줘서,
	// 캐릭터가 반만 걷고 반만 뛰는 애니메이션을 재생할 필요가 없게 됨.
	// 이 커브들은 최대한의 제어를 위해 속도에 맞춰 스트라이드 양을 매핑하는 데 사용됨.
	const float CurveTime = CharacterInformation.Speed / GetOwningComponent()->GetComponentScale().Z;
	const float ClampedGait = GetAnimCurveClamped(NAME_W_Gait, -1.0, 0.0f, 1.0f);
	const float LerpedStrideBlend =
		FMath::Lerp(StrideBlend_N_Walk->GetFloatValue(CurveTime), StrideBlend_N_Run->GetFloatValue(CurveTime),
					ClampedGait);
	return FMath::Lerp(LerpedStrideBlend, StrideBlend_C_Walk->GetFloatValue(CharacterInformation.Speed),
					   GetCurveValue(NAME_BasePose_CLF));
}

float UHuruCharacterAnimInstance::CalculateWalkRunBlend() const
{
	// 걷기와 달리기 혼합을 계산함. 이 값은 블렌드 스페이스 내에서 걷기와 달리기 간의 혼합을 위해 사용됨.
	return Gait.Walking() ? 0.0f : 1.0;
}

float UHuruCharacterAnimInstance::CalculateStandingPlayRate() const
{
	// 캐릭터의 속도를 각 보행 속도에 대한 애니메이션 속도로 나누어 재생 비율을 계산함.
	// 보행 속도는 모든 이동 사이클에 존재하는 "W_Gait" 애니메이션 커브에 의해 결정되어, 현재 혼합된 애니메이션과 항상 동기화됨.
	// 또한 이 값은 보폭 블렌드와 메쉬 스케일로 나뉘어 재생 비율이 보폭이나 스케일이 작아질수록 증가하도록 함.
	const float LerpedSpeed = FMath::Lerp(CharacterInformation.Speed / Config.AnimatedWalkSpeed,
										  CharacterInformation.Speed / Config.AnimatedRunSpeed,
										  GetAnimCurveClamped(NAME_W_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpedSpeed, CharacterInformation.Speed / Config.AnimatedSprintSpeed,
												  GetAnimCurveClamped(NAME_W_Gait, -2.0f, 0.0f, 1.0f));

	return FMath::Clamp((SprintAffectedSpeed / Grounded.StrideBlend) / GetOwningComponent()->GetComponentScale().Z,
						0.0f, 3.0f);
}

float UHuruCharacterAnimInstance::CalculateDiagonalScaleAmount() const
{
	// 대각선 스케일 양을 계산함. 이 값은 발 IK 루트 본을 스케일링하여 발 IK 본이 대각선 블렌드에서 더 많은 거리를 커버하도록 함.
	// 스케일링 없이, 발은 IK 본의 선형 변환 블렌딩으로 인해 대각선 방향으로 충분히 움직이지 않음.
	// 이 커브는 값을 쉽게 매핑하는 데 사용됨.
	return DiagonalScaleAmountCurve->GetFloatValue(FMath::Abs(VelocityBlend.F + VelocityBlend.B));
}

float UHuruCharacterAnimInstance::CalculateCrouchingPlayRate() const
{
	// 앉은 플레이 속도를 계산함. 캐릭터의 속도를 애니메이션 속도로 나누어 계산함.
	// 이 값은 움직이는 동안 앉음에서 서 있음으로의 블렌드를 개선하기 위해 서 있는 플레이 속도와 분리되어야 함.
	return FMath::Clamp(
		CharacterInformation.Speed / Config.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
		GetComponentScale().Z,
		0.0f, 2.0f);
}

float UHuruCharacterAnimInstance::CalculateLandPrediction() const
{
	// 착지 예측 가중치를 계산함. 속도 방향으로 추적하여 캐릭터가 떨어지고 있는 걷기 가능한 표면을 찾아서
	// 충돌까지 걸리는 '시간' (0-1 범위, 1은 최대, 0은 착지 직전)을 가져옴.
	// 착지 예측 곡선을 사용하여 시간이 최종 가중치에 미치는 영향을 조절하여 부드러운 블렌드를 구현함.
	if (InAir.FallSpeed >= -200.0f)
	{
		return 0.0f;
	}

	const UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
	const FVector& CapsuleWorldLoc = CapsuleComp->GetComponentLocation();
	const float VelocityZ = CharacterInformation.Velocity.Z;
	FVector VelocityClamped = CharacterInformation.Velocity;
	VelocityClamped.Z = FMath::Clamp(VelocityZ, -4000.0f, -200.0f);
	VelocityClamped.Normalize();

	const FVector TraceLength = VelocityClamped * FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, -4000.0f}, {50.0f, 2000.0f}, VelocityZ);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	FHitResult HitResult;
	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(CapsuleComp->GetUnscaledCapsuleRadius(), CapsuleComp->GetUnscaledCapsuleHalfHeight());
	const bool bHit = World->SweepSingleByChannel(
		HitResult,
		CapsuleWorldLoc,
		CapsuleWorldLoc + TraceLength,
		FQuat::Identity,
	ECC_Visibility, CapsuleCollisionShape, Params);

	if (Character->GetCharacterMovement()->IsWalkable(HitResult))
	{
		return FMath::Lerp(
			LandPredictionCurve->GetFloatValue(HitResult.Time),
			0.0f,
			GetCurveValue(NAME_Mask_LandPrediction));
	}

	return 0.0f;
}

FHuruLeanAmount UHuruCharacterAnimInstance::CalculateAirLeanAmount() const
{
	// 상대 속도 방향과 양을 사용하여 공중에서 캐릭터가 얼마나 기울어야 하는지 결정함.
	// 공중에서의 기울기 곡선은 낙하 속도를 가져오며, 이를 곱셈기로 사용하여
	// 위로 이동하다가 아래로 이동할 때 기울기 방향을 부드럽게 반전시킴.
	FHuruLeanAmount CalcLeanAmount;
	const FVector& UnrotatedVel = CharacterInformation.CharacterActorRotation.UnrotateVector(
		CharacterInformation.Velocity) / 350.0f;
	FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
	InversedVect *= LeanInAirCurve->GetFloatValue(InAir.FallSpeed);
	CalcLeanAmount.LR = InversedVect.X;
	CalcLeanAmount.FB = InversedVect.Y;
	return CalcLeanAmount;
}

EHuruMovementDirection UHuruCharacterAnimInstance::CalculateMovementDirection() const
{
	// 이동 방향을 계산함. 이 값은 캐릭터가 카메라를 기준으로 이동하는 방향을 나타내며 시선 방향 / 조준 회전 모드 동안 사용됨.
	// 적절한 방향 상태로 블렌딩하기 위해 Cycle Blending 애니메이션 레이어에서 사용됨.
	if (Gait.Sprinting() || RotationMode.VelocityDirection())
	{
		return EHuruMovementDirection::Forward;
	}

	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.AimingRotation;
	Delta.Normalize();
	return UHuruMathLibrary::CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

float UHuruCharacterAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}
