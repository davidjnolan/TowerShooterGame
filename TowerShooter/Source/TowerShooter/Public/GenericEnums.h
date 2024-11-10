// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "GenericEnums.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECombatState
{
	Exploring UMETA(DisplayName = "Exploring"),
	BetweenWaves UMETA(DisplayName = "Between Waves"),
	Combat UMETA(DisplayName = "Combat"),
};

UENUM(BlueprintType)
enum ELockCamera
{
	GamepadOnly UMETA(DisplayName = "Gamepad Only"),
	MouseOnly UMETA(DisplayName = "Mouse Only"),
	GamepadAndMouse UMETA(DisplayName = "Gamepad and Mouse"),
	None UMETA(DisplayName = "Off")
};

UENUM(BlueprintType)
enum EPlayerState
{
	NONE UMETA(DisplayName = "None"),
	Jumping UMETA(DisplayName = "Jumping"),
	DodgeRoll UMETA(DisplayName = "DodgeRoll"),
	Prone UMETA(DisplayName = "Prone"),
	Prone_Falling UMETA(DisplayName = "Prone Falling"),
	Prone_Reving UMETA(DisplayName = "Prone Reviving"),
};

UENUM(BlueprintType)
enum EStatusEffect
{
	NoEffect UMETA(DisplayName = "No Effect"),
	Burning UMETA(DisplayName = "Burning"),
	Slowed UMETA(DisplayName = "Slowed"),
	Disabled UMETA(DisplayName = "Disabled"),
};


class TOWERSHOOTER_API GenericEnums
{
public:
	GenericEnums();
	~GenericEnums();
};
