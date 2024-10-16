// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Library/HuruCharacterEnumLibrary.h"
#include "Library/HuruCharacterStructLibrary.h"

#include "HuruBaseCharacter.generated.h"


class UHuruPlayerCameraBehavior;
enum class EVisibilityBasedAnimTickOption : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpPressedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumpedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRagdollStateChangedSignature, bool, bRagdollState);

/*
 * Base character class
 */
UCLASS()
class HURU_API AHuruBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
#pragma region Input
	UPROPERTY(BlueprintAssignable, Category = "Huru|Input")
	FJumpPressedSignature JumpPressedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Huru|Input")
	FOnJumpedSignature OnJumpedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Huru|Input")
	FRagdollStateChangedSignature RagdollStateChangedDelegate;
#pragma endregion
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	AHuruBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Ragdoll System
	UFUNCTION(BlueprintCallable, Category = "Huru|Ragdoll System")
	virtual void RagdollStart();
#pragma endregion 
	
#pragma region Character States
	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetMovementState(EHuruMovementState NewState, bool bForce = false);
	
	UFUNCTION(BlueprintSetter, Category = "Huru|Character States")
	void SetDesiredStance(EHuruStance NewStance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Character States")
	void Server_SetDesiredStance(EHuruStance NewStance);

	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetDesiredGait(EHuruGait NewGait);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Character States")
	void Server_SetDesiredGait(EHuruGait NewGait);
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetRotationMode(EHuruRotationMode NewRotationMode, bool bForce = false);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Character States")
	void Server_SetRotationMode(EHuruRotationMode NewRotationMode, bool bForce);

	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetViewMode(EHuruViewMode NewViewMode, bool bForce = false);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Character States")
	void Server_SetViewMode(EHuruViewMode NewViewMode, bool bForce);

	/** Rolling 몽타주 재생 복제 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Character States")
	void Server_PlayMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Huru|Character States")
	void Multicast_PlayMontage(UAnimMontage* Montage, float PlayRate);

	/** Ragdoll*/
	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void ReplicatedRagdollStart();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Character States")
	void Server_RagdollStart();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Huru|Character States")
	void Multicast_RagdollStart();
#pragma endregion

#pragma region Movement System
	UFUNCTION(BlueprintCallable, Category = "Huru|Movement System")
	FHuruMovementSettings GetTargetMovementSettings() const;

	/** Rolling과 같이 몽타주가 시작될 때 호출되는 BP 구현 가능 함수 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Movement System")
	void Replicated_PlayMontage(UAnimMontage* Montage, float PlayRate);
	virtual void Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate);

	/** 캐릭터의 상태에 따라 필요한 Roll 애니메이션을 얻기 위해 BP에서 구현 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Huru|Movement System")
	UAnimMontage* GetRollAnimation();
#pragma endregion

#pragma region Camera System
	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	void SetRightShoulder(bool bNewRightShoulder);
#pragma endregion

#pragma region Input
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void ForwardMovementAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void RightMovementAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void CameraUpAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void CameraRightAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void JumpAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void SprintAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void AimAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void CameraTapAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void CameraHeldAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void StanceAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void WalkAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void RagdollAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void VelocityDirectionAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Input")
	void LookingDirectionAction();
#pragma endregion
	//=====================================================================================
	//                            FORCEINLINE FUNCTIONS
	//=====================================================================================
	UFUNCTION(BlueprintCallable, Category = "Huru|Movement")
	FORCEINLINE class UHuruCharacterMovementComponent* GetMyMovementComponent() const { return MyCharacterMovementComponent; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruMovementState GetMovementState() const { return MovementState; }

protected:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	/* 캐릭터의 움직임을 커스텀마이징 */
	UPROPERTY()
	TObjectPtr<UHuruCharacterMovementComponent> MyCharacterMovementComponent;

#pragma region Input
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Huru|Input")
	EHuruRotationMode DesiredRotationMode = EHuruRotationMode::LookingDirection;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Huru|Input")
	EHuruGait DesiredGait = EHuruGait::Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Huru|Input")
	EHuruStance DesiredStance = EHuruStance::Standing;
	
	UPROPERTY(EditDefaultsOnly, Category = "Huru|Input", BlueprintReadOnly)
	float LookUpDownRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Huru|Input", BlueprintReadOnly)
	float LookLeftRightRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Huru|Input", BlueprintReadOnly)
	float RollDoubleTapTimeout = 0.3f;
#pragma endregion
	
#pragma region Movement System
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Movement System")
	FHuruMovementStateSettings MovementData;
#pragma endregion

#pragma region Camera System
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Camera System")
	bool bRightShoulder = false;
#pragma endregion
	
#pragma region State_Values
	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruMovementState PrevMovementState = EHuruMovementState::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruMovementState MovementState = EHuruMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruMovementAction MovementAction = EHuruMovementAction::None;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values", ReplicatedUsing = OnRep_RotationMode)
	EHuruRotationMode RotationMode = EHuruRotationMode::LookingDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Huru|State Values")
	EHuruStance Stance = EHuruStance::Standing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Huru|State Values", ReplicatedUsing = OnRep_ViewMode)
	EHuruViewMode ViewMode = EHuruViewMode::ThirdPerson;
#pragma endregion

#pragma region Rotation System 
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Rotation System")
	FRotator InAirRotation = FRotator::ZeroRotator;
#pragma endregion 

#pragma region Replicated Essential Information
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Huru|Essential Information")
	FVector ReplicatedCurrentAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Huru|Essential Information")
	FRotator ReplicatedControlRotation = FRotator::ZeroRotator;
#pragma endregion
	
#pragma region Ragdoll System
	/** 서버에서 계산된 랙돌의 위치를 클라이언트와 동기화하는 데 사용될 수 있음 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Huru|Ragdoll System")
	FVector TargetRagdollLocation = FVector::ZeroVector;

	/* 서버에서 랙돌 상태 캐릭터에 적용할 끌어당기는 힘을 저장하는 변수 */
	float ServerRagdollPull = 0.0f;

	bool bPreRagdollURO = false;
#pragma endregion

	/* 전용 서버에서 메쉬의 기본 가시성 기반 애니메이션 틱 옵션 */
	EVisibilityBasedAnimTickOption DefVisBasedTickOp;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Camera")
	TObjectPtr<UHuruPlayerCameraBehavior> CameraBehavior;
	
	/** 마지막으로 '첫 번째' 무릎 꿇기/구르기 버튼이 눌렸을 때 */
	float LastStanceInputTime = 0.0f;
	
	/** 컨트롤 회전을 보간(interpolating)하여 에임을 부드럽게 처리 */
	FRotator AimingRotation = FRotator::ZeroRotator;
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
#pragma region State Changes
	virtual void OnMovementStateChanged(EHuruMovementState PreviousState);
	
	virtual void OnRotationModeChanged(EHuruRotationMode PreviousRotationMode);

	virtual void OnViewModeChanged(EHuruViewMode PreviousViewMode);
#pragma endregion

#pragma region Replication 
	UFUNCTION(Category = "Huru|Replication")
	void OnRep_RotationMode(EHuruRotationMode PrevRotMode);

	UFUNCTION(Category = "Huru|Replication")
	void OnRep_ViewMode(EHuruViewMode PrevViewMode);
#pragma endregion
};

