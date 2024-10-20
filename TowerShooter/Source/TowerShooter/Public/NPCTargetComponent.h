// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "NPCTargetComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateOccupied);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Structure Settings")
		TSubclassOf<AActor> RequiredNPC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Structure Settings")
		FString Resource;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category="Structure Settings")
		FString ResourceCapacity;

	// Multicast Delegate
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FUpdateOccupied ReportOccupiedState;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UFUNCTION(BlueprintPure)
		bool GetIsOccupied() const;
	UFUNCTION(BlueprintCallable)
		void SetIsOccupied(bool value);
	
	UFUNCTION(BlueprintCallable)
		void OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION(BlueprintCallable)
		void OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void DetermineOccupiedState();



	

private:
	bool bIsOccupied;


};
