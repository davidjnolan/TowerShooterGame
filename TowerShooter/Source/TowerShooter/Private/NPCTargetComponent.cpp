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
    // Register events in BeginPlay
    OnComponentBeginOverlap.AddDynamic(this, &UNPCTargetComponent::OnBeginOverlap); //Register the BeginOverlap event
    OnComponentEndOverlap.AddDynamic(this, &UNPCTargetComponent::OnEndOverlap);     //Register the EndOverlap event
	
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

void UNPCTargetComponent::OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor->IsA(RequiredNPC))
    {
        DetermineOccupiedState();
    }
}

void UNPCTargetComponent::OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor->IsA(RequiredNPC))
    {
        DetermineOccupiedState();
    }
}

void UNPCTargetComponent::DetermineOccupiedState()
{
    TArray<AActor*> Actors;
    GetOverlappingActors(Actors, RequiredNPC);

    bool PreviousOccupied = GetIsOccupied();

    if (Actors.Num() > 0)
    {
        SetIsOccupied(true);
    }
    else
    {
        SetIsOccupied(false);
    }

    if (PreviousOccupied == GetIsOccupied())
    {
        ReportOccupiedState.Broadcast();
    }


}