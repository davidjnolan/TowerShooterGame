// ©David Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "ResourceCountComponent.generated.h"



UCLASS(Blueprintable)
class TOWERSHOOTER_API UResourceCountComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UResourceCountComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Parent Only-DO NOT MODIFY")
		TMap<FString, int32> ResourceList;

	UPROPERTY(EditDefaultsOnly, category="ResourceDefaults")
		UDataTable* ResourceDataTable;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
	UFUNCTION(BlueprintCallable)
		void IncrementResource(FString ResourceName);

	UFUNCTION(BlueprintCallable)
		void AddResources(const TMap<FString, int32> CostsMap);

	UFUNCTION(BlueprintCallable)
		void SpendResources(const TMap<FString, int32> CostsMap);

	UFUNCTION(BlueprintPure)
		bool CanAfford(const TMap<FString, int32> CostsMap);
};
