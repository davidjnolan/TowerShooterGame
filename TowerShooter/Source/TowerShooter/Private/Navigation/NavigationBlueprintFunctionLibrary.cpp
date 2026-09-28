// ©David John Nolan, 2024

#include "Navigation/NavigationBlueprintFunctionLibrary.h"

#include "AIController.h"
#include "NavAreas/NavArea.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavModifierComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationData.h"
#include "NavigationSystem.h"

// Read the class default object so this remains a cheap, allocation-free query.
// Note that this is the NavArea default only; query-filter overrides are not applied here.
float UNavigationBlueprintFunctionLibrary::GetNavAreaDefaultCost(TSubclassOf<UNavArea> NavAreaClass)
{
    // A null class reference cannot provide a class default object.
    if (!NavAreaClass)
    {
        return 0.0f;
    }

    // Get the Class Default Object (CDO) for the supplied Nav Area class.
    // The CDO contains the default property values configured for that class,
    // allowing us to read them without creating an instance.
    const UNavArea* DefaultArea = NavAreaClass->GetDefaultObject<UNavArea>();

    // Return the Nav Area's configured traversal cost.
    // Guard against an invalid CDO, although a valid NavAreaClass should
    // normally always provide one.
    return DefaultArea ? DefaultArea->DefaultCost : 0.0f;
}


// Sample and cost a straight route rather than asking pathfinding to choose a route.
// This is intentionally gameplay-capable code: callers receive both the total and a detailed breakdown.
bool UNavigationBlueprintFunctionLibrary::GetDirectNavPathCost(
    UObject* WorldContextObject,
    const FVector& Start,
    const FVector& End,
    TSubclassOf<UNavigationQueryFilter> FilterClass,
    float SampleSpacing,
    double& DirectCost,
    TArray<FDirectNavCostSegment>& Segments)
{
    DirectCost = 0.0;
    Segments.Reset();

    if (!WorldContextObject || SampleSpacing <= 0.0f)
    {
        return false;
    }

    // Get the navigation system and its default Recast NavMesh.
    // This diagnostic relies on Recast polygon Area IDs, so other
    // navigation data types are not supported.
    UNavigationSystemV1* NavSystem =
        UNavigationSystemV1::GetCurrent(WorldContextObject);

    if (!NavSystem)
    {
        return false;
    }

    ANavigationData* NavData =
        NavSystem->GetDefaultNavDataInstance(
            FNavigationSystem::DontCreate);

    ARecastNavMesh* RecastNavMesh = Cast<ARecastNavMesh>(NavData);

    if (!RecastNavMesh)
    {
        return false;
    }

    // Build the effective navigation query filter. Using the initialized
    // filter rather than reading NavArea defaults directly ensures any
    // NavigationQueryFilter cost overrides are included.
    FSharedConstNavQueryFilter QueryFilter;

    if (FilterClass)
    {
        QueryFilter =
            UNavigationQueryFilter::GetQueryFilter(
                *RecastNavMesh,
                WorldContextObject,
                FilterClass);
    }
    else
    {
        QueryFilter = RecastNavMesh->GetDefaultQueryFilter();
    }

    if (!QueryFilter.IsValid())
    {
        return false;
    }

    // Retrieve the effective travel and fixed entering costs for every
    // NavArea supported by this NavMesh, indexed by Recast Area ID.
    const int32 MaxAreas = RecastNavMesh->GetMaxSupportedAreas();

    TArray<float> TravelCosts;
    TArray<float> EnteringCosts;

    TravelCosts.SetNumZeroed(MaxAreas);
    EnteringCosts.SetNumZeroed(MaxAreas);

    QueryFilter->GetAllAreaCosts(
        TravelCosts.GetData(),
        EnteringCosts.GetData(),
        MaxAreas);

    const double TotalDistance = FVector::Distance(Start, End);

    // A zero-length route has no traversal cost, but is still a valid query.
    if (TotalDistance <= UE_KINDA_SMALL_NUMBER)
    {
        return true;
    }

    // Divide the direct route into approximately equal samples.
    // The final segment may be shorter than SampleSpacing.
    const int32 NumSegments =
        FMath::Max(1, FMath::CeilToInt(TotalDistance / SampleSpacing));

    const FVector Direction = (End - Start) / TotalDistance;

    int32 PreviousAreaID = INDEX_NONE;

    for (int32 SegmentIndex = 0;
         SegmentIndex < NumSegments;
         ++SegmentIndex)
    {
        const double SegmentStartDistance =
            SegmentIndex * SampleSpacing;

        const double SegmentEndDistance =
            FMath::Min(
                (SegmentIndex + 1) * SampleSpacing,
                TotalDistance);

        const FVector SegmentStart =
            Start + Direction * SegmentStartDistance;

        const FVector SegmentEnd =
            Start + Direction * SegmentEndDistance;

        const double SegmentLength =
            SegmentEndDistance - SegmentStartDistance;

        // Classify each segment using its midpoint. This makes the result an
        // approximation whose boundary accuracy depends on SampleSpacing.
        const FVector SamplePoint =
            (SegmentStart + SegmentEnd) * 0.5;

        FNavLocation ProjectedLocation;

        const FVector ProjectionExtent(
            SampleSpacing,
            SampleSpacing,
            100.0f);

        // Project the sample onto the NavMesh so we can retrieve the Recast
        // polygon reference and therefore its assigned NavArea.
        const bool bProjected =
            RecastNavMesh->ProjectPoint(
                SamplePoint,
                ProjectedLocation,
                ProjectionExtent,
                QueryFilter,
                WorldContextObject);

        // Treat any sample that cannot be resolved to a valid NavMesh polygon
        // as a failed direct-route query rather than returning a partial cost.
        if (!bProjected || ProjectedLocation.NodeRef == INVALID_NAVNODEREF)
        {
            Segments.Reset();
            DirectCost = 0.0;
            return false;
        }

        const uint8 AreaID =
            RecastNavMesh->GetPolyAreaID(
                ProjectedLocation.NodeRef);

        if (!TravelCosts.IsValidIndex(AreaID) ||
            !EnteringCosts.IsValidIndex(AreaID))
        {
            Segments.Reset();
            DirectCost = 0.0;
            return false;
        }

        const float TravelCost = TravelCosts[AreaID];
        const float EnteringCost = EnteringCosts[AreaID];

        // Travel cost scales with distance through the area.
        double SegmentCost =
            SegmentLength * TravelCost;

        // Fixed entering cost is charged once when crossing from one NavArea
        // into another. The starting area does not incur an entering cost.
        if (PreviousAreaID != INDEX_NONE &&
            PreviousAreaID != AreaID)
        {
            SegmentCost += EnteringCost;
        }

        FDirectNavCostSegment Segment;
        Segment.Start = SegmentStart;
        Segment.End = SegmentEnd;
        Segment.AreaID = AreaID;
        Segment.TravelCost = TravelCost;
        Segment.EnteringCost = EnteringCost;
        Segment.SegmentCost = SegmentCost;

        Segments.Add(Segment);

        DirectCost += SegmentCost;

        PreviousAreaID = AreaID;
    }

    return true;
}


