// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/HuruMantleComponent.h"
#include "Library/HuruMathLibrary.h"
#include "Character/HuruBaseCharacter.h"
#include "Character/HuruDebugComponent.h"
//Engine
#include "Character/HuruCharacter.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

const FName NAME_MantleEnd(TEXT("MantleEnd"));
const FName NAME_MantleUpdate(TEXT("MantleUpdate"));
const FName NAME_MantleTimeline(TEXT("MantleTimeline"));

FName UHuruMantleComponent::NAME_IgnoreOnlyPawn(TEXT("IgnoreOnlyPawn"));


UHuruMantleComponent::UHuruMantleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	MantleTimeline = CreateDefaultSubobject<UTimelineComponent>(NAME_MantleTimeline);
}


void UHuruMantleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		OwnerCharacter = Cast<AHuruBaseCharacter>(GetOwner());
		if (OwnerCharacter)
		{
			AddTickPrerequisiteActor(OwnerCharacter); // 소유자 이후에 항상 틱을 수행하므로 업데이트된 값을 사용할 수 있음

			// Bindings
			FOnTimelineFloat TimelineUpdated;
			FOnTimelineEvent TimelineFinished;
			TimelineUpdated.BindUFunction(this, NAME_MantleUpdate);
			TimelineFinished.BindUFunction(this, NAME_MantleEnd);
			MantleTimeline->SetTimelineFinishedFunc(TimelineFinished);
			MantleTimeline->SetLooping(false);
			MantleTimeline->SetTimelineLengthMode(TL_TimelineLength);
			MantleTimeline->AddInterpFloat(MantleTimelineCurve, TimelineUpdated);

			OwnerCharacter->JumpPressedDelegate.AddUniqueDynamic(this, &UHuruMantleComponent::OnOwnerJumpInput);
			OwnerCharacter->RagdollStateChangedDelegate.AddUniqueDynamic(
				this, &UHuruMantleComponent::OnOwnerRagdollStateChanged);
		}
	}
	
}


void UHuruMantleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter && OwnerCharacter->GetMovementState() == EHuruMovementState::InAir)
	{
		// 움직임 입력이 눌린 상태에서 떨어지는 경우 맨틀 체크를 수행
		if (OwnerCharacter->HasMovementInput())
		{
			MantleCheck(FallingTraceSettings, EDrawDebugTrace::Type::ForOneFrame);
		}
	}
}

