// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Library/HuruCharacterEnumLibrary.h"
#include "HuruNotifyStateEarlyBlendOut.generated.h"

/**
 * Character early blend out anim state
 */
UCLASS()
class HURU_API UHuruNotifyStateEarlyBlendOut : public UAnimNotifyState
{
	GENERATED_BODY()

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
							const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	TObjectPtr<UAnimMontage> ThisMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	float BlendOutTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckMovementState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EHuruMovementState MovementStateEquals = EHuruMovementState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckStance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EHuruStance StanceEquals = EHuruStance::Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckMovementInput = false;
	
};
