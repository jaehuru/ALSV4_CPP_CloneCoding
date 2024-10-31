#pragma once

#include "CoreMinimal.h"
#include "HuruCharacterEnumLibrary.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Library/HuruCharacterStructLibrary.h"

#include "HuruMathLibrary.generated.h"

class UCapsuleComponent;

/**
 * Math library functions for Huru
 */
UCLASS()
class HURU_API UHuruMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static FTransform MantleComponentLocalToWorld(const FHuruComponentAndTransform& CompAndTransform);

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static FTransform TransformSub(const FTransform& T1, const FTransform& T2)
	{
		return FTransform(T1.GetRotation().Rotator() - T2.GetRotation().Rotator(),
		                  T1.GetLocation() - T2.GetLocation(), T1.GetScale3D() - T2.GetScale3D());
	}

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static FTransform TransformAdd(const FTransform& T1, const FTransform& T2)
	{
		return FTransform(T1.GetRotation().Rotator() + T2.GetRotation().Rotator(),
		                  T1.GetLocation() + T2.GetLocation(), T1.GetScale3D() + T2.GetScale3D());
	}

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static FVector GetCapsuleBaseLocation(float ZOffset, UCapsuleComponent* Capsule);

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static FVector GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset, UCapsuleComponent* Capsule);

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static bool CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TargetLocation, float HeightOffset, float RadiusOffset);

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);

	UFUNCTION(BlueprintCallable, Category = "Huru|Math Utils")
	static EHuruMovementDirection CalculateQuadrant(EHuruMovementDirection Current, float FRThreshold, float FLThreshold, float BRThreshold, float BLThreshold, float Buffer, float Angle);
};
