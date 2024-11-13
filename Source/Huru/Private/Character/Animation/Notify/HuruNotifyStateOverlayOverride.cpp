// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/HuruNotifyStateOverlayOverride.h"

#include "Character/HuruBaseCharacter.h"


void UHuruNotifyStateOverlayOverride::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AHuruBaseCharacter* Character = Cast<AHuruBaseCharacter>(MeshComp->GetOwner());
	if (Character)
	{
		Character->SetOverlayOverrideState(OverlayOverrideState);
	}
}

void UHuruNotifyStateOverlayOverride::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AHuruBaseCharacter* Character = Cast<AHuruBaseCharacter>(MeshComp->GetOwner());
	if (Character)
	{
		Character->SetOverlayOverrideState(0);
	}
}

FString UHuruNotifyStateOverlayOverride::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Overlay Override State: "));
	Name.Append(FString::FromInt(OverlayOverrideState));
	return Name;
}
