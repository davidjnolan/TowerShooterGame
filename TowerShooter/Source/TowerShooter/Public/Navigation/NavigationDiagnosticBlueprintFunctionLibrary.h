// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigationDiagnosticBlueprintFunctionLibrary.generated.h"

class AAIController;
class UCharacterMovementComponent;

/**
 * Blueprint-accessible diagnostics for inspecting Unreal navigation behaviour.
 *
 * These helpers exist to answer development/debugging questions such as which
 * Supported Agent NavData was selected, whether a point projects onto that
 * NavMesh, or why an AI move request behaves differently between enemy classes.
 * They should not normally become dependencies of gameplay logic.
 */
UCLASS()
class TOWERSHOOTER_API UNavigationDiagnosticBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Reads the navigation-agent radius and height currently reported by a
     * CharacterMovementComponent.
     *
     * This reports the values Unreal uses for navigation-agent selection, which
     * is useful when checking that runtime values match the intended capsule size
     * and Supported Agent configuration.
     * Outputs are reset to zero if CharacterMovement is invalid.
     */
    UFUNCTION(BlueprintPure, Category = "Navigation|Debug")
    static void GetNavAgentDimensions(
        UCharacterMovementComponent* CharacterMovement,
        float& AgentRadius,
        float& AgentHeight);

    /**
     * Resolves the NavData Unreal selects for the supplied CharacterMovementComponent.
     *
     * The component's FNavAgentProperties are passed to NavigationSystem, making
     * this a convenient way to verify Supported Agent selection at runtime.
     * Returns a descriptive error string when the query cannot be completed.
     */
    UFUNCTION(BlueprintPure, Category = "Navigation|Debug", meta = (WorldContext = "WorldContextObject"))
    static FString GetNavDataNameForCharacterMovement(
        UObject* WorldContextObject,
        UCharacterMovementComponent* CharacterMovement);

    /**
     * Attempts to project a world-space point onto the NavData selected for the
     * supplied CharacterMovementComponent.
     *
     * Unlike generic projection helpers, this explicitly resolves NavData from
     * the character's navigation-agent properties, so it can distinguish Medium,
     * Large, or other Supported Agent meshes during testing.
     *
     * @return True when the point could be projected onto the selected NavData.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Debug", meta = (WorldContext = "WorldContextObject"))
    static bool ProjectPointToCharacterNavMesh(
        UObject* WorldContextObject,
        UCharacterMovementComponent* CharacterMovement,
        const FVector& Point,
        FVector& ProjectedPoint);

    /**
     * Tests whether a complete path exists between Start and End on the NavData
     * selected for the supplied CharacterMovementComponent.
     *
     * This bypasses AIController/PathFollowing behaviour and is therefore useful
     * for separating pure NavMesh/pathfinding problems from movement-system problems.
     * Partial paths are deliberately treated as failure.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Debug", meta = (WorldContext = "WorldContextObject"))
    static bool DoesCharacterHaveNavPath(
        UObject* WorldContextObject,
        UCharacterMovementComponent* CharacterMovement,
        const FVector& Start,
        const FVector& End);

    /**
     * Returns a compact text snapshot of an AI Controller's current path-following
     * state, including agent dimensions, status, installed path, movement authority,
     * current move input and the PathFollowingComponent's own debug string.
     *
     * Intended for side-by-side comparison of working and failing AI agents.
     */
    UFUNCTION(BlueprintPure, Category = "Navigation|Debug")
    static FString GetAIPathFollowingDebugInfo(AAIController* AIController);

    /**
     * Builds the same pathfinding query an AIController would build for a normal
     * location move, executes that query directly, and reports the selected NavData
     * and resulting path state.
     *
     * This is useful for proving whether failure occurs during query/pathfinding
     * or later when the path is handed to PathFollowingComponent.
     */
    UFUNCTION(BlueprintPure, Category = "Navigation|Debug")
    static FString GetAIMoveQueryDebugInfo(
        AAIController* AIController,
        const FVector& Destination);

    /**
     * Deep diagnostic for the hand-off between AI pathfinding and path following.
     *
     * The function manually builds a normal FAIMoveRequest, asks the controller to
     * build its pathfinding query, finds the path, attaches observers, and then calls
     * PathFollowingComponent::RequestMove directly. The returned text records whether
     * RequestMove synchronously installs, rejects, or finishes that path.
     *
     * IMPORTANT: this function actually issues a move request and is intended only
     * for controlled debugging. It is not a read-only inspection helper and should
     * not be used as gameplay movement logic.
     */
    UFUNCTION(BlueprintCallable, Category = "Navigation|Debug")
    static FString TestAIControllerMoveTo(
        AAIController* AIController,
        const FVector& Destination);
};
