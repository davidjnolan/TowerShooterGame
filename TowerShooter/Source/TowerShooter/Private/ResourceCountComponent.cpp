// ©David Nolan, 2024


#include "ResourceCountComponent.h"
#include "GenericStructs.h"

// Sets default values for this component's properties
UResourceCountComponent::UResourceCountComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UResourceCountComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UResourceCountComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UResourceCountComponent::IncrementResource(FString ResourceName)
{
	if (ResourceList.Contains(ResourceName))
	{
		int32 *temp = ResourceList.Find(ResourceName);
		ResourceList.Add(ResourceName, *temp+1);
	}
}

void UResourceCountComponent::AddResources(const TMap<FString, int32> CostsMap)
{
	int32 *available;
	FString resource;
	for (const TPair<FString, int32>& Cost : CostsMap)
	{
		available = ResourceList.Find(Cost.Key);
		resource = Cost.Key;
		if (ResourceList.Contains(resource))
		{
			UE_LOG(LogTemp, Warning, TEXT("Adding %s: %i"), *resource, *available);
			FResourceType* Row = ResourceDataTable->FindRow<FResourceType>(*resource, "Resource Description");	
			if(Row->bIsSpendable)
			{
				ResourceList.Add(Cost.Key, *available + Cost.Value);
			}
		}
	}
}

void UResourceCountComponent::SpendResources(const TMap<FString, int32> CostsMap)
{
	if (!ResourceDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Resource Data Table not found!"));
		return;

	}
	int32 *available;
	FString resource;
	for (const TPair<FString, int32>& Cost : CostsMap)
	{
		available = ResourceList.Find(Cost.Key);
		resource = Cost.Key;
		FResourceType* Row = ResourceDataTable->FindRow<FResourceType>(*resource, "Resource Description");	
		if(Row->bIsSpendable)
		{
			ResourceList.Add(Cost.Key, *available - Cost.Value);
		}
		
	}
}

bool UResourceCountComponent::CanAfford(const TMap<FString, int32> CostsMap)
{
	int32 *available;
	for (const TPair<FString, int32>& Cost : CostsMap)
	{
		available = ResourceList.Find(Cost.Key);
		if (Cost.Value > *available )
		{
			return false;
		}
	}

	return true;
}




