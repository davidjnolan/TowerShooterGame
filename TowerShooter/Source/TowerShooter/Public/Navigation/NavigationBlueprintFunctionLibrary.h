// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationBlueprintFunctionLibrary.generated.h"

class AAIController;
class UNavArea;
class UNavModifierComponent;
class UNavigationQueryFilter;

/**
 * Describes one sampled segment of a straight-line navigation cost query.
 *
 * The segment is assigned to the NavMesh area found at its midpoint and
 * records the effective travel and entering costs from the navigation filter.
 * SegmentCost contains the resulting contribution to the total direct cost.
 */
USTRUCT(BlueprintType)
struct FDirectNavCostSegment
{
    GENERATED_BODY()

    /** World-space start position of this sampled segment. */
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    FVector Start = FVector::ZeroVector;

    /** World-space end position of this sampled segment. */
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    FVector End = FVector::ZeroVector;

    /** Recast NavArea ID assigned to the NavMesh polygon at the sample point. */
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    int32 AreaID = INDEX_NONE;

    /** Distance multiplier applied while travelling through this NavArea. */
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    float TravelCost = 0.0f;

    /** Fixed cost applied when entering this NavArea from another area. */
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    float EnteringCost = 0.0f;

    /** Cost contributed by this segment to the complete direct route. */
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    double SegmentCost = 0.0;
};

/**
 * Blueprint-accessible navigation helpers intended for real gameplay use.
 *
 * Keep functions here when they perform useful navigation work or expose a
 * navigation capability that gameplay systems may legitimately depend on.
 * Pure investigation/debug-only helpers belong in
 * UNavigationDiagnosticBlueprintFunctionLibrary instead.
 */
UCLASS()
class TOWERSHOOTER_API UNavigationBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Returns the default traversal multiplier configured on a NavArea class.
     *
     * This reads the class default object directly; it does not create an area
     * instance and does not include overrides supplied by a Navigation Query Filter.
     *
     * @param NavAreaClass NavArea class whose default traversal cost is required.
     * @return The NavArea DefaultCost, or 0.0 if the class reference is invalid.
     */
    UFUNCTION(BlueprintPure, Category = "Navigation")
    static float GetNavAreaDefaultCost(TSubclassOf<UNavArea> NavAreaClass);

    /**
     * Approximates the navigation cost of travelling directly from Start to End.
     *
     * Unlike normal pathfinding, this function does not search for a cheaper route.
     * It samples the straight line, projects each sample onto the default Recast
     * NavMesh, determines the NavArea under that sample, and accumulates the
     * effective travel/entering costs from the supplied query filter.
     *
     * This is useful for gameplay decisions such as deciding whether a direct route
     * through expensive/destructible structures is preferable to another behaviour.
     * The result is an approximation: smaller SampleSpacing values resolve NavArea
     * boundaries more accurately but require more projection queries.
     *
     * Important: the current implementation deliberately uses the world's DEFAULT
     * NavData. It does not select NavData from a particular character's agent props.
     *
     * @param WorldContextObject Object used to obtain the current world/navigation system.
     * @param Start World-space start of the straight route.
     * @param End World-space end of the straight route.
     * @param FilterClass Optional Navigation Query Filter; default NavMesh filter if unset.
     * @param SampleSpacing Approximate distance in Unreal units between samples; must be > 0.
     * @param DirectCost Total estimated navigation cost of the complete straight route.
     * @param Segments Per-segment breakdown of sampled areas and their cost contribution.
     * @return True if every sample could be resolved and costed successfully.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation", meta = (WorldContext = "WorldContextObject"))
    static bool GetDirectNavPathCost(
        UObject* WorldContextObject,
        const FVector& Start,
        const FVector& End,
        TSubclassOf<UNavigationQueryFilter> FilterClass,
        float SampleSpacing,
        double& DirectCost,
        TArray<FDirectNavCostSegment>& Segments);

    /**
     * Returns the current world-space navigation bounds contributed by a
     * NavModifierComponent.
     *
     * Center and Extent are reset to zero when NavModifier is null, so Blueprint
     * callers never retain stale values from an earlier successful query.
     */
    UFUNCTION(BlueprintPure, Category = "Navigation")
    static void GetNavModifierBounds(
        UNavModifierComponent* NavModifier,
        FVector& Center,
        FVector& Extent);

    /**
     * Enables or disables Detour Crowd simulation on an AI Controller whose
     * PathFollowingComponent is a CrowdFollowingComponent.
     *
     * Disabling crowd simulation does NOT disable AI navigation. The existing
     * CrowdFollowingComponent falls back to the normal PathFollowingComponent
     * path-following implementation while crowd simulation is disabled.
     *
     * This currently provides a gameplay-facing switch while the project uses
     * multiple Supported Agent NavMeshes, because UE's CrowdManager may reject
     * a path whose Recast NavData differs from the CrowdManager's NavData.
     *
     * The CrowdFollowingComponent must be idle when changing simulation state;
     * Unreal refuses the state change while a move is already active.
     *
     * @param AIController Controller whose path-following component should be changed.
     * @param bEnabled True to enable crowd simulation; false to use normal path following.
     * @return True if the controller owns a CrowdFollowingComponent and the request was issued.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    static bool SetCrowdSimulationEnabled(
        AAIController* AIController,
        bool bEnabled);
};
