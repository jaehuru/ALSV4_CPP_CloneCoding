// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/HuruAnimNotifyCameraShake.h"

void UHuruAnimNotifyCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& 
EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (OwnerPawn)
	{
		APlayerController* OwnerController = Cast<APlayerController>(OwnerPawn->GetController());
		if (OwnerController)
		{
			OwnerController->ClientStartCameraShake(ShakeClass, Scale); 
		}
	}
}
