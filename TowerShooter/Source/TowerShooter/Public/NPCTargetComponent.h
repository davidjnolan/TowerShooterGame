// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "NPCTargetComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class TOWERSHOOTER_API UNPCTargetComponent : public UBoxComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this component's properties
	UNPCTargetComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UFUNCTION(BlueprintPure)
		bool GetIsOccupied() const;
	
	UFUNCTION(BlueprintCallable)
		void SetIsOccupied(bool value);

	
	UFUNCTION(BlueprintCallable)
		void TriggerEnter(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	//UFUNCTION(BlueprintCallable)
		//void OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


private:
	bool bIsOccupied;


};
