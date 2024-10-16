#pragma once

#include "CoreMinimal.h"
#include "HuruCharacterEnumLibrary.generated.h"


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

/** 캐릭터 회전 모드. note: 새로운 열거형을 추가할 경우 ALSStructEnumLibrary의 관련 구조체도 수정해야 함s*/
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