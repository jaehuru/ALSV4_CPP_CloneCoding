// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "HuruPlayerCameraManager.generated.h"

class UHuruDebugComponent;
class AHuruBaseCharacter;
/**
 * Player camera manager class
 */
UCLASS(Blueprintable, BlueprintType)
class HURU_API AHuruPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Huru|Camera")
	TObjectPtr<AHuruBaseCharacter> ControlledCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Huru|Camera")
	TObjectPtr<USkeletalMeshComponent> CameraBehavior = nullptr;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	AHuruPlayerCameraManager();
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Camera")
	void OnPossess(AHuruBaseCharacter* NewCharacter);

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera")
	float GetCameraBehaviorParam(FName CurveName) const;

	/** 디버그 로직이 블루프린트에서 구현됨 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "ALS|Camera")
	void DrawDebugTargets(FVector PivotTargetLocation);

protected:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Huru|Camera")
	FVector RootLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Huru|Camera")
	FTransform SmoothedPivotTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Huru|Camera")
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Huru|Camera")
	FVector TargetCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Huru|Camera")
	FRotator TargetCameraRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Camera")
	FRotator DebugViewRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Camera")
	FVector DebugViewOffset;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera")
	static FVector CalculateAxisIndependentLag(
		FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeeds, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera")
	bool CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV);

private:
	UPROPERTY()
	TObjectPtr<UHuruDebugComponent> HuruDebugComponent = nullptr;
	
};
