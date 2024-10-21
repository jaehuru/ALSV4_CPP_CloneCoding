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

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Ragdoll System
	UFUNCTION(BlueprintCallable, Category = "Huru|Ragdoll System")
	virtual void RagdollStart();

	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = "Huru|Ragdoll System")
	void Server_SetMeshLocationDuringRagdoll(FVector MeshLocation);
#pragma endregion

#pragma region Rotation System

	UFUNCTION(BlueprintCallable, Category = "Huru|Rotation System")
	void SetActorLocationAndTargetRotation(FVector NewLocation, FRotator NewRotation);
	
#pragma endregion 
	
#pragma region Character States
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetMovementState(EHuruMovementState NewState, bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetMovementAction(EHuruMovementAction NewAction, bool bForce = false);
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetStance(EHuruStance NewStance, bool bForce = false);
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Character States")
	void SetGait(EHuruGait NewGait, bool bForce = false);
	
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

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruMovementState GetPrevMovementState() const { return PrevMovementState; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruMovementAction GetMovementAction() const { return MovementAction; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruStance GetStance() const { return Stance; }
	
	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintGetter, Category = "Huru|CharacterStates")
	EHuruGait GetDesiredGait() const { return DesiredGait; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruRotationMode GetRotationMode() const { return RotationMode; }
	
	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruViewMode GetViewMode() const { return ViewMode; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Character States")
	EHuruGroundedEntryState GetGroundedEntryState() const { return GroundedEntryState; }

	UFUNCTION(BlueprintSetter, Category = "Huru|Chracter States")
	void SetDesiredRotationMode(EHuruRotationMode NewRotMode);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Huru|Chracter States")
	void Server_SetDesiredRotationMode(EHuruRotationMode NewRotMode);
	
#pragma endregion

#pragma region Movement System
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Movement System")
	FHuruMovementSettings GetTargetMovementSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Huru|Movement System")
	EHuruGait GetAllowedGait() const;

	UFUNCTION(BlueprintCallable, Category = "Huru|Movement System")
	EHuruGait GetActualGait(EHuruGait AllowedGait); 
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Movement System")
	bool CanSprint() const;
	
	/** Rolling과 같이 몽타주가 시작될 때 호출되는 BP 구현 가능 함수 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Huru|Movement System")
	void Replicated_PlayMontage(UAnimMontage* Montage, float PlayRate);
	virtual void Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate);

	/** 캐릭터의 상태에 따라 필요한 Roll 애니메이션을 얻기 위해 BP에서 구현 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Huru|Movement System")
	UAnimMontage* GetRollAnimation();

	UFUNCTION(BlueprintGetter, Category = "Huru|Movement System")
	bool HasMovementInput() const { return bHasMovementInput; }
	
#pragma endregion

#pragma region

	UFUNCTION(BlueprintCallable, Category = "Huru|Utility")
	float GetAnimCurveValue(FName CurveName) const;
	
#pragma endregion 

#pragma region Camera System
	
	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	void SetRightShoulder(bool bNewRightShoulder);

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius);

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	virtual FTransform GetThirdPersonPivotTarget();

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	virtual FVector GetFirstPersonCameraTarget();

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	void GetCameraParameters(float& TPFOVOut, float& FPFOVOut, bool& bRightShoulderOut) const;

	UFUNCTION(BlueprintGetter, Category = "Huru|Camera System")
	bool IsRightShoulder() const { return bRightShoulder; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Camera System")
	void SetCameraBehavior(UHuruPlayerCameraBehavior* CamBeh) { CameraBehavior = CamBeh; }
	
#pragma endregion

#pragma region Essential Information
	
	UFUNCTION(BlueprintGetter, Category = "Huru|Essential Information")
	FVector GetAcceleration() const { return Acceleration; }
	
	UFUNCTION(BlueprintGetter, Category = "Huru|Essential Information")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Essential Information")
	FVector GetMovementInput() const;

	UFUNCTION(BlueprintGetter, Category = "Huru|Essential Information")
	float GetMovementInputAmount() const { return MovementInputAmount; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Essential Information")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, Category = "Huru|Essential Information")
	FRotator GetAimingRotation() const { return AimingRotation; }

	UFUNCTION(BlueprintGetter, Category = "Huru|Essential Information")
	float GetAimYawRate() const { return AimYawRate; }

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Huru|Movement System")
	FDataTableRowHandle MovementModel;
#pragma endregion

#pragma region Camera System

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Camera System")
	float ThirdPersonFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Camera System")
	float FirstPersonFOV = 90.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Huru|Camera System")
	bool bRightShoulder = false;
	
#pragma endregion

#pragma region Essential Information

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	FRotator LastMovementInputRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	float AimYawRate = 0.0f;

#pragma endregion

#pragma region Replicated Essential Information

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Essential Information")
	float EasedMaxAcceleration = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Huru|Essential Information")
	FVector ReplicatedCurrentAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Huru|Essential Information")
	FRotator ReplicatedControlRotation = FRotator::ZeroRotator;

#pragma endregion

#pragma region State_Values
	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruGroundedEntryState GroundedEntryState;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruMovementState PrevMovementState = EHuruMovementState::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruMovementState MovementState = EHuruMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruMovementAction MovementAction = EHuruMovementAction::None;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values", ReplicatedUsing = OnRep_RotationMode)
	EHuruRotationMode RotationMode = EHuruRotationMode::LookingDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|State Values")
	EHuruGait Gait = EHuruGait::Walking;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Huru|State Values")
	EHuruStance Stance = EHuruStance::Standing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Huru|State Values", ReplicatedUsing = OnRep_ViewMode)
	EHuruViewMode ViewMode = EHuruViewMode::ThirdPerson;
#pragma endregion

#pragma region Rotation System
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Rotation System")
	FRotator TargetRotation = FRotator::ZeroRotator;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Rotation System")
	FRotator InAirRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Rotation System")
	float YawOffset = 0.0f;
	
#pragma endregion

#pragma region Breakfall System

	/** 플레이어가 지정된 속도로 지면에 충돌하면 브레이크폴 상태로 전환 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Huru|Breakfall System")
	bool bBreakfallOnLand = true;

	/** 플레이어가 지면에 지정된 값보다 더 큰 속도로 충돌하면 브레이크폴 상태로 전환 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Huru|Breakfall System", meta = (EditCondition ="bBreakfallOnLand"))
	float BreakfallOnLandVelocity = 700.0f;

#pragma endregion 

#pragma region Ragdoll System

	/** 스켈레톤이 반대로 설정된 골반 본을 사용하는 경우, 계산 연산자를 반대로 설정함 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Huru|Ragdoll System")
	bool bReversedPelvis = false;

	/** 플레이어가 특정 속도로 땅에 부딪히면 랙돌 상태로 전환함 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Huru|Ragdoll System")
	bool bRagdollOnLand = false;

	/** 플레이어가 지정된 값보다 큰 속도로 땅에 부딪히면 랙돌 상태로 전환함 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Huru|Ragdoll System", meta = (EditCondition ="bRagdollOnLand"))
	float RagdollOnLandVelocity = 1000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Ragdoll System")
	bool bRagdollOnGround = false;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Ragdoll System")
	bool bRagdollFaceUp = false;

	UPROPERTY(BlueprintReadOnly, Category = "Huru|Ragdoll System")
	FVector LastRagdollVelocity = FVector::ZeroVector;
	
	/** 서버에서 계산된 랙돌의 위치를 클라이언트와 동기화하는 데 사용될 수 있음 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Huru|Ragdoll System")
	FVector TargetRagdollLocation = FVector::ZeroVector;

	/* 서버에서 랙돌 상태 캐릭터에 적용할 끌어당기는 힘을 저장하는 변수 */
	float ServerRagdollPull = 0.0f;
	
	bool bPreRagdollURO = false;
	
	/* 전용 서버에서 메쉬의 기본 가시성 기반 애니메이션 틱 옵션 */
	EVisibilityBasedAnimTickOption DefVisBasedTickOp;
	
#pragma endregion

#pragma region Cached Variables

	/** 이전 프레임에서의 속도 값을 저장하여 현재 속도와 비교하거나 상태 변화를 체크할 때 사용*/
	FVector PreviousVelocity = FVector::ZeroVector;

	/** 이전 프레임에서의 조준 각도를 저장해두고, 이후의 조준 변화량을 계산할 때 활용*/
	float PreviousAimYaw = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Huru|Camera")
	TObjectPtr<UHuruPlayerCameraBehavior> CameraBehavior;
	
	/** 마지막으로 '첫 번째' 무릎 꿇기/구르기 버튼이 눌렸을 때 */
	float LastStanceInputTime = 0.0f;

	/* 착지 이벤트 이후 제동 마찰 계수를 초기화하기 위한 타이머 관리 */
	FTimerHandle OnLandedFrictionResetTimer;
	
	/** 컨트롤 회전을 보간(interpolating)하여 에임을 부드럽게 처리 */
	FRotator AimingRotation = FRotator::ZeroRotator;

	/** 네트워크 게임에서는 곡선 기반 이동같은 다른 기능을 사용하지 않을 것 */
	bool bEnableNetworkOptimizations = false;
	
#pragma endregion
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
#pragma region Ragdoll Sysyem

	void RagdollUpdate(float DeltaTime);

	void SetActorLocationDuringRagdoll(float DeltaTime);
	
#pragma endregion 
	
#pragma region State Changes
	
	virtual void OnMovementStateChanged(EHuruMovementState PreviousState);

	virtual void OnMovementActionChanged(EHuruMovementAction PreviousAction);

	virtual void OnStanceChanged(EHuruStance PreviousStance);

	virtual void OnRotationModeChanged(EHuruRotationMode PreviousRotationMode);

	virtual void OnGaitChanged(EHuruGait PreviousGait);

	virtual void OnViewModeChanged(EHuruViewMode PreviousViewMode);

	void SetEssentialValues(float DeltaTime);

	void UpdateCharacterMovement();

	void UpdateGroundedRotation(float DeltaTime);

	void UpdateInAirRotation(float DeltaTime);
	
#pragma endregion

#pragma region Utils
	
	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);

	float CalculateGroundedRotationRate() const;

	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
	
	void SetMovementModel();

	void ForceUpdateCharacterState();
	
#pragma endregion 

#pragma region Replication 
	UFUNCTION(Category = "Huru|Replication")
	void OnRep_RotationMode(EHuruRotationMode PrevRotMode);

	UFUNCTION(Category = "Huru|Replication")
	void OnRep_ViewMode(EHuruViewMode PrevViewMode);

#pragma endregion
};


