// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HuruBaseCharacter.h"
#include "HuruCharacter.generated.h"


UCLASS(Blueprintable, BlueprintType)
class HURU_API AHuruCharacter : public AHuruBaseCharacter
{
	GENERATED_BODY()

public:
	AHuruCharacter(const FObjectInitializer& ObjectInitialize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};

