// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/HuruBaseCharacter.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HuruMantleComponent.generated.h"

class UTimelineComponent;
class UHuruDebugComponent;

UCLASS(Blueprintable, BlueprintType)
class HURU_API UHuruMantleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UHuruMantleComponent();

	UFUNCTION(BlueprintCallable, Category = "Huru|Mantle System")
	bool MantleCheck(const FHuruMantleTraceSettings& TraceSettings, EDrawDebugTrace::Type DebugType);

	UFUNCTION(BlueprintCallable, Category = "Huru|Mantle System")
	void MantleStart(float MantleHeight, const FHuruComponentAndTransform& MantleLedgeWS,EHuruMantleType MantleType);

	UFUNCTION(BlueprintCallable, Category = "Huru|Mantle System")
	void MantleUpdate(float BlendIn);

	UFUNCTION(BlueprintCallable, Category = "Huru|Mantle System")
	void MantleEnd();

	UFUNCTION(BlueprintCallable, Category = "Huru|Mantle System")
	void OnOwnerJumpInput();

	UFUNCTION(BlueprintCallable, Category = "Huru|Mantle System")
	void OnOwnerRagdollStateChanged(bool bRagdollState);

	/** BP에서 캐릭터 상태에 따라 올바른 맨틀 파라미터 세트를 가져오도록 구현 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Huru|Mantle System")
	FHuruMantleAsset GetMantleAsset(EHuruMantleType MantleType, EHuruOverlayState CurrentOverlayState);

protected:
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Mantling*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Mantle System")
	void Server_MantleStart(float MantleHeight, const FHuruComponentAndTransform& MantleLedgeWS,EHuruMantleType MantleType);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Huru|Mantle System")
	void Multicast_MantleStart(float MantleHeight, const FHuruComponentAndTransform& MantleLedgeWS,EHuruMantleType MantleType);

	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Huru|Mantle System")
	TObjectPtr<UTimelineComponent> MantleTimeline = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	FHuruMantleTraceSettings GroundedTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	FHuruMantleTraceSettings AutomaticTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	FHuruMantleTraceSettings FallingTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	TObjectPtr<UCurveFloat> MantleTimelineCurve;

	static FName NAME_IgnoreOnlyPawn;
	/** 맨틀링을 허용하는 객체를 감지하는 데 사용할 프로파일 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	FName MantleObjectDetectionProfile = NAME_IgnoreOnlyPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	TEnumAsByte<ECollisionChannel> WalkableSurfaceDetectionChannel = ECC_Visibility;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Mantle System")
	FHuruMantleParams MantleParams;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Mantle System")
	FHuruComponentAndTransform MantleLedgeLS;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Mantle System")
	FTransform MantleTarget = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Mantle System")
	FTransform MantleActualStartOffset = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Mantle System")
	FTransform MantleAnimatedStartOffset = FTransform::Identity;
	
	/** 동적 객체의 속도가 이 값을 초과하면 맨틀을 시작하지 않음 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Mantle System")
	float AcceptableVelocityWhileMantling = 10.0f;

private:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	UPROPERTY()
	TObjectPtr<AHuruBaseCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UHuruDebugComponent> HuruDebugComponent = nullptr;


		
};
