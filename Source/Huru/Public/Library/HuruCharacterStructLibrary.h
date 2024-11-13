#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Library/HuruCharacterEnumLibrary.h"

#include "HuruCharacterStructLibrary.generated.h"

class UCurveVector;
class UAnimMontage;
class UAnimSequenceBase;
class UCurveFloat;
class UNiagaraSystem;
class UMaterialInterface;
class USoundBase;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FHuruComponentAndTransform
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Character Struct Library")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Character Struct Library")
	TObjectPtr<UPrimitiveComponent> Component = nullptr;
};

USTRUCT(BlueprintType)
struct FHuruCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Camera")
	float TargetArmLength = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LagSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float RotationLagSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	bool bDoCollisionTest = true;
};

USTRUCT(BlueprintType)
struct FALSCameraGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Camera")
	FHuruCameraSettings Walking;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FHuruCameraSettings Running;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FHuruCameraSettings Sprinting;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FHuruCameraSettings Crouching;
};

USTRUCT(BlueprintType)
struct FHuruMantleAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	TObjectPtr<UAnimMontage> AnimMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	TObjectPtr<UCurveVector> PositionCorrectionCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	FVector StartingOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float LowHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float LowPlayRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float LowStartPosition = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float HighHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float HighPlayRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float HighStartPosition = 0.0f;
};

USTRUCT(BlueprintType)
struct FHuruMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;

	float GetSpeedForGait(const EHuruGait Gait) const
	{
		switch (Gait)
		{
		case EHuruGait::Running:
			return RunSpeed;
		case EHuruGait::Sprinting:
			return SprintSpeed;
		case EHuruGait::Walking:
			return WalkSpeed;
		default:
			return RunSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FHuruMantleParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	TObjectPtr<UAnimMontage> AnimMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	TObjectPtr<UCurveVector> PositionCorrectionCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float StartingPosition = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float PlayRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	FVector StartingOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FHuruMantleTraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float MaxLedgeHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float MinLedgeHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float ReachDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float ForwardTraceRadius = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
	float DownwardTraceRadius = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;

	float GetSpeedForGait(const EHuruGait Gait) const
	{
		switch (Gait)
		{
		case EHuruGait::Running:
			return RunSpeed;
		case EHuruGait::Sprinting:
			return SprintSpeed;
		case EHuruGait::Walking:
			return WalkSpeed;
		default:
			return RunSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FHuruMovementStanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FHuruMovementStateSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementStanceSettings VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementStanceSettings LookingDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FHuruMovementStanceSettings Aiming;
};

USTRUCT(BlueprintType)
struct FHuruRotateInPlaceAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Rotation System")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, Category = "Rotation System")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Rotation System")
	float SlowTurnRate = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation System")
	float FastTurnRate = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation System")
	float SlowPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation System")
	float FastPlayRate = 1.0f;
};

USTRUCT(BlueprintType)
struct FHuruHitFX : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Surface")
	TEnumAsByte<enum EPhysicalSurface> SurfaceType = EPhysicalSurface::SurfaceType_Default;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TSoftObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Sound")
	EHuruSpawnType SoundSpawnType = EHuruSpawnType::Location;

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (EditCondition = "SoundSpawnType == EHuruSpawnType::Attached"))
	TEnumAsByte<enum EAttachLocation::Type> SoundAttachmentType = EAttachLocation::KeepRelativeOffset;

	UPROPERTY(EditAnywhere, Category = "Sound")
	FVector SoundLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Sound")
	FRotator SoundRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Decal")
	TSoftObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Decal")
	EHuruSpawnType DecalSpawnType = EHuruSpawnType::Location;

	UPROPERTY(EditAnywhere, Category = "Decal", meta = (EditCondition = "DecalSpawnType == EHuruSpawnType::Attached"))
	TEnumAsByte<enum EAttachLocation::Type> DecalAttachmentType = EAttachLocation::KeepRelativeOffset;

	UPROPERTY(EditAnywhere, Category = "Decal")
	float DecalLifeSpan = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Decal")
	FVector DecalSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Decal")
	FVector DecalLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Decal")
	FRotator DecalRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	EHuruSpawnType NiagaraSpawnType = EHuruSpawnType::Location;

	UPROPERTY(EditAnywhere, Category = "Niagara", meta = (EditCondition = "NiagaraSpawnType == EHuruSpawnType::Attached"))
	TEnumAsByte<enum EAttachLocation::Type> NiagaraAttachmentType = EAttachLocation::KeepRelativeOffset;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FVector NiagaraLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FRotator NiagaraRotationOffset = FRotator::ZeroRotator;
};
