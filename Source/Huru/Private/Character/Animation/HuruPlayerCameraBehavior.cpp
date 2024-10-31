// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/Animation/HuruPlayerCameraBehavior.h"
#include "Character/HuruBaseCharacter.h"
//Engine

void UHuruPlayerCameraBehavior::SetRotationMode(EHuruRotationMode RotationMode)
{
	bVelocityDirection = RotationMode == EHuruRotationMode::VelocityDirection;
	bLookingDirection = RotationMode == EHuruRotationMode::LookingDirection;
	bAiming = RotationMode == EHuruRotationMode::Aiming;
}
