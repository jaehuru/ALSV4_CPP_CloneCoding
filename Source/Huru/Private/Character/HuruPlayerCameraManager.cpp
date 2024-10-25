// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/HuruPlayerCameraManager.h"
#include "Character/HuruBaseCharacter.h"
#include "Character/Animation/HuruPlayerCameraBehavior.h"
//Engine
#include "Kismet/KismetMathLibrary.h"

const FName NAME_CameraBehavior(TEXT("CameraBehavior"));
const FName NAME_CameraOffset_X(TEXT("CameraOffset_X"));
const FName NAME_CameraOffset_Y(TEXT("CameraOffset_Y"));
const FName NAME_CameraOffset_Z(TEXT("CameraOffset_Z"));
const FName NAME_Override_Debug(TEXT("Override_Debug"));
const FName NAME_PivotLagSpeed_X(TEXT("PivotLagSpeed_X"));
const FName NAME_PivotLagSpeed_Y(TEXT("PivotLagSpeed_Y"));
const FName NAME_PivotLagSpeed_Z(TEXT("PivotLagSpeed_Z"));
const FName NAME_PivotOffset_X(TEXT("PivotOffset_X"));
const FName NAME_PivotOffset_Y(TEXT("PivotOffset_Y"));
const FName NAME_PivotOffset_Z(TEXT("PivotOffset_Z"));
const FName NAME_RotationLagSpeed(TEXT("RotationLagSpeed"));
const FName NAME_Weight_FirstPerson(TEXT("Weight_FirstPerson"));

AHuruPlayerCameraManager::AHuruPlayerCameraManager()
{
	CameraBehavior = CreateDefaultSubobject<USkeletalMeshComponent>(NAME_CameraBehavior);
	CameraBehavior->SetupAttachment(GetRootComponent());
	CameraBehavior->bHiddenInGame = true;
}

void AHuruPlayerCameraManager::OnPossess(AHuruBaseCharacter* NewCharacter)
{
	// 플레이어 컨트롤러가 새로운 캐릭터를 소유할 때 '제어된 폰'을 설정함 (플레이어 컨트롤러에서 호출됨)
	check(NewCharacter);
	ControlledCharacter = NewCharacter;

	// 카메라 행동 애니메이션 블루프린트에서 참조를 업데이트
	UHuruPlayerCameraBehavior* CastedBehv = Cast<UHuruPlayerCameraBehavior>(CameraBehavior->GetAnimInstance());
	if (CastedBehv)
	{
		NewCharacter->SetCameraBehavior(CastedBehv);
		CastedBehv->MovementState = NewCharacter->GetMovementState();
		CastedBehv->MovementAction = NewCharacter->GetMovementAction();
		CastedBehv->bRightShoulder = NewCharacter->IsRightShoulder();
		CastedBehv->Gait = NewCharacter->GetGait();
		CastedBehv->SetRotationMode(NewCharacter->GetRotationMode());
		CastedBehv->Stance = NewCharacter->GetStance();
		CastedBehv->ViewMode = NewCharacter->GetViewMode();
	}

	// 초기 위치
	const FVector& TPSLoc = ControlledCharacter->GetThirdPersonPivotTarget().GetLocation();
	SetActorLocation(TPSLoc);
	SmoothedPivotTarget.SetLocation(TPSLoc);
	
}

float AHuruPlayerCameraManager::GetCameraBehaviorParam(FName CurveName) const
{
	UAnimInstance* Inst = CameraBehavior->GetAnimInstance();
	if (Inst)
	{
		return Inst->GetCurveValue(CurveName);
	}
	return 0.0f;
}

void AHuruPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	// 기본 클래스에서 부분적으로 가져옴

	if (OutVT.Target)
	{
		FVector OutLocation;
		FRotator OutRotation;
		float OutFOV;

		if (OutVT.Target->IsA<AHuruBaseCharacter>())
		{
			if (CustomCameraBehavior(DeltaTime, OutLocation, OutRotation, OutFOV))
			{
				OutVT.POV.Location = OutLocation;
				OutVT.POV.Rotation = OutRotation;
				OutVT.POV.FOV = OutFOV;
			}
			else
			{
				OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
			}
		}
		else
		{
			OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
		}
	}
}

FVector AHuruPlayerCameraManager::CalculateAxisIndependentLag(FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation,
	FVector LagSpeeds, float DeltaTime)
{
	CameraRotation.Roll = 0.0f;
	CameraRotation.Pitch = 0.0f;
	const FVector UnrotatedCurLoc = CameraRotation.UnrotateVector(CurrentLocation);
	const FVector UnrotatedTargetLoc = CameraRotation.UnrotateVector(TargetLocation);

	const FVector ResultVector(
		FMath::FInterpTo(UnrotatedCurLoc.X, UnrotatedTargetLoc.X, DeltaTime, LagSpeeds.X),
		FMath::FInterpTo(UnrotatedCurLoc.Y, UnrotatedTargetLoc.Y, DeltaTime, LagSpeeds.Y),
		FMath::FInterpTo(UnrotatedCurLoc.Z, UnrotatedTargetLoc.Z, DeltaTime, LagSpeeds.Z));

	return CameraRotation.RotateVector(ResultVector);
}

