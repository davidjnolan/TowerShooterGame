// ©David John Nolan, 2024


#include "AimAssistComponent.h"

// Sets default values for this component's properties
UAimAssistComponent::UAimAssistComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UAimAssistComponent::BeginPlay()
{
	Super::BeginPlay();
    // Register events in BeginPlay
	
}