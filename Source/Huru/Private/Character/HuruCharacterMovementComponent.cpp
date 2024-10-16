// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HuruCharacterMovementComponent.h"

void UHuruCharacterMovementComponent::SetMovementSettings(FHuruMovementSettings NewMovementSettings)
{
	// 소유자(플레이어 또는 캐릭터)로부터 현재 이동 설정을 설정하다
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}
