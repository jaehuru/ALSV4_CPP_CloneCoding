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

	UPROPERTY()
	EHuruGait AllowedGait = EHuruGait::Walking;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Movement System")
	FHuruMovementSettings CurrentMovementSettings;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FHuruMovementSettings NewMovementSettings);

	/** 최대 걷기 속도를 설정함 (소유 클라이언트에서 호출됨) */
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetAllowedGait(EHuruGait NewAllowedGait);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetAllowedGait(EHuruGait NewAllowedGait);
};
