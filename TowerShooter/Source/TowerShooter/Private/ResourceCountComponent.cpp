// ©David Nolan, 2024


#include "ResourceCountComponent.h"
#include "GenericStructs.h"

// Sets default values for this component's properties
UResourceCountComponent::UResourceCountComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Orbs.ResourceName = "Orbs";
	Limestone.ResourceName = "Limestone";
	Copper.ResourceName = "Copper";
	Iron.ResourceName = "Iron";
}


// Called when the game starts
void UResourceCountComponent::BeginPlay()
{
	Super::BeginPlay();

	Orbs.Count = OrbsCount;
	Limestone.Count = LimestoneCount;
	Copper.Count = CopperCount;
	Iron.Count = IronCount;
	
}


// Called every frame
void UResourceCountComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UResourceCountComponent::IncrementResourceCountByName(FString Name, int32 count)
{
	if (Name == "Orbs")
	{
		Orbs.Count += count;
	}
	else if (Name == "Limestone")
	{
		Limestone.Count += count;
	}
	else if (Name == "Copper")
	{
		Copper.Count += count;
	}
	else if (Name == "Iron")
	{
		Iron.Count += count;
	}
	
}