// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "AimAssistComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TOWERSHOOTER_API UAimAssistComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAimAssistComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};
