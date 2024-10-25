// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Library/HuruCharacterEnumLibrary.h"
#include "HuruAnimNotifyGroundedEntryState.generated.h"

/**
 * 
 */
UCLASS()
class HURU_API UHuruAnimNotifyGroundedEntryState : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EHuruGroundedEntryState GroundedEntryState = EHuruGroundedEntryState::None;
	
};
