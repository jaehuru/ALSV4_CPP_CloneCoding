// Fill out your copyright notice in the Description page of Project Settings.

//Huru
#include "Character/HuruCharacterMovementComponent.h"
#include "Character/HuruBaseCharacter.h"
//Endgine
#include "Curves/CurveVector.h"

UHuruCharacterMovementComponent::UHuruCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRequestMovementSettingsChange = 1; // 초기값 설정
	AllowedGait = EHuruGait::Walking; // 초기값 설정
	CurrentMovementSettings = FHuruMovementSettings(); // 기본 생성자 호출하여 초기화
}

void UHuruCharacterMovementComponent::FSavedMove_My::Clear()
{
	FSavedMove_Character::Clear();

	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EHuruGait::Walking;
}

uint8 UHuruCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UHuruCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UHuruCharacterMovementComponent* CharacterMovement = Cast<UHuruCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;
	}
}

void UHuruCharacterMovementComponent::FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	FSavedMove_Character::PrepMoveFor(Character);

	UHuruCharacterMovementComponent* CharacterMovement = Cast<UHuruCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;
	}
}

UHuruCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(
	const UCharacterMovementComponent& ClientMovement)
		: Super(ClientMovement)
{
}

FSavedMovePtr UHuruCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return MakeShared<FSavedMove_My>();
}

void UHuruCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // 오직 클라이언트
{
	Super::UpdateFromCompressedFlags(Flags);

	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UHuruCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UHuruCharacterMovementComponent* MutableThis = const_cast<UHuruCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UHuruCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	if (bRequestMovementSettingsChange)
	{
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;

		bRequestMovementSettingsChange = false;
	}
}

void UHuruCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UHuruCharacterMovementComponent::GetMaxAcceleration() const
{
	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

float UHuruCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

float UHuruCharacterMovementComponent::GetMappedSpeed() const
{
	// 캐릭터의 현재 속도를 설정된 이동 속도에 따라 0에서 3까지의 범위로 매핑합니다.
	// 여기서 0은 정지 상태, 1은 걷기 속도, 2는 달리기 속도, 3은 전력 질주 속도를 의미합니다.
	// 이를 통해 이동 속도를 다양하게 설정하더라도 일관된 결과를 위한 계산에 매핑된 범위를 사용할 수 있습니다.

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UHuruCharacterMovementComponent::SetMovementSettings(FHuruMovementSettings NewMovementSettings)
{
	// 소유자(플레이어 또는 캐릭터)로부터 현재 이동 설정을 설정하다
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UHuruCharacterMovementComponent::SetAllowedGait(EHuruGait NewAllowedGait)
{
	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}

void UHuruCharacterMovementComponent::Server_SetAllowedGait_Implementation(EHuruGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}
