// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/HuruPlayerCameraBehavior.h"
#include "Character/HuruBaseCharacter.h"

void UHuruPlayerCameraBehavior::SetRotationMode(EHuruRotationMode RotationMode)
{
	bVelocityDirection = RotationMode == EHuruRotationMode::VelocityDirection;
	bLookingDirection = RotationMode == EHuruRotationMode::LookingDirection;
	bAiming = RotationMode == EHuruRotationMode::Aiming;
}