bool AHuruPlayerCameraManager::CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV)
{
	if (!ControlledCharacter)
	{
		return false;
	}

	// Step 1: 카메라 인터페이스를 통해 CharacterBP로부터 카메라 파라미터를 가져옴.
	const FTransform& PivotTarget = ControlledCharacter->GetThirdPersonPivotTarget();
	const FVector& FPTarget = ControlledCharacter->GetFirstPersonCameraTarget();
	float TPFOV = 90.0f;
	float FPFOV = 90.0f;
	bool bRightShoulder = false;
	ControlledCharacter->GetCameraParameters(TPFOV, FPFOV, bRightShoulder);

	// Step 2: 목표 카메라 회전을 계산함. 제어 회전을 사용하고, 부드러운 카메라 회전을 위해 보간함.
	const FRotator& InterpResult = FMath::RInterpTo(GetCameraRotation(),
	                                                GetOwningPlayerController()->GetControlRotation(), DeltaTime,
	                                                GetCameraBehaviorParam(NAME_RotationLagSpeed));

	TargetCameraRotation = UKismetMathLibrary::RLerp(InterpResult, DebugViewRotation,
	                                                 GetCameraBehaviorParam(TEXT("Override_Debug")), true);

	// Step 3: 부드러운 피벗 타겟(주황색 구)을 계산함. 3인칭 피벗 타겟(초록색 구)을 가져오고, 최대 제어를 위해 축 독립적 지연을 사용하여 보간함.
	const FVector LagSpd(GetCameraBehaviorParam(NAME_PivotLagSpeed_X),
	                     GetCameraBehaviorParam(NAME_PivotLagSpeed_Y),
	                     GetCameraBehaviorParam(NAME_PivotLagSpeed_Z));

	const FVector& AxisIndpLag = CalculateAxisIndependentLag(SmoothedPivotTarget.GetLocation(),
	                                                         PivotTarget.GetLocation(), TargetCameraRotation, LagSpd,
	                                                         DeltaTime);

	SmoothedPivotTarget.SetRotation(PivotTarget.GetRotation());
	SmoothedPivotTarget.SetLocation(AxisIndpLag);
	SmoothedPivotTarget.SetScale3D(FVector::OneVector);

	// Step 4: 피벗 위치(파란색 구)를 계산합니다. 부드러운 피벗 타겟을 가져오고, 추가 카메라 제어를 위해 로컬 오프셋을 적용함.
	PivotLocation =
		SmoothedPivotTarget.GetLocation() +
		UKismetMathLibrary::GetForwardVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(
			NAME_PivotOffset_X) +
		UKismetMathLibrary::GetRightVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(
			NAME_PivotOffset_Y) +
		UKismetMathLibrary::GetUpVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(
			NAME_PivotOffset_Z);

	// Step 5: 목표 카메라 위치를 계산함. 피벗 위치를 가져오고 카메라 상대 오프셋을 적용함.
	TargetCameraLocation = UKismetMathLibrary::VLerp(
		PivotLocation +
		UKismetMathLibrary::GetForwardVector(TargetCameraRotation) * GetCameraBehaviorParam(
			NAME_CameraOffset_X) +
		UKismetMathLibrary::GetRightVector(TargetCameraRotation) * GetCameraBehaviorParam(NAME_CameraOffset_Y)
		+
		UKismetMathLibrary::GetUpVector(TargetCameraRotation) * GetCameraBehaviorParam(NAME_CameraOffset_Z),
		PivotTarget.GetLocation() + DebugViewOffset,
		GetCameraBehaviorParam(NAME_Override_Debug));

	// Step 6: 카메라와 캐릭터 사이에 있는 오브젝트를 추적하여 보정 오프셋을 적용함. 추적 원점은 카메라 인터페이스를 통해 Character BP 내에서 설정됨. 일반적인 스프링 암과 유사하게 작동하지만 피벗에 관계없이 다른 추적 원점을 허용할 수 있음
	FVector TraceOrigin;
	float TraceRadius;
	ECollisionChannel TraceChannel = ControlledCharacter->GetThirdPersonTraceParams(TraceOrigin, TraceRadius);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ControlledCharacter);

	FHitResult HitResult;
	const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceRadius);
	const bool bHit = World->SweepSingleByChannel(HitResult, TraceOrigin, TargetCameraLocation, FQuat::Identity,
	                                              TraceChannel, SphereCollisionShape, Params);

	if (HitResult.IsValidBlockingHit())
	{
		TargetCameraLocation += HitResult.Location - HitResult.TraceEnd;
	}

	// Step 7: 1인칭 오버라이드를 보간하고 목표 카메라 파라미터를 반환함
	FTransform TargetCameraTransform(TargetCameraRotation, TargetCameraLocation, FVector::OneVector);
	FTransform FPTargetCameraTransform(TargetCameraRotation, FPTarget, FVector::OneVector);

	const FTransform& MixedTransform = UKismetMathLibrary::TLerp(TargetCameraTransform, FPTargetCameraTransform,
	                                                             GetCameraBehaviorParam(
		                                                             NAME_Weight_FirstPerson));

	const FTransform& TargetTransform = UKismetMathLibrary::TLerp(MixedTransform,
	                                                              FTransform(DebugViewRotation, TargetCameraLocation,
	                                                                         FVector::OneVector),
	                                                              GetCameraBehaviorParam(
		                                                              NAME_Override_Debug));

	Location = TargetTransform.GetLocation();
	Rotation = TargetTransform.Rotator();
	FOV = FMath::Lerp(TPFOV, FPFOV, GetCameraBehaviorParam(NAME_Weight_FirstPerson));

	return true;
}

