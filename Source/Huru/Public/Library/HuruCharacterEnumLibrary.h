#pragma once

#include "CoreMinimal.h"
#include "HuruCharacterEnumLibrary.generated.h"

/* 열거형 인덱스를 반환 */
template <typename Enumeration>
static FORCEINLINE int32 GetEnumerationIndex(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetIndexByValue(static_cast<int64>(InValue));
}

/* 열거형 값을 문자열로 반환 */
template <typename Enumeration>
static FORCEINLINE FString GetEnumerationToString(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetNameStringByValue(static_cast<int64>(InValue));
}

/** 캐릭터 보행 상태. note: 새로운 열거형을 추가하면 HuruStructEnumLibrary의 관련 구조체도 수정해야 함 */
UENUM(BlueprintType, meta = (ScriptName = "Huru_Gait"))
enum class EHuruGait : uint8
{
	Walking,
	Running,
	Sprinting
};

/** 캐릭터 이동 동작 상태. note: 새로운 열거형을 추가하면 HuruStructEnumLibrary의 관련 구조체도 수정해야 함*/
UENUM(BlueprintType, meta = (ScriptName = "Huru_MovementAction"))
enum class EHuruMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
};

/** 캐릭터 움직임 상태. note: 새로운 enum을 추가하면 HuruStructEnumLibrary에 있는 관련 구조체도 수정해야 함 */
UENUM(BlueprintType, meta = (ScriptName = "Huru_MovementState"))
enum class EHuruMovementState : uint8
{
	None,
	Grounded,
	InAir,
	Mantling,
	Ragdoll
};

/** 캐릭터 회전 모드. note: 새로운 열거형을 추가할 경우 HuruStructEnumLibrary의 관련 구조체도 수정해야 함s*/
UENUM(BlueprintType, meta = (ScriptName="HuruRotationMode"))
enum class EHuruRotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

/** 캐릭터 자세. note: 새로운 열거형을 추가하면 HuruStructEnumLibrary의 관련 구조체도 수정해야 함*/
UENUM(BlueprintType, meta = (ScriptName = "Huru_Stance"))
enum class EHuruStance : uint8
{
	Standing,
	Crouching
};

/** 캐릭터 뷰 모드. note: 새로운 열거형을 추가할 경우 HuruStructEnumLibrary의 관련 구조체도 수정해야 함*/
UENUM(BlueprintType, meta = (ScriptName = "Huru_ViewMode"))
enum class EHuruViewMode : uint8
{
	ThirdPerson,
	FirstPerson
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_AnimFeatureExample"))
enum class EHuruAnimFeatureExample : uint8
{
	StrideBlending,
	AdditiveBlending,
	SprintImpulse
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_FootstepType"))
enum class EHuruFootstepType : uint8
{
	Step,
	WalkRun,
	Jump,
	Land
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_GroundedEntryState"))
enum class EHuruGroundedEntryState : uint8
{
	None,
	Roll
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_HipsDirection"))
enum class EHuruHipsDirection : uint8
{
	F,
	B,
	RF,
	RB,
	LF,
	LB
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_MantleType"))
enum class EHuruMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_MovementDirection"))
enum class EHuruMovementDirection : uint8
{
	Forward,
	Right,
	Left,
	Backward
};

UENUM(BlueprintType, meta = (ScriptName = "Huru_SpawnType"))
enum class EHuruSpawnType : uint8
{
	Location,
	Attached
};