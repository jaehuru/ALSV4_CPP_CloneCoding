// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/HuruNotifyStateEarlyBlendOut.h"

#include "Character/HuruBaseCharacter.h"

void UHuruNotifyStateEarlyBlendOut::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetAnimInstance())
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	AHuruBaseCharacter* OwnerCharacter = Cast<AHuruBaseCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter || !AnimInstance)
	{
		return;
	}

	bool bStopMontage = false;
	if (bCheckMovementState && OwnerCharacter->GetMovementState() == MovementStateEquals)
	{
		bStopMontage = true;
	}
	else if (bCheckStance && OwnerCharacter->GetStance() == StanceEquals)
	{
		bStopMontage = true;
	}
	else if (bCheckMovementInput && OwnerCharacter->HasMovementInput())
	{
		bStopMontage = true;
	}

	if (bStopMontage)
	{
		AnimInstance->Montage_Stop(BlendOutTime, ThisMontage);
	}
}

FString UHuruNotifyStateEarlyBlendOut::GetNotifyName_Implementation() const
{
	return FString(TEXT("Early Blend Out"));
}