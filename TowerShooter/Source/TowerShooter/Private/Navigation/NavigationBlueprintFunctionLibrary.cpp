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
