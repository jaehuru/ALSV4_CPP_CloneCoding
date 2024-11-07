// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Library/HuruAnimationStructLibrary.h"
#include "Library/HuruStructEnumLibrary.h"

#include "HuruCharacterAnimInstance.generated.h"

class UHuruDebugComponent;
class UHuruCharacterMovementComponent;
class AHuruBaseCharacter;
class UCurveFloat;
class UAnimSequence;
class UCurveVector;
/**
 * Main anim instance class for character
 */
UCLASS(Blueprintable, BlueprintType)
class HURU_API UHuruCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
#pragma region References
	
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	TObjectPtr<AHuruBaseCharacter> Character = nullptr;

#pragma endregion 

#pragma region Character Information
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimCharacterInformation CharacterInformation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FHuruMovementState MovementState = EHuruMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FHuruMovementAction MovementAction = EHuruMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FHuruRotationMode RotationMode = EHuruRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FHuruGait Gait = EHuruGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FHuruStance Stance = EHuruStance::Standing;

#pragma endregion

#pragma region Anim Graph - Grounded
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimGraphGrounded Grounded;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FHuruVelocityBlend VelocityBlend;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FHuruLeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FHuruGroundedEntryState GroundedEntryState = EHuruGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FHuruMovementDirection MovementDirection = EHuruMovementDirection::Forward;

#pragma endregion

#pragma region Anim Graph - In Air
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - In Air", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimGraphInAir InAir;

#pragma endregion 

#pragma region Anim Graph - Aiming Values 
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimGraphAimingValues AimingValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

#pragma endregion

#pragma region Anim Graph - Ragdoll
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Ragdoll")
	float FlailRate = 0.0f;

#pragma endregion

#pragma region Anim Graph - Layer Blending
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimGraphLayerBlending LayerBlendingValues;

#pragma endregion

#pragma region Anim Graph - Foot IK

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimGraphFootIK FootIKValues;
	
#pragma endregion 

#pragma region Turn In Place
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Turn In Place", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimTurnInPlace TurnInPlaceValues;

#pragma endregion 

#pragma region Rotate In Place
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Place", Meta = (
		    ShowOnlyInnerProperties))
	FHuruAnimRotateInPlace RotateInPlace;

#pragma endregion 

#pragma region Configuration

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Main Configuration", Meta = (
		ShowOnlyInnerProperties))
	FHuruAnimConfiguration Config;

#pragma endregion

#pragma region Blend Curves

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LandPredictionCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LeanInAirCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_FB = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_LR = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L = nullptr;

#pragma endregion

#pragma region IK Bone Names

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootL_BoneName = FName(TEXT("ik_foot_l"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootR_BoneName = FName(TEXT("ik_foot_r"));
	
#pragma endregion 
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Huru|Animation")
	void PlayTransition(const FHuruDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Huru|Animation")
	void PlayTransitionChecked(const FHuruDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Huru|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FHuruDynamicMontageParams Parameters);

	UFUNCTION(BlueprintCallable, Category = "Huru|Event")
	void OnJumped();

	UFUNCTION(BlueprintCallable, Category = "Huru|Event")
	void OnPivot();

protected:
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	UFUNCTION(BlueprintCallable, Category = "Huru|Grounded")
	void SetTrackedHipsDirection(EHuruHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	UFUNCTION(BlueprintCallable, Category = "Huru|Grounded")
	void SetGroundedEntryState(EHuruGroundedEntryState NewState)
	{
		GroundedEntryState = NewState;
	}

	/** 움직임 애니메이션을 활성화하려면, 캐릭터가 움직이고 있고 움직임 입력이 있거나, 속도가 150 이상이어야 함 */
	UFUNCTION(BlueprintCallable, Category = "Huru|Grounded")
	bool ShouldMoveCheck() const;

	/** 캐릭터가 조준 중이거나 1인칭 시점일 때만 제자리 회전 체크를 수행함 */
	UFUNCTION(BlueprintCallable, Category = "Huru|Grounded")
	bool CanRotateInPlace() const;

	/**
	 * 캐릭터가 3인칭 시점에서 카메라를 향해 보고 있을 때만 제자리 회전 체크를 수행함,
     * 그리고 "Enable Transition" 커브가 완전히 가중치가 적용된 경우에만 체크함. Enable_Transition 커브는
     * AnimBP(애니메이션 블루프린트)의 특정 상태에서 수정되며, 캐릭터가 해당 상태에 있을 때만 회전할 수 있도록 설정됨. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Huru|Grounded")
	bool CanTurnInPlace() const;

	/**
	 * "Enable Transition" 커브가 완전히 가중치가 적용된 경우에만 동적 전환 체크를 수행함.
     * Enable_Transition 커브는 AnimBP의 특정 상태에서 수정되며, 캐릭터가 해당 상태에 있을 때만 전환이 가능하도록 설정됨.
	 */
	UFUNCTION(BlueprintCallable, Category = "Huru|Grounded")
	bool CanDynamicTransition() const;

private:
	//=====================================================================================
	//                            PROPERTIES & VARIABLES
	//=====================================================================================
	FTimerHandle OnPivotTimer;

	FTimerHandle PlayDynamicTransitionTimer;

	FTimerHandle OnJumpedTimer;

	bool bCanPlayDynamicTransition = true;

	UPROPERTY()
	TObjectPtr<UHuruDebugComponent> HuruDebugComponent = nullptr;
	
	//=====================================================================================
	//                                   FUNCTIONS
	//=====================================================================================
	void PlayDynamicTransitionDelay();

	void OnJumpedDelay();

	void OnPivotDelay();

#pragma region Update Values
	
	void UpdateAimingValues(float DeltaSeconds);

	void UpdateLayerValues();

	void UpdateFootIK(float DeltaSeconds);
	
	void UpdateMovementValues(float DeltaSeconds);

	void UpdateRotationValues();

	void UpdateInAirValues(float DeltaSeconds);

	void UpdateRagdollValues();

#pragma endregion

#pragma region Foot IK

	void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
						  float& CurFootLockAlpha, bool& UseFootLockCurve,
						  FVector& CurFootLockLoc, FRotator& CurFootLockRot);

	void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot);

	void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);

	void ResetIKOffsets(float DeltaSeconds);

	void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
						  FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset);

#pragma endregion 

#pragma region Grounded
	
	void RotateInPlaceCheck();

	void TurnInPlaceCheck(float DeltaSeconds);

	void DynamicTransitionCheck();

	FHuruVelocityBlend CalculateVelocityBlend() const;

	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

#pragma endregion 
	
#pragma region Movement
	
	FVector CalculateRelativeAccelerationAmount() const;

	float CalculateStrideBlend() const;

	float CalculateWalkRunBlend() const;

	float CalculateStandingPlayRate() const;

	float CalculateDiagonalScaleAmount() const;

	float CalculateCrouchingPlayRate() const;

	float CalculateLandPrediction() const;

	FHuruLeanAmount CalculateAirLeanAmount() const;

	EHuruMovementDirection CalculateMovementDirection() const;

#pragma endregion 
	
#pragma region Util
	
	float GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const;

#pragma endregion 
};
