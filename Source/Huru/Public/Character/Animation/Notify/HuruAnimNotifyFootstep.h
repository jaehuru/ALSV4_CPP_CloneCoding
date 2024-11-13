// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Library/HuruCharacterEnumLibrary.h"
#include "HuruAnimNotifyFootstep.generated.h"

/**
 * 
 */
UCLASS()
class HURU_API UHuruAnimNotifyFootstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<UDataTable> HitDataTable;

	static FName NAME_Foot_R;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FName FootSocketName = NAME_Foot_R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceLength = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bSpawnDecal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bMirrorDecalX = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bMirrorDecalY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bMirrorDecalZ = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bSpawnSound = true;

	static FName NAME_FootstepType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName SoundParameterName = NAME_FootstepType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	EHuruFootstepType FootstepType = EHuruFootstepType::Step;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bOverrideMaskCurve = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	bool bSpawnNiagara = false;

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
	
};
