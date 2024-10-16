// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/HuruGameMode.h"
#include "Character/HuruCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHuruGameMode::AHuruGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
