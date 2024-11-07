// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/Component/HuruMantleComponent.h"
#include "Character/HuruCharacter.h"
#include "Library/HuruMathLibrary.h"
#include "Character/Component/HuruDebugComponent.h"
//Engine
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
			HuruDebugComponent = OwnerCharacter->FindComponentByClass<UHuruDebugComponent>();

			AddTickPrerequisiteActor(OwnerCharacter); // 항상 소유자 뒤에 틱을 호출하므로, 업데이트된 값을 사용함

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
		// 이동 입력이 눌린 상태에서 떨어지면 맨틀 체크 수행
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

	// Step 1: 캐릭터가 걸을 수 없는 벽/오브젝트를 찾기 위해 앞쪽으로 트레이스 수행
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

		if (HuruDebugComponent && HuruDebugComponent->GetShowTraces())
		{
			UHuruDebugComponent::DrawDebugCapsuleTraceSingle(World,
			                                                TraceStart,
			                                                TraceEnd,
			                                                CapsuleCollisionShape,
			                                                DebugType,
			                                                bHit,
			                                                HitResult,
			                                                FLinearColor::Black,
			                                                FLinearColor::Black,
			                                                1.0f);
		}
	}

	if (!HitResult.IsValidBlockingHit() || OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// 매끈하지 않은 표면, 매늘링할 수 없음
		return false;
	}

	if (HitResult.GetComponent() != nullptr)
	{
		UPrimitiveComponent* PrimitiveComponent = HitResult.GetComponent();
		if (PrimitiveComponent && PrimitiveComponent->GetComponentVelocity().Size() > AcceptableVelocityWhileMantling)
		{
			// 매늘링할 표면이 너무 빠르게 이동함
			return false;
		}
	}

	const FVector InitialTraceImpactPoint = HitResult.ImpactPoint;
	const FVector InitialTraceNormal = HitResult.ImpactNormal;

	// Step 2: 첫 번째 트레이스의 충돌 지점에서 아래로 트레이스를 하여, 충돌 위치가 걸을 수 있는지 확인
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

		if (HuruDebugComponent && HuruDebugComponent->GetShowTraces())
		{
			UHuruDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               SphereCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Black,
			                                               FLinearColor::Black,
			                                               1.0f);
		}
	}


	if (!OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// 유효하지 않은 표면으로, 맨틀링을 할 수 없음
		return false;
	}

	const FVector DownTraceLocation(HitResult.Location.X, HitResult.Location.Y, HitResult.ImpactPoint.Z);
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	// Step 3: 캡슐이 내려간 추적 위치에서 서 있을 수 있는지 확인
	// 만약 그렇다면, 그 위치를 목표 변환으로 설정하고 맨틀링 높이를 계산
	const FVector& CapsuleLocationFBase = UHuruMathLibrary::GetCapsuleLocationFromBase(
		DownTraceLocation, 2.0f, OwnerCharacter->GetCapsuleComponent());
	const bool bCapsuleHasRoom = UHuruMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
	                                                                  CapsuleLocationFBase, 0.0f,
	                                                                  0.0f, DebugType, HuruDebugComponent && HuruDebugComponent->GetShowTraces());

	if (!bCapsuleHasRoom)
	{
		// 캡슐이 맨틀링할 충분한 공간이 없음
		return false;
	}

	const FTransform TargetTransform(
		(InitialTraceNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator(),
		CapsuleLocationFBase,
		FVector::OneVector);

	const float MantleHeight = (CapsuleLocationFBase - OwnerCharacter->GetActorLocation()).Z;

	// Step 4: 이동 모드와 맨틀링 높이를 확인하여 맨틀링 유형을 결정
	EHuruMantleType MantleType;
	if (OwnerCharacter->GetMovementState() == EHuruMovementState::InAir)
	{
		MantleType = EHuruMantleType::FallingCatch;
	}
	else
	{
		MantleType = MantleHeight > 125.0f ? EHuruMantleType::HighMantle : EHuruMantleType::LowMantle;
	}

	// Step 5: 모든 것이 확인되면 맨틀링 시작
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

	// 손에 무기를 들었을때 무기를 없애는
	/*if (MantleType != EHuruMantleType::LowMantle && OwnerCharacter->IsA(AHuruCharacter::StaticClass()))
	{
		Cast<AHuruCharacter>(OwnerCharacter)->ClearHeldObject();
	}*/

	// 맨틀링 중에는 틱을 비활성화
	SetComponentTickEnabledAsync(false);

	// Step 1: 맨틀 자산을 가져와 새로운 맨틀 파라미터를 설정
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

	// Step 2: 월드 공간 타겟을 맨틀 컴포넌트의 로컬 공간으로 변환하여 객체 이동에 사용
	MantleLedgeLS.Component = MantleLedgeWS.Component;
	MantleLedgeLS.Transform = MantleLedgeWS.Transform * MantleLedgeWS.Component->GetComponentToWorld().Inverse();

	// Step 3: 맨틀 타겟을 설정하고 시작 오프셋 계산
	// (액터와 타겟 변환 간의 오프셋 양).
	MantleTarget = MantleLedgeWS.Transform;
	MantleActualStartOffset = UHuruMathLibrary::TransformSub(OwnerCharacter->GetActorTransform(), MantleTarget);

	// Step 4: 타겟 위치에서 애니메이션 시작 오프셋 계산
	// 이는 실제 애니메이션이 타겟 변환을 기준으로 시작하는 위치가 된다.
	FVector RotatedVector = MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y;
	RotatedVector.Z = MantleParams.StartingOffset.Z;
	const FTransform StartOffset(MantleTarget.Rotator(), MantleTarget.GetLocation() - RotatedVector,
	                             FVector::OneVector);
	MantleAnimatedStartOffset = UHuruMathLibrary::TransformSub(StartOffset, MantleTarget);

	// Step 5: 캐릭터의 이동 모드를 초기화하고 이동 상태를 맨틀링으로 설정
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
	OwnerCharacter->SetMovementState(EHuruMovementState::Mantling);

	// Step 6: 맨틀 타임라인을 설정하여 시작 위치를 제외한 Lerp/Correction 곡선의 길이와 동일하게 설정하고,
	// 애니메이션 속도와 동일한 속도로 재생되도록 한다.
	// 그런 다음 타임라인을 시작한다.
	float MinTime = 0.0f;
	float MaxTime = 0.0f;
	MantleParams.PositionCorrectionCurve->GetTimeRange(MinTime, MaxTime);
	MantleTimeline->SetTimelineLength(MaxTime - MantleParams.StartingPosition);
	MantleTimeline->SetPlayRate(MantleParams.PlayRate);
	MantleTimeline->PlayFromStart();

	// Step 7: 애니메이션 몽타주가 유효한 경우, 이를 재생한다.
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

	// Step 1: 이동 중인 객체를 따라갈 수 있도록 저장된 로컬 변환에서 지속적으로 맨틀 타겟을 업데이트한다.
	MantleTarget = UHuruMathLibrary::MantleComponentLocalToWorld(MantleLedgeLS);

	// Step 2: 각 맨틀에 설정된 위치/보정 곡선을 사용하여 위치와 보정 알파를 업데이트한다.
	const FVector CurveVec = MantleParams.PositionCorrectionCurve
	                                     ->GetVectorValue(
		                                     MantleParams.StartingPosition + MantleTimeline->GetPlaybackPosition());
	const float PositionAlpha = CurveVec.X;
	const float XYCorrectionAlpha = CurveVec.Y;
	const float ZCorrectionAlpha = CurveVec.Z;

	// Step 3: 애니메이션 시작 위치와 목표 위치에 대해 수평 및 수직 블렌드를 독립적으로 제어할 수 있도록
	// 여러 변환을 Lerp로 결합한다.

	// Position/Correction Curve의 Y 값을 사용하여 애니메이션의 수평 및 회전 오프셋을 블렌딩한다.
	const FTransform TargetHzTransform(MantleAnimatedStartOffset.GetRotation(),
	                                   {
		                                   MantleAnimatedStartOffset.GetLocation().X,
		                                   MantleAnimatedStartOffset.GetLocation().Y,
		                                   MantleActualStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& HzLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetHzTransform, XYCorrectionAlpha);

	// Position/Correction Curve의 Z 값을 사용하여 애니메이션의 수직 오프셋을 블렌딩한다.
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
	
	// 현재 블렌딩 중인 변환에서 Position/Correction Curve의 X 값을 사용하여 최종 맨틀 타겟으로 블렌딩한다.
	const FTransform& ResultLerp = UKismetMathLibrary::TLerp(
		UHuruMathLibrary::TransformAdd(MantleTarget, ResultTransform), MantleTarget,
		PositionAlpha);

	// 초기 블렌드 인 (타임라인 커브에서 제어됨)으로, 애니메이션 맨틀보다 낮은 물체를 맨틀링 할 때 팝을 방지하기 위해
	// 액터가 Position/Correction 커브의 중간 지점에서 블렌딩되도록 한다.
	const FTransform& LerpedTarget =
		UKismetMathLibrary::TLerp(UHuruMathLibrary::TransformAdd(MantleTarget, MantleActualStartOffset), ResultLerp,
		                          BlendIn);

	// Step 4: 액터의 위치와 회전을 Lerp된 타겟으로 설정한다.
	OwnerCharacter->SetActorLocationAndTargetRotation(LerpedTarget.GetLocation(), LerpedTarget.GetRotation().Rotator());
}

void UHuruMantleComponent::MantleEnd()
{
	// 캐릭터 이동 모드를 걷기로 설정한다.
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		// 손에 무기를 들었을때 다시 무기가 보이게하는
		/*if (OwnerCharacter->IsA(AHuruCharacter::StaticClass()))
		{
			Cast<AHuruCharacter>(OwnerCharacter)->UpdateHeldObject();
		}*/
	}

	// 맨틀이 끝난 후, 다시 틱을 활성화한다.
	SetComponentTickEnabledAsync(true);
}

void UHuruMantleComponent::OnOwnerJumpInput()
{
	// 캐릭터가 특별한 맨틀을 할 수 있는지 확인한다.

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
	// 소유자가 래그돌 상태로 전환되는 경우, 즉시 맨틀을 중지한다.
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

