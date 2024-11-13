// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/Animation/Notify/HuruAnimNotifyFootstep.h"
#include "Library/HuruCharacterStructLibrary.h"
//Engine
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

const FName NAME_Mask_FootstepSound(TEXT("Mask_FootstepSound"));

FName UHuruAnimNotifyFootstep::NAME_FootstepType(TEXT("FootstepType"));
FName UHuruAnimNotifyFootstep::NAME_Foot_R(TEXT("Foot_R"));

void UHuruAnimNotifyFootstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* MeshOwner = MeshComp->GetOwner();
	if (!MeshOwner)
	{
		return;
	}

	if (HitDataTable)
	{
		UWorld* World = MeshComp->GetWorld();
		check(World);

		const FVector FootLocation = MeshComp->GetSocketLocation(FootSocketName);
		const FRotator FootRotation = MeshComp->GetSocketRotation(FootSocketName);
		const FVector TraceEnd = FootLocation - MeshOwner->GetActorUpVector() * TraceLength;

		FHitResult Hit;

		if (UKismetSystemLibrary::LineTraceSingle(MeshOwner /*used by bIgnoreSelf*/, FootLocation, TraceEnd, TraceChannel, true /*bTraceComplex*/, MeshOwner->Children,
		                                          DrawDebugType, Hit, true /*bIgnoreSelf*/))
		{
			if (!Hit.PhysMaterial.Get())
			{
				return;
			}

			const EPhysicalSurface SurfaceType = Hit.PhysMaterial.Get()->SurfaceType;

			check(IsInGameThread());
			checkNoRecursion();
			static TArray<FHuruHitFX*> HitFXRows;
			HitFXRows.Reset();

			HitDataTable->GetAllRows<FHuruHitFX>(FString(), HitFXRows);

			FHuruHitFX* HitFX = nullptr;
			if (auto FoundResult = HitFXRows.FindByPredicate([&](const FHuruHitFX* Value)
			{
				return SurfaceType == Value->SurfaceType;
			}))
			{
				HitFX = *FoundResult;
			}
			else if (auto DefaultResult = HitFXRows.FindByPredicate([&](const FHuruHitFX* Value)
			{
				return EPhysicalSurface::SurfaceType_Default == Value->SurfaceType;
			}))
			{
				HitFX = *DefaultResult;
			}
			else
			{
				return;
			}

			if (bSpawnSound && HitFX->Sound.LoadSynchronous())
			{
				UAudioComponent* SpawnedSound = nullptr;

				const float MaskCurveValue = MeshComp->GetAnimInstance()->GetCurveValue(
					NAME_Mask_FootstepSound);
				const float FinalVolMult = bOverrideMaskCurve
					                           ? VolumeMultiplier
					                           : VolumeMultiplier * (1.0f - MaskCurveValue);

				switch (HitFX->SoundSpawnType)
				{
				case EHuruSpawnType::Location:
					SpawnedSound = UGameplayStatics::SpawnSoundAtLocation(
						World, HitFX->Sound.Get(), Hit.Location + HitFX->SoundLocationOffset,
						HitFX->SoundRotationOffset, FinalVolMult, PitchMultiplier);
					break;

				case EHuruSpawnType::Attached:
					SpawnedSound = UGameplayStatics::SpawnSoundAttached(HitFX->Sound.Get(), MeshComp, FootSocketName,
					                                                    HitFX->SoundLocationOffset,
					                                                    HitFX->SoundRotationOffset,
					                                                    HitFX->SoundAttachmentType, true, FinalVolMult,
					                                                    PitchMultiplier);

					break;
				}
				if (SpawnedSound)
				{
					SpawnedSound->SetIntParameter(SoundParameterName, static_cast<int32>(FootstepType));
				}
			}

			if (bSpawnNiagara && HitFX->NiagaraSystem.LoadSynchronous())
			{
				UNiagaraComponent* SpawnedParticle = nullptr;
				const FVector Location = Hit.Location + MeshOwner->GetTransform().TransformVector(
					HitFX->DecalLocationOffset);

				switch (HitFX->NiagaraSpawnType)
				{
				case EHuruSpawnType::Location:
					SpawnedParticle = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						World, HitFX->NiagaraSystem.Get(), Location, FootRotation + HitFX->NiagaraRotationOffset);
					break;

				case EHuruSpawnType::Attached:
					SpawnedParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(
						HitFX->NiagaraSystem.Get(), MeshComp, FootSocketName, HitFX->NiagaraLocationOffset,
						HitFX->NiagaraRotationOffset, HitFX->NiagaraAttachmentType, true);
					break;
				}
			}

			if (bSpawnDecal && HitFX->DecalMaterial.LoadSynchronous())
			{
				const FVector Location = Hit.Location + MeshOwner->GetTransform().TransformVector(
					HitFX->DecalLocationOffset);

				const FVector DecalSize = FVector(bMirrorDecalX ? -HitFX->DecalSize.X : HitFX->DecalSize.X,
				                                  bMirrorDecalY ? -HitFX->DecalSize.Y : HitFX->DecalSize.Y,
				                                  bMirrorDecalZ ? -HitFX->DecalSize.Z : HitFX->DecalSize.Z);

				UDecalComponent* SpawnedDecal = nullptr;
				switch (HitFX->DecalSpawnType)
				{
				case EHuruSpawnType::Location:
					SpawnedDecal = UGameplayStatics::SpawnDecalAtLocation(
						World, HitFX->DecalMaterial.Get(), DecalSize, Location,
						FootRotation + HitFX->DecalRotationOffset, HitFX->DecalLifeSpan);
					break;

				case EHuruSpawnType::Attached:
					SpawnedDecal = UGameplayStatics::SpawnDecalAttached(HitFX->DecalMaterial.Get(), DecalSize,
					                                                    Hit.Component.Get(), NAME_None, Location,
					                                                    FootRotation + HitFX->DecalRotationOffset,
					                                                    HitFX->DecalAttachmentType,
					                                                    HitFX->DecalLifeSpan);
					break;
				}
			}
		}
	}
}

FString UHuruAnimNotifyFootstep::GetNotifyName_Implementation() const
{
	FString Name(TEXT("Footstep Type: "));
	Name.Append(GetEnumerationToString(FootstepType));
	return Name;
}