bool UHuruMantleComponent::MantleCheck(const FHuruMantleTraceSettings& TraceSettings, EDrawDebugTrace::Type DebugType)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	// Step 1: Trace forward to find a wall / object the character cannot walk on.
	const FVector& TraceDirection = OwnerCharacter->GetActorForwardVector();
	const FVector& CapsuleBaseLocation = UHuruMathLibrary::GetCapsuleBaseLocation(
		2.0f, OwnerCharacter->GetCapsuleComponent());
	FVector TraceStart = CapsuleBaseLocation + TraceDirection * -30.0f;
	TraceStart.Z += (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2.0f;
	const FVector TraceEnd = TraceStart + TraceDirection * TraceSettings.ReachDistance;
	const float HalfHeight = 1.0f + (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2.0f;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(TraceSettings.ForwardTraceRadius, HalfHeight);
		const bool bHit = World->SweepSingleByProfile(HitResult, TraceStart, TraceEnd, FQuat::Identity, MantleObjectDetectionProfile,
	                                                  CapsuleCollisionShape, Params);
		
	}

	if (!HitResult.IsValidBlockingHit() || OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// Not a valid surface to mantle
		return false;
	}

	if (HitResult.GetComponent() != nullptr)
	{
		UPrimitiveComponent* PrimitiveComponent = HitResult.GetComponent();
		if (PrimitiveComponent && PrimitiveComponent->GetComponentVelocity().Size() > AcceptableVelocityWhileMantling)
		{
			// The surface to mantle moves too fast
			return false;
		}
	}

	const FVector InitialTraceImpactPoint = HitResult.ImpactPoint;
	const FVector InitialTraceNormal = HitResult.ImpactNormal;

	// Step 2: Trace downward from the first trace's Impact Point and determine if the hit location is walkable.
	FVector DownwardTraceEnd = InitialTraceImpactPoint;
	DownwardTraceEnd.Z = CapsuleBaseLocation.Z;
	DownwardTraceEnd += InitialTraceNormal * -15.0f;
	FVector DownwardTraceStart = DownwardTraceEnd;
	DownwardTraceStart.Z += TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius + 1.0f;

	{
		const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceSettings.DownwardTraceRadius);
		const bool bHit = World->SweepSingleByChannel(HitResult, DownwardTraceStart, DownwardTraceEnd, FQuat::Identity,
	                                                  WalkableSurfaceDetectionChannel, SphereCollisionShape,
	                                                  Params);
		
	}


	if (!OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// Not a valid surface to mantle
		return false;
	}

	const FVector DownTraceLocation(HitResult.Location.X, HitResult.Location.Y, HitResult.ImpactPoint.Z);
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	// Step 3: Check if the capsule has room to stand at the downward trace's location.
	// If so, set that location as the Target Transform and calculate the mantle height.
	const FVector& CapsuleLocationFBase = UHuruMathLibrary::GetCapsuleLocationFromBase(
		DownTraceLocation, 2.0f, OwnerCharacter->GetCapsuleComponent());
	const bool bCapsuleHasRoom = UHuruMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
																	  CapsuleLocationFBase, 0.0f,
																	  0.0f, DebugType, HuruDebugComponent && 
																	  HuruDebugComponent->GetShowTraces());

	if (!bCapsuleHasRoom)
	{
		// Capsule doesn't have enough room to mantle
		return false;
	}

	const FTransform TargetTransform(
		(InitialTraceNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator(),
		CapsuleLocationFBase,
		FVector::OneVector);

	const float MantleHeight = (CapsuleLocationFBase - OwnerCharacter->GetActorLocation()).Z;

	// Step 4: Determine the Mantle Type by checking the movement mode and Mantle Height.
	EHuruMantleType MantleType;
	if (OwnerCharacter->GetMovementState() == EHuruMovementState::InAir)
	{
		MantleType = EHuruMantleType::FallingCatch;
	}
	else
	{
		MantleType = MantleHeight > 125.0f ? EHuruMantleType::HighMantle : EHuruMantleType::LowMantle;
	}

	// Step 5: If everything checks out, start the Mantle
	FHuruComponentAndTransform MantleWS;
	MantleWS.Component = HitComponent;
	MantleWS.Transform = TargetTransform;
	MantleStart(MantleHeight, MantleWS, MantleType);
	Server_MantleStart(MantleHeight, MantleWS, MantleType);

	return true;
}

