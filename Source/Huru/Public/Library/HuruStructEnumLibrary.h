// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "HuruCharacterEnumLibrary.h"

#include "HuruStructEnumLibrary.generated.h"


USTRUCT(BlueprintType)
struct FHuruMovementState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	EHuruMovementState State = EHuruMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Grounded_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool InAir_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Mantling_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Ragdoll_ = false;

public:
	FHuruMovementState()
	{
	}

	FHuruMovementState(const EHuruMovementState InitialState) { *this = InitialState; }

	const bool& None() const { return None_; }
	const bool& Grounded() const { return Grounded_; }
	const bool& InAir() const { return InAir_; }
	const bool& Mantling() const { return Mantling_; }
	const bool& Ragdoll() const { return Ragdoll_; }

	operator EHuruMovementState() const { return State; }

	void operator=(const EHuruMovementState NewState)
	{
		State = NewState;
		None_ = State == EHuruMovementState::None;
		Grounded_ = State == EHuruMovementState::Grounded;
		InAir_ = State == EHuruMovementState::InAir;
		Mantling_ = State == EHuruMovementState::Mantling;
		Ragdoll_ = State == EHuruMovementState::Ragdoll;
	}
};

USTRUCT(BlueprintType)
struct FHuruStance
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Character States")
	EHuruStance Stance = EHuruStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Character States")
	bool Standing_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Character States")
	bool Crouching_ = false;

public:
	FHuruStance()
	{
	}

	FHuruStance(const EHuruStance InitialStance) { *this = InitialStance; }

	const bool& Standing() const { return Standing_; }
	const bool& Crouching() const { return Crouching_; }

	operator EHuruStance() const { return Stance; }

	void operator=(const EHuruStance NewStance)
	{
		Stance = NewStance;
		Standing_ = Stance == EHuruStance::Standing;
		Crouching_ = Stance == EHuruStance::Crouching;
	}
};

USTRUCT(BlueprintType)
struct FHuruRotationMode
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Rotation System")
	EHuruRotationMode RotationMode = EHuruRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Rotation System")
	bool VelocityDirection_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Rotation System")
	bool LookingDirection_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Rotation System")
	bool Aiming_ = false;

public:
	FHuruRotationMode()
	{
	}

	FHuruRotationMode(const EHuruRotationMode InitialRotationMode) { *this = InitialRotationMode; }

	const bool& VelocityDirection() const { return VelocityDirection_; }
	const bool& LookingDirection() const { return LookingDirection_; }
	const bool& Aiming() const { return Aiming_; }

	operator EHuruRotationMode() const { return RotationMode; }

	void operator=(const EHuruRotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		VelocityDirection_ = RotationMode == EHuruRotationMode::VelocityDirection;
		LookingDirection_ = RotationMode == EHuruRotationMode::LookingDirection;
		Aiming_ = RotationMode == EHuruRotationMode::Aiming;
	}
};

USTRUCT(BlueprintType)
struct FHuruMovementDirection
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	EHuruMovementDirection MovementDirection = EHuruMovementDirection::Forward;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Forward_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Right_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Left_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Backward_ = false;

public:
	FHuruMovementDirection()
	{
	}

	FHuruMovementDirection(const EHuruMovementDirection InitialMovementDirection)
	{
		*this = InitialMovementDirection;
	}

	const bool& Forward() const { return Forward_; }
	const bool& Right() const { return Right_; }
	const bool& Left() const { return Left_; }
	const bool& Backward() const { return Backward_; }

	operator EHuruMovementDirection() const { return MovementDirection; }

	void operator=(const EHuruMovementDirection NewMovementDirection)
	{
		MovementDirection = NewMovementDirection;
		Forward_ = MovementDirection == EHuruMovementDirection::Forward;
		Right_ = MovementDirection == EHuruMovementDirection::Right;
		Left_ = MovementDirection == EHuruMovementDirection::Left;
		Backward_ = MovementDirection == EHuruMovementDirection::Backward;
	}
};

USTRUCT(BlueprintType)
struct FHuruMovementAction
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	EHuruMovementAction Action = EHuruMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool LowMantle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool HighMantle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Rolling_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool GettingUp_ = false;

public:
	FHuruMovementAction()
	{
	}

	FHuruMovementAction(const EHuruMovementAction InitialAction) { *this = InitialAction; }

	const bool& None() const { return None_; }
	const bool& LowMantle() const { return LowMantle_; }
	const bool& HighMantle() const { return HighMantle_; }
	const bool& Rolling() const { return Rolling_; }
	const bool& GettingUp() const { return GettingUp_; }

	operator EHuruMovementAction() const { return Action; }

	void operator=(const EHuruMovementAction NewAction)
	{
		Action = NewAction;
		None_ = Action == EHuruMovementAction::None;
		LowMantle_ = Action == EHuruMovementAction::LowMantle;
		HighMantle_ = Action == EHuruMovementAction::HighMantle;
		Rolling_ = Action == EHuruMovementAction::Rolling;
		GettingUp_ = Action == EHuruMovementAction::GettingUp;
	}
};

USTRUCT(BlueprintType)
struct FHuruGait
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	EHuruGait Gait = EHuruGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Walking_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Running_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Movement System")
	bool Sprinting_ = false;

public:
	FHuruGait()
	{
	}

	FHuruGait(const EHuruGait InitialGait) { *this = InitialGait; }

	const bool& Walking() const { return Walking_; }
	const bool& Running() const { return Running_; }
	const bool& Sprinting() const { return Sprinting_; }

	operator EHuruGait() const { return Gait; }

	void operator=(const EHuruGait NewGait)
	{
		Gait = NewGait;
		Walking_ = Gait == EHuruGait::Walking;
		Running_ = Gait == EHuruGait::Running;
		Sprinting_ = Gait == EHuruGait::Sprinting;
	}
};

USTRUCT(BlueprintType)
struct FHuruGroundedEntryState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Breakfall System")
	EHuruGroundedEntryState State = EHuruGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Breakfall System")
	bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Huru|Breakfall System")
	bool Roll_ = false;

public:
	FHuruGroundedEntryState()
	{
	}

	FHuruGroundedEntryState(const EHuruGroundedEntryState InitialState) { *this = InitialState; }

	const bool& None() const { return None_; }
	const bool& Roll() const { return Roll_; }

	operator EHuruGroundedEntryState() const { return State; }

	void operator=(const EHuruGroundedEntryState NewState)
	{
		State = NewState;
		None_ = State == EHuruGroundedEntryState::None;
		Roll_ = State == EHuruGroundedEntryState::Roll;
	}
};