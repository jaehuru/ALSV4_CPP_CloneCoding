// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/HuruDebugComponent.h"

#include "Character/HuruBaseCharacter.h"
#include "Character/HuruPlayerCameraManager.h"
#include "Character/Animation/HuruPlayerCameraBehavior.h"
//Engine
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

bool UHuruDebugComponent::bDebugView = false;
bool UHuruDebugComponent::bShowTraces = false;
bool UHuruDebugComponent::bShowDebugShapes = false;
bool UHuruDebugComponent::bShowLayerColors = false;

UHuruDebugComponent::UHuruDebugComponent()
{
#if UE_BUILD_SHIPPING
	PrimaryComponentTick.bCanEverTick = false;
#else
	PrimaryComponentTick.bCanEverTick = true;
#endif
}

void UHuruDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AHuruBaseCharacter>(GetOwner());
	DebugFocusCharacter = OwnerCharacter;
	if (OwnerCharacter)
	{
		SetDynamicMaterials();
		SetResetColors();
	}
	
}

void UHuruDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !UE_BUILD_SHIPPING
	if (!OwnerCharacter)
	{
		return;
	}

	if (bNeedsColorReset)
	{
		bNeedsColorReset = false;
		SetResetColors();
	}

	if (bShowLayerColors)
	{
		UpdateColoringSystem();
	}
	else
	{
		bNeedsColorReset = true;
	}

	if (bShowDebugShapes)
	{
		DrawDebugSpheres();

		APlayerController* Controller = Cast<APlayerController>(OwnerCharacter->GetController());
		if (Controller)
		{
			AHuruPlayerCameraManager* CamManager = Cast<AHuruPlayerCameraManager>(Controller->PlayerCameraManager);
			if (CamManager)
			{
				CamManager->DrawDebugTargets(OwnerCharacter->GetThirdPersonPivotTarget().GetLocation());
			}
		}
	}
#endif
}

void UHuruDebugComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// 파괴 시 정적 값을 false로 유지
	bDebugView = false;
	bShowTraces = false;
	bShowDebugShapes = false;
	bShowLayerColors = false;
}

void UHuruDebugComponent::ToggleGlobalTimeDilationLocal(float TimeDilation)
{
	if (UKismetSystemLibrary::IsStandalone(this))
	{
		UGameplayStatics::SetGlobalTimeDilation(this, TimeDilation);
	}
}

void UHuruDebugComponent::ToggleSlomo()
{
	bSlomo = !bSlomo;
	ToggleGlobalTimeDilationLocal(bSlomo ? 0.15f : 1.f);
}