// Expose the modifier's calculated navigation bounds in a Blueprint-friendly form.
// Always clear outputs first so an invalid component cannot leave stale Blueprint values behind.
void UNavigationBlueprintFunctionLibrary::GetNavModifierBounds(
    UNavModifierComponent* NavModifier,
    FVector& Center,
    FVector& Extent)
{
    Center = FVector::ZeroVector;
    Extent = FVector::ZeroVector;

    if (!NavModifier)
    {
        return;
    }

    const FBox Bounds = NavModifier->GetNavigationBounds();

    Center = Bounds.GetCenter();
    Extent = Bounds.GetExtent();
}


// Toggle the simulation state on the controller's existing CrowdFollowingComponent.
// When disabled, UCrowdFollowingComponent falls back to normal path following; it is not disabling navigation itself.
bool UNavigationBlueprintFunctionLibrary::SetCrowdSimulationEnabled(
    AAIController* AIController,
    bool bEnabled)
{
    if (!AIController)
    {
        return false;
    }

    UPathFollowingComponent* PathFollowing =
        AIController->GetPathFollowingComponent();

    if (!PathFollowing)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdFollowing =
        Cast<UCrowdFollowingComponent>(PathFollowing);

    if (!CrowdFollowing)
    {
        return false;
    }

    CrowdFollowing->SetCrowdSimulationState(
        bEnabled
            ? ECrowdSimulationState::Enabled
            : ECrowdSimulationState::Disabled);

    return true;
}


