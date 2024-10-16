// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "HuruPlayerCameraManager.generated.h"

class AHuruBaseCharacter;
/**
 * Player camera manager class
 */
UCLASS()
class HURU_API AHuruPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Huru|Camera")
	void OnPossess(AHuruBaseCharacter* NewCharacter);
};
