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

	class HURU_API FSavedMove_My : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
								class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Walk Speed Update
		uint8 bSavedRequestMovementSettingsChange : 1;
		EHuruGait SavedAllowedGait = EHuruGait::Walking;
	};

	class HURU_API FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};
	
	
public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	/** 이동 설정 변수*/
	UPROPERTY()
	uint8 bRequestMovementSettingsChange;

	UPROPERTY()
	EHuruGait AllowedGait;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Movement System")
	FHuruMovementSettings CurrentMovementSettings;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UHuruCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Movement Settings Override
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	
	// Set Movement Curve (Called in every instance)
	float GetMappedSpeed() const;
	
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FHuruMovementSettings NewMovementSettings);

	/** 최대 걷기 속도를 설정함 (소유 클라이언트에서 호출됨) */
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetAllowedGait(EHuruGait NewAllowedGait);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetAllowedGait(EHuruGait NewAllowedGait);
};