void UHuruMantleComponent::MantleStart(float MantleHeight, const FHuruComponentAndTransform& MantleLedgeWS, EHuruMantleType MantleType)
{
	if (OwnerCharacter == nullptr || !IsValid(MantleLedgeWS.Component) || !IsValid(MantleTimeline))
	{
		return;
	}

	
	// 맨틀 중에는 틱을 비활성화합니다
	SetComponentTickEnabledAsync(false);

	// Step 1: 맨틀 자산을 가져와서 새로운 맨틀 매개변수를 설정하는 데 사용합니다
	const FHuruMantleAsset MantleAsset = GetMantleAsset(MantleType, OwnerCharacter->GetOverlayState());
	check(MantleAsset.PositionCorrectionCurve)
	
	MantleParams.AnimMontage = MantleAsset.AnimMontage;
	MantleParams.PositionCorrectionCurve = MantleAsset.PositionCorrectionCurve;
	MantleParams.StartingOffset = MantleAsset.StartingOffset;
	MantleParams.StartingPosition = FMath::GetMappedRangeValueClamped<float, float>({MantleAsset.LowHeight, MantleAsset.HighHeight},
	                                                                  {
		                                                                  MantleAsset.LowStartPosition,
		                                                                  MantleAsset.HighStartPosition
	                                                                  },
	                                                                  MantleHeight);
	MantleParams.PlayRate = FMath::GetMappedRangeValueClamped<float, float>({MantleAsset.LowHeight, MantleAsset.HighHeight},
	                                                          {MantleAsset.LowPlayRate, MantleAsset.HighPlayRate},
	                                                          MantleHeight);

	// Step 2: 월드 스페이스 타겟을 맨틀 컴포넌트의 로컬 스페이스로 변환하여 이동 객체에 사용합
	MantleLedgeLS.Component = MantleLedgeWS.Component;
	MantleLedgeLS.Transform = MantleLedgeWS.Transform * MantleLedgeWS.Component->GetComponentToWorld().Inverse();

	// Step 3: 맨틀 타겟을 설정하고 시작 오프셋을 계산합니다. (액터와 타겟 변환 사이의 오프셋 양)
	MantleTarget = MantleLedgeWS.Transform;
	MantleActualStartOffset = UHuruMathLibrary::TransformSub(OwnerCharacter->GetActorTransform(), MantleTarget);

	// Step 4: 타겟 위치에서 애니메이션 시작 오프셋을 계산
	// 실제 애니메이션이 타겟 변환에 대해 시작되는 위치
	FVector RotatedVector = MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y;
	RotatedVector.Z = MantleParams.StartingOffset.Z;
	const FTransform StartOffset(MantleTarget.Rotator(), MantleTarget.GetLocation() - RotatedVector,
	                             FVector::OneVector);
	MantleAnimatedStartOffset = UHuruMathLibrary::TransformSub(StartOffset, MantleTarget);

	// Step 5: 캐릭터 이동 모드를 지우고 이동 상태를 맨틀링으로 설정
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
	OwnerCharacter->SetMovementState(EHuruMovementState::Mantling);

	// Step 6: 맨틀 타임라인을 애니메이션과 동일한 속도로 재생되도록 설정하고,
	// 시작 위치를 뺀 Lerp/Correction 곡선과 동일한 길이로 설정합니다. 그런 다음 타임라인을 시작
	float MinTime = 0.0f;
	float MaxTime = 0.0f;
	MantleParams.PositionCorrectionCurve->GetTimeRange(MinTime, MaxTime);
	MantleTimeline->SetTimelineLength(MaxTime - MantleParams.StartingPosition);
	MantleTimeline->SetPlayRate(MantleParams.PlayRate);
	MantleTimeline->PlayFromStart();

	// Step 7: 유효한 경우 애니메이션 몽타주를 재생
	if (MantleParams.AnimMontage && OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(MantleParams.AnimMontage, MantleParams.PlayRate,
		                                                    EMontagePlayReturnType::MontageLength,
		                                                    MantleParams.StartingPosition, false);
	}
}

