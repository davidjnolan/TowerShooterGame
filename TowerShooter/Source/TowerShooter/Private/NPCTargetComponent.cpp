// ©David John Nolan, 2024


#include "NPCTargetComponent.h"

UNPCTargetComponent::UNPCTargetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UNPCTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void UNPCTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UNPCTargetComponent::GetIsOccupied() const
{
    return bIsOccupied;
}

void UNPCTargetComponent::SetIsOccupied(bool value)
{
    bIsOccupied = value;
}

