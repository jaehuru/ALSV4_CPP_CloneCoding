//Huru
#include "Library/HuruMathLibrary.h"
#include "Library/HuruCharacterStructLibrary.h"
//Engine
#include "Components/CapsuleComponent.h"

FTransform UHuruMathLibrary::MantleComponentLocalToWorld(const FHuruComponentAndTransform& CompAndTransform)
{
	const FTransform& InverseTransform = CompAndTransform.Component->GetComponentToWorld().Inverse();
	const FVector Location = InverseTransform.InverseTransformPosition(CompAndTransform.Transform.GetLocation());
	const FQuat Quat = InverseTransform.InverseTransformRotation(CompAndTransform.Transform.GetRotation());
	const FVector Scale = InverseTransform.InverseTransformPosition(CompAndTransform.Transform.GetScale3D());
	return {Quat, Location, Scale};
}

FVector UHuruMathLibrary::GetCapsuleBaseLocation(float ZOffset, UCapsuleComponent* Capsule)
{
	return Capsule->GetComponentLocation() -
		Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + ZOffset);
}

FVector UHuruMathLibrary::GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset, UCapsuleComponent* Capsule)
{
	BaseLocation.Z += Capsule->GetScaledCapsuleHalfHeight() + ZOffset;
	return BaseLocation;
}

bool UHuruMathLibrary::CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TargetLocation, float HeightOffset, float RadiusOffset,
	EDrawDebugTrace::Type DebugType, bool DrawDebugTrace)
{
	// 캡슐이 목표 위치에 있을 공간이 있는지 확인하기 위해 트레이스를 수행함.
	const float ZTarget = Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() - RadiusOffset + HeightOffset;
	FVector TraceStart = TargetLocation;
	TraceStart.Z += ZTarget;
	FVector TraceEnd = TargetLocation;
	TraceEnd.Z -= ZTarget;
	const float Radius = Capsule->GetUnscaledCapsuleRadius() + RadiusOffset;

	const UWorld* World = Capsule->GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Capsule->GetOwner());

	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(Radius),
		Params);

	
	return !(HitResult.bBlockingHit || HitResult.bStartPenetrating);
}

bool UHuruMathLibrary::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	if (IncreaseBuffer)
	{
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
}

EHuruMovementDirection UHuruMathLibrary::CalculateQuadrant(EHuruMovementDirection Current, float FRThreshold, float FLThreshold, float BRThreshold, float BLThreshold, float Buffer, float Angle)
{
	// 입력 각도를 받아 해당 사분면(방향)을 결정함.
	// 현재 Movement Direction을 사용하여 각 사분면의 각도 범위에 대한 버퍼를 증가시키거나 감소시킴.
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer,
					 Current != EHuruMovementDirection::Forward && Current != EHuruMovementDirection::Backward))
	{
		return EHuruMovementDirection::Forward;
	}

	if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer,
					 Current != EHuruMovementDirection::Right && Current != EHuruMovementDirection::Left))
	{
		return EHuruMovementDirection::Right;
	}

	if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer,
					 Current != EHuruMovementDirection::Right && Current != EHuruMovementDirection::Left))
	{
		return EHuruMovementDirection::Left;
	}

	return EHuruMovementDirection::Backward;
}