void UHuruMantleComponent::MantleUpdate(float BlendIn)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Step 1: 저장된 로컬 변환에서 맨틀 대상을 지속적으로 업데이트하여 이동하는 객체를 따라감
	MantleTarget = UHuruMathLibrary::MantleComponentLocalToWorld(MantleLedgeLS);

	// Step 2: 각 맨틀에 대해 설정된 위치/보정 곡선을 사용하여 위치와 보정 알파를 업데이트
	const FVector CurveVec = MantleParams.PositionCorrectionCurve
	                                     ->GetVectorValue(
		                                     MantleParams.StartingPosition + MantleTimeline->GetPlaybackPosition());
	const float PositionAlpha = CurveVec.X;
	const float XYCorrectionAlpha = CurveVec.Y;
	const float ZCorrectionAlpha = CurveVec.Z;

	// Step 3: 애니메이션 시작 위치와 대상 위치에 대해 수평 및 수직 블렌드를 독립적으로 제어하기 위해 여러 변환을 Lerp

	// 위치/수정 곡선의 Y 값을 사용하여 애니메이션 수평 및 회전 오프셋으로 블렌딩
	const FTransform TargetHzTransform(MantleAnimatedStartOffset.GetRotation(),
	                                   {
		                                   MantleAnimatedStartOffset.GetLocation().X,
		                                   MantleAnimatedStartOffset.GetLocation().Y,
		                                   MantleActualStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& HzLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetHzTransform, XYCorrectionAlpha);

	// 위치/수정 곡선의 Z 값을 사용하여 애니메이션 수직 오프셋으로 블렌딩
	const FTransform TargetVtTransform(MantleActualStartOffset.GetRotation(),
	                                   {
		                                   MantleActualStartOffset.GetLocation().X,
		                                   MantleActualStartOffset.GetLocation().Y,
		                                   MantleAnimatedStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& VtLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetVtTransform, ZCorrectionAlpha);

	const FTransform ResultTransform(HzLerpResult.GetRotation(),
	                                 {
		                                 HzLerpResult.GetLocation().X, HzLerpResult.GetLocation().Y,
		                                 VtLerpResult.GetLocation().Z
	                                 },
	                                 FVector::OneVector);

	// 위치/수정 곡선의 X 값을 사용하여 현재 블렌딩 중인 변환에서 최종 맨틀 타겟으로 블렌딩
	const FTransform& ResultLerp = UKismetMathLibrary::TLerp(
		UHuruMathLibrary::TransformAdd(MantleTarget, ResultTransform), MantleTarget,
		PositionAlpha);

	// 타임라인 곡선에서 제어되는 초기 블렌드 인으로, 액터가 중간 지점에서 위치/수정 곡선으로 블렌딩할 수 있도록 함.
	// 이를 통해 애니메이션된 맨틀보다 낮은 물체를 맨틀링할 때 발생하는 팝 현상을 방지
	const FTransform& LerpedTarget =
		UKismetMathLibrary::TLerp(UHuruMathLibrary::TransformAdd(MantleTarget, MantleActualStartOffset), ResultLerp,
		                          BlendIn);

	// Step 4: 액터의 위치와 회전을 Lerp된 타겟으로 설정
	OwnerCharacter->SetActorLocationAndTargetRotation(LerpedTarget.GetLocation(), LerpedTarget.GetRotation().Rotator());
}

void UHuruMantleComponent::MantleEnd()
{
	// 캐릭터의 이동 모드를 걷기로 설정
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	// 맨틀이 끝난 후 다시 틱을 활성화
	SetComponentTickEnabledAsync(true);
}

void UHuruMantleComponent::OnOwnerJumpInput()
{
	// 캐릭터가 특별한 맨틀을 수행할 수 있는지 확인

	if (OwnerCharacter && OwnerCharacter->GetMovementAction() == EHuruMovementAction::None)
	{
		if (OwnerCharacter->GetMovementState() == EHuruMovementState::Grounded)
		{
			if (OwnerCharacter->HasMovementInput())
			{
				MantleCheck(GroundedTraceSettings, EDrawDebugTrace::Type::ForDuration);
			}
		}
		else if (OwnerCharacter->GetMovementState() == EHuruMovementState::InAir)
		{
			MantleCheck(FallingTraceSettings, EDrawDebugTrace::Type::ForDuration);
		}
	}
}

void UHuruMantleComponent::OnOwnerRagdollStateChanged(bool bRagdollState)
{
	// 소유자가 래그돌 상태로 들어가면 즉시 맨틀을 중단
	if (bRagdollState)
	{
		MantleTimeline->Stop();
	}
}

void UHuruMantleComponent::Server_MantleStart_Implementation(float MantleHeight, const FHuruComponentAndTransform& MantleLedgeWS,
	EHuruMantleType MantleType)
{
	Multicast_MantleStart(MantleHeight, MantleLedgeWS, MantleType);
}

void UHuruMantleComponent::Multicast_MantleStart_Implementation(float MantleHeight, const FHuruComponentAndTransform& MantleLedgeWS,
	EHuruMantleType MantleType)
{
	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		MantleStart(MantleHeight, MantleLedgeWS, MantleType);
	}
}



