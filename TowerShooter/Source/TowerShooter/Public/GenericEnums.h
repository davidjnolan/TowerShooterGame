// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "GenericEnums.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ELockCamera
{
	GamepadOnly UMETA(DisplayName = "Gamepad Only"),
	MouseOnly UMETA(DisplayName = "Mouse Only"),
	GamepadAndMouse UMETA(DisplayName = "Gamepad and Mouse"),
	None UMETA(DisplayName = "Off")
};

UENUM(BlueprintType)
enum ECombatState
{
	Exploring UMETA(DisplayName = "Exploring"),
	BetweenWaves UMETA(DisplayName = "Between Waves"),
	Combat UMETA(DisplayName = "Combat"),
};


class TOWERSHOOTER_API GenericEnums
{
public:
	GenericEnums();
	~GenericEnums();
};
