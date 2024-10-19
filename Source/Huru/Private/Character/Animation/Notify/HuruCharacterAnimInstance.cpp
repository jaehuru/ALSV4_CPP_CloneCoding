// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/Animation/Notify/HuruCharacterAnimInstance.h"
#include "Character/HuruBaseCharacter.h"
//Engine
#include "GameFramework/CharacterMovementComponent.h"

static const FName NAME_BasePose_CLF(TEXT("BasePose_CLF"));
static const FName NAME_BasePose_N(TEXT("BasePose_N"));
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
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, NAME_Grounded___Slot,
									  Parameters.BlendInTime, Parameters.BlendOutTime, Parameters.PlayRate, 1,
									  0.0f, Parameters.StartTime);
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
		World->GetTimerManager().SetTimer(PlayDynamicTransitionTimer, this,
										  &UHuruCharacterAnimInstance::PlayDynamicTransitionDelay,
										  ReTriggerDelay, false);
	}
}

void UHuruCharacterAnimInstance::OnJumped()
{
}

void UHuruCharacterAnimInstance::OnPivot()
{
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
}

void UHuruCharacterAnimInstance::UpdateInAirValues(float DeltaSeconds)
{
}

void UHuruCharacterAnimInstance::UpdateRagdollValues()
{
}

void UHuruCharacterAnimInstance::RotateInPlaceCheck()
{
}

void UHuruCharacterAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
{
}

void UHuruCharacterAnimInstance::DynamicTransitionCheck()
{
}

FHuruVelocityBlend UHuruCharacterAnimInstance::CalculateVelocityBlend() const
{
}

void UHuruCharacterAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent)
{
}

FVector UHuruCharacterAnimInstance::CalculateRelativeAccelerationAmount() const
{
}

float UHuruCharacterAnimInstance::CalculateStrideBlend() const
{
}

float UHuruCharacterAnimInstance::CalculateWalkRunBlend() const
{
}

float UHuruCharacterAnimInstance::CalculateStandingPlayRate() const
{
}

float UHuruCharacterAnimInstance::CalculateDiagonalScaleAmount() const
{
}

float UHuruCharacterAnimInstance::CalculateCrouchingPlayRate() const
{
}

float UHuruCharacterAnimInstance::CalculateLandPrediction() const
{
}

FHuruLeanAmount UHuruCharacterAnimInstance::CalculateAirLeanAmount() const
{
}

EHuruMovementDirection UHuruCharacterAnimInstance::CalculateMovementDirection() const
{
}

float UHuruCharacterAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const
{
}
