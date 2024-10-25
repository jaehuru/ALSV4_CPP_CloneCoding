// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/HuruNotifyStateMovementAction.h"

#include "Character/HuruBaseCharacter.h"

void UHuruNotifyStateMovementAction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AHuruBaseCharacter* BaseCharacter = Cast<AHuruBaseCharacter>(MeshComp->GetOwner());
	if (BaseCharacter)
	{
		BaseCharacter->SetMovementAction(MovementAction);
	}
}

void UHuruNotifyStateMovementAction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AHuruBaseCharacter* BaseCharacter = Cast<AHuruBaseCharacter>(MeshComp->GetOwner());
	if (BaseCharacter && BaseCharacter->GetMovementAction() == MovementAction)
	{
		BaseCharacter->SetMovementAction(EHuruMovementAction::None);
	}
}

FString UHuruNotifyStateMovementAction::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Movement Action: "));
	Name.Append(GetEnumerationToString(MovementAction));
	return Name;
}