void UHuruDebugComponent::ToggleDebugView()
{
	bDebugView = !bDebugView;

	AHuruPlayerCameraManager* CamManager = Cast<AHuruPlayerCameraManager>(
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
	if (CamManager)
	{
		UHuruPlayerCameraBehavior* CameraBehavior = Cast<UHuruPlayerCameraBehavior>(
			CamManager->CameraBehavior->GetAnimInstance());
		if (CameraBehavior)
		{
			CameraBehavior->bDebugView = bDebugView;
		}
	}
}

void UHuruDebugComponent::OpenOverlayMenu_Implementation(bool bValue)
{
}

void UHuruDebugComponent::OverlayMenuCycle_Implementation(bool bValue)
{
}

void UHuruDebugComponent::ToggleDebugMesh()
{
}

void UHuruDebugComponent::FocusedDebugCharacterCycle(bool bValue)
{
	// 리스트를 새로 고쳐서 런타임에서 생성된 캐릭터를 디버깅하고 소멸된 캐릭터를 제거할 수 있게 해야함
	DetectDebuggableCharactersInWorld();

	if (FocusedDebugCharacterIndex == INDEX_NONE)
	{
		// BeginPlay 호출 중 AHuruBaseCharacter가 발견되지 않았으므로 여기서 반환
		// 안전을 위해 포커스된 디버그 캐릭터도 설정하지 않음
		DebugFocusCharacter = nullptr;
		return;
	}

	if (bValue)
	{
		FocusedDebugCharacterIndex++;
		if (FocusedDebugCharacterIndex >= AvailableDebugCharacters.Num())
		{
			FocusedDebugCharacterIndex = 0;
		}
	}
	else
	{
		FocusedDebugCharacterIndex--;
		if (FocusedDebugCharacterIndex < 0)
		{
			FocusedDebugCharacterIndex = AvailableDebugCharacters.Num() - 1;
		}
	}

	DebugFocusCharacter = AvailableDebugCharacters[FocusedDebugCharacterIndex];
}

void UHuruDebugComponent::DrawDebugLineTraceSingle(const UWorld* World, const FVector& Start, const FVector& End,
	EDrawDebugTrace::Type DrawDebugType, bool bHit, const FHitResult& OutHit, FLinearColor TraceColor, FLinearColor TraceHitColor,
	float DrawTime)
{
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugLine(World, Start, OutHit.ImpactPoint, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, OutHit.ImpactPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 16.0f, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
}

void UHuruDebugComponent::DrawDebugCapsuleTraceSingle(const UWorld* World, const FVector& Start, const FVector& End,
	const FCollisionShape& CollisionShape, EDrawDebugTrace::Type DrawDebugType, bool bHit, const FHitResult& OutHit,
	FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugCapsule(World, Start, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugCapsule(World, OutHit.Location, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, Start, OutHit.Location, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 16.0f, TraceColor.ToFColor(true), bPersistent, LifeTime);

			DrawDebugCapsule(World, End, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, OutHit.Location, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugCapsule(World, Start, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugCapsule(World, End, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
}

static void DrawDebugSweptSphere(const UWorld* InWorld,
							FVector const& Start,
							FVector const& End,
							float Radius,
							FColor const& Color,
							bool bPersistentLines = false,
							float LifeTime = -1.f,
							uint8 DepthPriority = 0)
{
	FVector const TraceVec = End - Start;
	float const Dist = TraceVec.Size();

	FVector const Center = Start + TraceVec * 0.5f;
	float const HalfHeight = (Dist * 0.5f) + Radius;

	FQuat const CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	DrawDebugCapsule(InWorld, Center, HalfHeight, Radius, CapsuleRot, Color, bPersistentLines, LifeTime, DepthPriority);
}

void UHuruDebugComponent::DrawDebugSphereTraceSingle(const UWorld* World, const FVector& Start, const FVector& End,
	const FCollisionShape& CollisionShape, EDrawDebugTrace::Type DrawDebugType, bool bHit, const FHitResult& OutHit,
	FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugSweptSphere(World, Start, OutHit.Location, CollisionShape.GetSphereRadius(), TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugSweptSphere(World, OutHit.Location, End, CollisionShape.GetSphereRadius(), TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 16.0f, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugSweptSphere(World, Start, End, CollisionShape.GetSphereRadius(), TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
}

void UHuruDebugComponent::DetectDebuggableCharactersInWorld()
{
	// 현재 존재하는 모든 HuruBaseCharacter를 가져와 나중에 ALS HUD에서 디버깅 목적으로 표시
	TArray<AActor*> AlsBaseCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHuruBaseCharacter::StaticClass(), AlsBaseCharacters);

	AvailableDebugCharacters.Empty();
	if (AlsBaseCharacters.Num() > 0)
	{
		AvailableDebugCharacters.Reserve(AlsBaseCharacters.Num());
		for (AActor* Character : AlsBaseCharacters)
		{
			if (AHuruBaseCharacter* AlsBaseCharacter = Cast<AHuruBaseCharacter>(Character))
			{
				AvailableDebugCharacters.Add(AlsBaseCharacter);
			}
		}

		FocusedDebugCharacterIndex = AvailableDebugCharacters.Find(DebugFocusCharacter);
		if (FocusedDebugCharacterIndex == INDEX_NONE && AvailableDebugCharacters.Num() > 0)
		{
			// 이 컴포넌트가 AHuruBaseCharacter에 첨부되지 않은 것처럼 보입니다. 따라서 인덱스는 배열의 첫 번째 요소로 설정
			FocusedDebugCharacterIndex = 0;
		}
	}
}

