// ©David John Nolan, 2024


#include "ActionButtonTriggerComponent.h"

// Sets default values for this component's properties
UActionButtonTriggerComponent::UActionButtonTriggerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UActionButtonTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
	// Register events in BeginPlay

}

