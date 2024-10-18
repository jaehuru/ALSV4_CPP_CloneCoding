// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HuruCharacterMovementComponent.h"
#include "Character/HuruBaseCharacter.h"

#include "Curves/CurveVector.h"


void UHuruCharacterMovementComponent::SetMovementSettings(FHuruMovementSettings NewMovementSettings)
{
	// 소유자(플레이어 또는 캐릭터)로부터 현재 이동 설정을 설정하다
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UHuruCharacterMovementComponent::SetAllowedGait(EHuruGait NewAllowedGait)
{
	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}

void UHuruCharacterMovementComponent::Server_SetAllowedGait_Implementation(EHuruGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}