FString UNavigationBlueprintFunctionLibrary::GetCrowdFollowingDebugInfo(AAIController* AIController)
{
    // Make sure we were given a valid AI Controller.
    if (!AIController)
    {
        return TEXT("ERROR: AIController is null");
    }


    // Get the controller's Path Following Component and check that it
    // is actually a CrowdFollowingComponent.
    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return TEXT(
            "ERROR: PathFollowingComponent is not a CrowdFollowingComponent");
    }


    // Convert the avoidance quality enum into something readable.
    FString AvoidanceQualityString;

    switch (CrowdComp->GetCrowdAvoidanceQuality())
    {
        case ECrowdAvoidanceQuality::Low:
            AvoidanceQualityString = TEXT("Low");
            break;

        case ECrowdAvoidanceQuality::Medium:
            AvoidanceQualityString = TEXT("Medium");
            break;

        case ECrowdAvoidanceQuality::Good:
            AvoidanceQualityString = TEXT("Good");
            break;

        case ECrowdAvoidanceQuality::High:
            AvoidanceQualityString = TEXT("High");
            break;

        default:
            AvoidanceQualityString = TEXT("Unknown");
            break;
    }


    // Build one formatted string containing all the settings
    // we currently care about.
    return FString::Printf(
        TEXT(
            "Crowd Simulation Enabled: %s\n"
            "Crowd Simulation Active: %s\n"
            "Avoidance Quality: %s\n"
            "Obstacle Avoidance Enabled: %s\n"
            "Obstacle Avoidance Active: %s\n"
            "Avoidance Range Multiplier: %.2f\n"
            "Collision Query Range: %.2f\n"
            "Separation Enabled: %s\n"
            "Separation Active: %s\n"
            "Separation Weight: %.2f\n"
            "Anticipate Turns Enabled: %s\n"
            "Optimize Visibility Enabled: %s\n"
            "Optimize Topology Enabled: %s\n"
            "Path Offset Enabled: %s\n"
            "Path Optimization Range: %.2f"
        ),

        CrowdComp->IsCrowdSimulationEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->IsCrowdSimulationActive()
            ? TEXT("True") : TEXT("False"),

        *AvoidanceQualityString,

        CrowdComp->IsCrowdObstacleAvoidanceEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->IsCrowdObstacleAvoidanceActive()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->GetCrowdAvoidanceRangeMultiplier(),

        CrowdComp->GetCrowdCollisionQueryRange(),

        CrowdComp->IsCrowdSeparationEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->IsCrowdSeparationActive()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->GetCrowdSeparationWeight(),

        CrowdComp->IsCrowdAnticipateTurnsEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->IsCrowdOptimizeVisibilityEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->IsCrowdOptimizeTopologyEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->IsCrowdPathOffsetEnabled()
            ? TEXT("True") : TEXT("False"),

        CrowdComp->GetCrowdPathOptimizationRange()
    );
}


bool UNavigationBlueprintFunctionLibrary::GetCrowdMovementDebugInfo(
    AAIController* AIController,
    FVector& CrowdAgentVelocity,
    FVector& CrowdMoveDirection)
{
    CrowdAgentVelocity = FVector::ZeroVector;
    CrowdMoveDirection = FVector::ZeroVector;

    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdAgentVelocity = CrowdComp->GetCrowdAgentVelocity();
    CrowdMoveDirection = CrowdComp->CrowdAgentMoveDirection;

    return true;
}

bool UNavigationBlueprintFunctionLibrary::SetCrowdAvoidanceQuality(
    AAIController* AIController,
    ECrowdAvoidanceQualityBP Quality)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    ECrowdAvoidanceQuality::Type EngineQuality;

    switch (Quality)
    {
        case ECrowdAvoidanceQualityBP::Low:
            EngineQuality = ECrowdAvoidanceQuality::Low;
            break;

        case ECrowdAvoidanceQualityBP::Medium:
            EngineQuality = ECrowdAvoidanceQuality::Medium;
            break;

        case ECrowdAvoidanceQualityBP::Good:
            EngineQuality = ECrowdAvoidanceQuality::Good;
            break;

        case ECrowdAvoidanceQualityBP::High:
            EngineQuality = ECrowdAvoidanceQuality::High;
            break;

        default:
            return false;
    }

    CrowdComp->SetCrowdAvoidanceQuality(
        EngineQuality,
        true); // Update the registered crowd agent immediately.

    return true;
}


bool UNavigationBlueprintFunctionLibrary::SetCrowdSeparation(
    AAIController* AIController,
    bool bEnable)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdComp->SetCrowdSeparation(bEnable);

    return true;
}


bool UNavigationBlueprintFunctionLibrary::SetCrowdSeparationWeight(
    AAIController* AIController,
    float Weight)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdComp->SetCrowdSeparationWeight(Weight, true);

    return true;
}


bool UNavigationBlueprintFunctionLibrary::SetCrowdAnticipateTurns(
    AAIController* AIController,
    bool bEnable)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdComp->SetCrowdAnticipateTurns(bEnable);

    return true;
}


bool UNavigationBlueprintFunctionLibrary::SetCrowdPathOffset(
    AAIController* AIController,
    bool bEnable)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdComp->SetCrowdPathOffset(bEnable);

    return true;
}

bool UNavigationBlueprintFunctionLibrary::SetCrowdCollisionQueryRange(
    AAIController* AIController,
    float Range)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdComp->SetCrowdCollisionQueryRange(Range, true);

    return true;
}


bool UNavigationBlueprintFunctionLibrary::SetCrowdAvoidanceRangeMultiplier(
    AAIController* AIController,
    float Multiplier)
{
    if (!AIController)
    {
        return false;
    }

    UCrowdFollowingComponent* CrowdComp =
        Cast<UCrowdFollowingComponent>(
            AIController->GetPathFollowingComponent());

    if (!CrowdComp)
    {
        return false;
    }

    CrowdComp->SetCrowdAvoidanceRangeMultiplier(
        Multiplier,
        true);

    return true;
}