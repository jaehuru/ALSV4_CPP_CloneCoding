// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/HuruAnimNotifyGroundedEntryState.h"
#include "Character/HuruBaseCharacter.h"


void UHuruAnimNotifyGroundedEntryState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const 
                                               FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AHuruBaseCharacter* Character = Cast<AHuruBaseCharacter>(MeshComp->GetOwner());
	if (Character)
	{
		Character->SetGroundedEntryState(GroundedEntryState);
	}
}

FString UHuruAnimNotifyGroundedEntryState::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Grounded Entry State: "));
	Name.Append(GetEnumerationToString(GroundedEntryState));
	return Name;
}
