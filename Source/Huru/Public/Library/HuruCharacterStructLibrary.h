#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Library/HuruCharacterEnumLibrary.h"

#include "HuruCharacterStructLibrary.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FHuruMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;

	float GetSpeedForGait(const EHuruGait Gait) const
	{
		switch (Gait)
		{
		case EHuruGait::Running:
			return RunSpeed;
		case EHuruGait::Sprinting:
			return SprintSpeed;
		case EHuruGait::Walking:
			return WalkSpeed;
		default:
			return RunSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FHuruMovementStanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FHuruMovementStateSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementStanceSettings VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementStanceSettings LookingDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementStanceSettings Aiming;
};