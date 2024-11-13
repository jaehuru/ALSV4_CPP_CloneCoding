// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HuruBaseCharacter.h"
#include "HuruCharacter.generated.h"

/**
 * Specialized character class, with additional features like held object etc.
 */
UCLASS(Blueprintable, BlueprintType)
class HURU_API AHuruCharacter : public AHuruBaseCharacter
{
	GENERATED_BODY()

public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Huru|Component")
	TObjectPtr<USceneComponent> HeldObjectRoot = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Huru|Component")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Huru|Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh = nullptr;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	AHuruCharacter(const FObjectInitializer& ObjectInitializer);

	/** Implemented on BP to update held objects */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Huru|HeldObject")
	void UpdateHeldObject();

	UFUNCTION(BlueprintCallable, Category = "Huru|HeldObject")
	void ClearHeldObject();

	UFUNCTION(BlueprintCallable, Category = "Huru|HeldObject")
	void AttachToHand(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh,
					  class UClass* NewAnimClass, bool bLeftHand, FVector Offset);

	virtual void RagdollStart() override;
	
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius) override;

	virtual FTransform GetThirdPersonPivotTarget() override;

	virtual FVector GetFirstPersonCameraTarget() override;

protected:
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual void OnOverlayStateChanged(EHuruOverlayState PreviousState) override;

	/** Implement on BP to update animation states of held objects */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Huru|HeldObject")
	void UpdateHeldObjectAnimations();

private:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	bool bNeedsColorReset = false;
	
};

