// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Library/HuruCharacterStructLibrary.h"

#include "HuruCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class HURU_API UHuruCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	/** 이동 설정 변수*/
	UPROPERTY()
	uint8 bRequestMovementSettingsChange = 1;
	
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Movement System")
	FHuruMovementSettings CurrentMovementSettings;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FHuruMovementSettings NewMovementSettings);
};
