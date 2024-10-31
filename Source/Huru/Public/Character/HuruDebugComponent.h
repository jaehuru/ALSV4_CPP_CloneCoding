// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CollisionShape.h"
#include "Components/ActorComponent.h"
#include "HuruDebugComponent.generated.h"


class AHuruBaseCharacter;

UCLASS(Blueprintable, BlueprintType)
class HURU_API UHuruDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Debug")
	TObjectPtr<AHuruBaseCharacter> OwnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Huru|Debug")
	bool bSlomo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Huru|Debug")
	bool bShowHud = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Huru|Debug")
	bool bShowCharacterInfo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Huru|Debug")
	TObjectPtr<USkeletalMesh> DebugSkeletalMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Debug")
	TArray<TObjectPtr<AHuruBaseCharacter>> AvailableDebugCharacters;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Debug")
	TObjectPtr<AHuruBaseCharacter> DebugFocusCharacter = nullptr;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UHuruDebugComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Huru|Debug")
	void OnPlayerControllerInitialized(APlayerController* Controller);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/** 레이어링 색상을 업데이트하기 위해 블루프린트에서 구현됨 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Huru|Debug")
	void UpdateColoringSystem();

	/** 디버그 구체를 그리기 위해 블루프린트에서 구현됨 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Huru|Debug")
	void DrawDebugSpheres();

	/** 레이어링 색상을 설정/초기화하기 위해 블루프린트에서 구현됨 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Huru|Debug")
	void SetResetColors();

	/** 디버깅을 위해 동적 색상 재질을 설정하기 위해 블루프린트에서 구현됨 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Huru|Debug")
	void SetDynamicMaterials();

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleGlobalTimeDilationLocal(float TimeDilation);

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleSlomo();

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleHud() { bShowHud = !bShowHud; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleDebugView();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Debug")
	void OpenOverlayMenu(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Debug")
	void OverlayMenuCycle(bool bValue);

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleDebugMesh();

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleTraces() { bShowTraces = !bShowTraces; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleDebugShapes() { bShowDebugShapes = !bShowDebugShapes; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleLayerColors() { bShowLayerColors = !bShowLayerColors; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void ToggleCharacterInfo() { bShowCharacterInfo = !bShowCharacterInfo; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	bool GetDebugView() { return bDebugView; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	bool GetShowTraces() { return bShowTraces; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	bool GetShowDebugShapes() { return bShowDebugShapes; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	bool GetShowLayerColors() { return bShowLayerColors; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Debug")
	void FocusedDebugCharacterCycle(bool bValue);

	// 추적 디버그 형태를 그리기 위한 유틸리티 함수들로, Engine/Private/KismetTraceUtils.h에서 파생됨
	// 안타깝게도 이 함수들은 비공개이기 때문에 여기에서 재구현됨
	static void DrawDebugLineTraceSingle(const UWorld* World,
	                                     const FVector& Start,
	                                     const FVector& End,
	                                     EDrawDebugTrace::Type DrawDebugType,
	                                     bool bHit,
	                                     const FHitResult& OutHit,
	                                     FLinearColor TraceColor,
	                                     FLinearColor TraceHitColor,
	                                     float DrawTime);

	static void DrawDebugCapsuleTraceSingle(const UWorld* World,
	                                        const FVector& Start,
	                                        const FVector& End,
	                                        const FCollisionShape& CollisionShape,
	                                        EDrawDebugTrace::Type DrawDebugType,
	                                        bool bHit,
	                                        const FHitResult& OutHit,
	                                        FLinearColor TraceColor,
	                                        FLinearColor TraceHitColor,
	                                        float DrawTime);

	static void DrawDebugSphereTraceSingle(const UWorld* World,
	                                       const FVector& Start,
	                                       const FVector& End,
	                                       const FCollisionShape& CollisionShape,
	                                       EDrawDebugTrace::Type DrawDebugType,
	                                       bool bHit,
	                                       const FHitResult& OutHit,
	                                       FLinearColor TraceColor,
	                                       FLinearColor TraceHitColor,
	                                       float DrawTime);

protected:
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	void DetectDebuggableCharactersInWorld();

private:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	static bool bDebugView;

	static bool bShowTraces;

	static bool bShowDebugShapes;

	static bool bShowLayerColors;

	bool bNeedsColorReset = false;

	bool bDebugMeshVisible = false;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> DefaultSkeletalMesh = nullptr;

	/// 다음으로 집중할 디버그 HuruBaseCharacter를 선택하는 데 사용되는 인덱스를 저장함
	/// BeginPlay 동안 캐릭터가 발견되지 않으면 값은 INDEX_NONE으로 설정되어야 함
	int32 FocusedDebugCharacterIndex = INDEX_NONE;
		
};
