// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_Base.generated.h"


/**
 * 
 */
UCLASS()
class TOWERSHOOTER_API APlayerController_Base : public APlayerController
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;


};
