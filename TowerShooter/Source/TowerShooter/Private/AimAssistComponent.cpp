// ©David John Nolan, 2024


#include "AimAssistComponent.h"

UAimAssistComponent::UAimAssistComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


void UAimAssistComponent::BeginPlay()
{
	Super::BeginPlay();
    // Register events in BeginPlay
	
}