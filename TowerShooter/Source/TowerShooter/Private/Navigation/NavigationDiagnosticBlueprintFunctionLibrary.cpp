// ©David John Nolan, 2024

#include "Navigation/NavigationDiagnosticBlueprintFunctionLibrary.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationData.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

// Report the runtime navigation-agent dimensions, not merely the authored capsule defaults.
// These are the properties Unreal uses when matching a pawn to Supported Agent NavData.
void UNavigationDiagnosticBlueprintFunctionLibrary::GetNavAgentDimensions(
    UCharacterMovementComponent* CharacterMovement,
    float& AgentRadius,
    float& AgentHeight)
{
    AgentRadius = 0.0f;
    AgentHeight = 0.0f;

    if (!CharacterMovement)
    {
        return;
    }

    const FNavAgentProperties& NavAgentProps =
        CharacterMovement->GetNavAgentPropertiesRef();

    AgentRadius = NavAgentProps.AgentRadius;
    AgentHeight = NavAgentProps.AgentHeight;
}


// Resolve NavData from the movement component's agent properties so the result mirrors Supported Agent selection.
// Returning the object name makes Medium/Large NavData differences easy to inspect from Blueprint.
FString UNavigationDiagnosticBlueprintFunctionLibrary::GetNavDataNameForCharacterMovement(
    UObject* WorldContextObject,
    UCharacterMovementComponent* CharacterMovement)
{
    if (!WorldContextObject || !CharacterMovement)
    {
        return TEXT("Invalid Input");
    }

    UNavigationSystemV1* NavSystem =
        UNavigationSystemV1::GetCurrent(WorldContextObject);

    if (!NavSystem)
    {
        return TEXT("No Navigation System");
    }

    const FNavAgentProperties& AgentProperties =
        CharacterMovement->GetNavAgentPropertiesRef();

    ANavigationData* NavData =
        NavSystem->GetNavDataForProps(AgentProperties);

    if (!NavData)
    {
        return TEXT("No Matching NavData");
    }

    return NavData->GetName();
}


// Project specifically against the NavData selected for this character's agent properties.
// This avoids accidentally testing only the world's default NavMesh when diagnosing multiple Supported Agents.
bool UNavigationDiagnosticBlueprintFunctionLibrary::ProjectPointToCharacterNavMesh(
    UObject* WorldContextObject,
    UCharacterMovementComponent* CharacterMovement,
    const FVector& Point,
    FVector& ProjectedPoint)
{
    ProjectedPoint = FVector::ZeroVector;

    if (!WorldContextObject || !CharacterMovement)
    {
        return false;
    }

    UNavigationSystemV1* NavSystem =
        UNavigationSystemV1::GetCurrent(WorldContextObject);

    if (!NavSystem)
    {
        return false;
    }

    const FNavAgentProperties& AgentProperties =
        CharacterMovement->GetNavAgentPropertiesRef();

    ANavigationData* NavData =
        NavSystem->GetNavDataForProps(AgentProperties);

    if (!NavData)
    {
        return false;
    }

    FNavLocation NavLocation;

    const bool bSuccess = NavData->ProjectPoint(
        Point,
        NavLocation,
        FVector(500.0f, 500.0f, 500.0f));

    if (bSuccess)
    {
        ProjectedPoint = NavLocation.Location;
    }

    return bSuccess;
}


// Perform pathfinding directly on the NavData selected for this character.
// This deliberately bypasses AIController and PathFollowingComponent so those systems cannot affect the result.
bool UNavigationDiagnosticBlueprintFunctionLibrary::DoesCharacterHaveNavPath(
    UObject* WorldContextObject,
    UCharacterMovementComponent* CharacterMovement,
    const FVector& Start,
    const FVector& End)
{
    if (!WorldContextObject || !CharacterMovement)
    {
        return false;
    }

    UNavigationSystemV1* NavSystem =
        UNavigationSystemV1::GetCurrent(WorldContextObject);

    if (!NavSystem)
    {
        return false;
    }

    const FNavAgentProperties& AgentProperties =
        CharacterMovement->GetNavAgentPropertiesRef();

    ANavigationData* NavData =
        NavSystem->GetNavDataForProps(AgentProperties);

    if (!NavData)
    {
        return false;
    }

    FPathFindingQuery Query(
        nullptr,
        *NavData,
        Start,
        End);

    const FPathFindingResult Result =
        NavData->FindPath(
            AgentProperties,
            Query);

    return Result.IsSuccessful() &&
           Result.Path.IsValid() &&
           !Result.Path->IsPartial();
}


// Capture the current PathFollowingComponent state in one Blueprint-readable string.
// This is intended for comparing agents at the same moment rather than driving gameplay decisions.
FString UNavigationDiagnosticBlueprintFunctionLibrary::GetAIPathFollowingDebugInfo(
    AAIController* AIController)
{
    if (!AIController)
    {
        return TEXT("Invalid AIController");
    }

    UPathFollowingComponent* PathFollowing =
        AIController->GetPathFollowingComponent();

    if (!PathFollowing)
    {
        return TEXT("No PathFollowingComponent");
    }

    const FNavAgentProperties& ControllerProps =
        AIController->GetNavAgentPropertiesRef();

    const FNavPathSharedPtr CurrentPath =
        PathFollowing->GetPath();

    const FString PathState =
        CurrentPath.IsValid()
            ? TEXT("Valid")
            : TEXT("None");

    return FString::Printf(
		TEXT(
			"Controller: %s\n"
			"Agent Radius: %.2f\n"
			"Agent Height: %.2f\n"
			"PathFollowing Status: %s\n"
			"Current Path: %s\n"
			"Has Movement Authority: %s\n"
			"Current Move Input: %s\n"
			"Debug: %s"
		),
		*AIController->GetName(),
		ControllerProps.AgentRadius,
		ControllerProps.AgentHeight,
		*PathFollowing->GetStatusDesc(),
		*PathState,
		PathFollowing->HasMovementAuthority() ? TEXT("True") : TEXT("False"),
		*PathFollowing->GetCurrentMoveInput().ToString(),
		*PathFollowing->GetDebugString()
	);
}


// Recreate the AIController query-building stage without handing the resulting path to PathFollowingComponent.
// If this succeeds while movement fails, the fault lies after pathfinding rather than in NavMesh generation/query selection.
FString UNavigationDiagnosticBlueprintFunctionLibrary::GetAIMoveQueryDebugInfo(
    AAIController* AIController,
    const FVector& Destination)
{
    if (!AIController)
    {
        return TEXT("Invalid AIController");
    }

    APawn* Pawn = AIController->GetPawn();

    if (!Pawn)
    {
        return TEXT("AIController has no Pawn");
    }

    // Construct a move request equivalent to the one the AI Controller
    // would normally use when moving to a location.
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(Destination);
    MoveRequest.SetUsePathfinding(true);
    MoveRequest.SetAllowPartialPath(false);
    MoveRequest.SetProjectGoalLocation(true);

    // Ask the AI Controller to construct its own pathfinding query.
    // This ensures we test the same NavData/filter selection used by
    // the normal AI movement pipeline.
    FPathFindingQuery Query;

    const bool bQueryBuilt =
        AIController->BuildPathfindingQuery(
            MoveRequest,
            Query);

    if (!bQueryBuilt)
    {
        return FString::Printf(
            TEXT(
                "Controller: %s\n"
                "Pawn: %s\n"
                "BuildPathfindingQuery: FAILED"
            ),
            *AIController->GetName(),
            *Pawn->GetName()
        );
    }

    // Get the NavData selected by the controller-generated query.
    const ANavigationData* QueryNavData =
        Query.NavData.Get();

    const FString NavDataName =
        QueryNavData
            ? QueryNavData->GetName()
            : TEXT("None");

    const FNavAgentProperties& AgentProps =
        AIController->GetNavAgentPropertiesRef();

    // Execute the exact query constructed by the AI Controller.
    FPathFindingResult PathResult;

    if (QueryNavData)
    {
        PathResult = QueryNavData->FindPath(
            AgentProps,
            Query);
    }

    const bool bPathValid =
        PathResult.Path.IsValid();

    const bool bPathPartial =
        bPathValid && PathResult.Path->IsPartial();

    const int32 NumPathPoints =
        bPathValid
            ? PathResult.Path->GetPathPoints().Num()
            : 0;

    return FString::Printf(
        TEXT(
            "Controller: %s\n"
            "Pawn: %s\n"
            "Agent Radius: %.2f\n"
            "Agent Height: %.2f\n"
            "BuildPathfindingQuery: SUCCESS\n"
            "Query NavData: %s\n"
            "FindPath Successful: %s\n"
            "Path Valid: %s\n"
            "Path Partial: %s\n"
            "Path Points: %d"
        ),
        *AIController->GetName(),
        *Pawn->GetName(),
        AgentProps.AgentRadius,
        AgentProps.AgentHeight,
        *NavDataName,
        PathResult.IsSuccessful() ? TEXT("True") : TEXT("False"),
        bPathValid ? TEXT("True") : TEXT("False"),
        bPathPartial ? TEXT("True") : TEXT("False"),
        NumPathPoints
    );
}


// Deep diagnostic: manually reproduce the pathfinding-to-PathFollowing hand-off.
// Observers are attached before RequestMove so synchronous rejection can be distinguished from later path invalidation.
FString UNavigationDiagnosticBlueprintFunctionLibrary::TestAIControllerMoveTo(
    AAIController* AIController,
    const FVector& Destination)
{
    if (!AIController)
    {
        return TEXT("ERROR: AIController is null");
    }

    UPathFollowingComponent* PathFollowing =
        AIController->GetPathFollowingComponent();

    if (!PathFollowing)
    {
        return TEXT("ERROR: PathFollowingComponent is null");
    }

    FString DebugOutput;

    DebugOutput += FString::Printf(
        TEXT("Controller: %s\n"),
        *AIController->GetName());

    DebugOutput += FString::Printf(
        TEXT("PathFollowing Class: %s\n"),
        *PathFollowing->GetClass()->GetName());

    // ------------------------------------------------------------
    // Build the move request.
    // ------------------------------------------------------------

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(Destination);
    MoveRequest.SetUsePathfinding(true);
    MoveRequest.SetAllowPartialPath(false);
    MoveRequest.SetProjectGoalLocation(true);

    // ------------------------------------------------------------
    // Build the pathfinding query.
    // ------------------------------------------------------------

    FPathFindingQuery Query;

    const bool bQueryBuilt =
        AIController->BuildPathfindingQuery(MoveRequest, Query);

    DebugOutput += FString::Printf(
        TEXT("BuildPathfindingQuery: %s\n"),
        bQueryBuilt ? TEXT("SUCCESS") : TEXT("FAILED"));

    if (!bQueryBuilt)
    {
        return DebugOutput;
    }

    const ANavigationData* QueryNavData = Query.NavData.Get();

    DebugOutput += FString::Printf(
        TEXT("Query NavData: %s\n"),
        QueryNavData
            ? *QueryNavData->GetName()
            : TEXT("None"));

    if (!QueryNavData)
    {
        DebugOutput += TEXT("ERROR: Query has no NavData");
        return DebugOutput;
    }

    // ------------------------------------------------------------
    // Find the path before giving it to PathFollowing.
    // ------------------------------------------------------------

    const FNavAgentProperties& AgentProps =
        AIController->GetNavAgentPropertiesRef();

    FPathFindingResult PathResult =
        QueryNavData->FindPath(AgentProps, Query);

    FNavPathSharedPtr Path = PathResult.Path;

    DebugOutput += FString::Printf(
        TEXT("FindPath Successful: %s\n"),
        PathResult.IsSuccessful() ? TEXT("True") : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("Path Valid Before RequestMove: %s\n"),
        (Path.IsValid() && Path->IsValid())
            ? TEXT("True")
            : TEXT("False"));

    if (!Path.IsValid())
    {
        DebugOutput += TEXT("ERROR: No path returned");
        return DebugOutput;
    }

    DebugOutput += FString::Printf(
        TEXT("Path Points: %d\n"),
        Path->GetPathPoints().Num());

    DebugOutput += FString::Printf(
        TEXT("Path NavData: %s\n"),
        Path->GetNavigationDataUsed()
            ? *Path->GetNavigationDataUsed()->GetName()
            : TEXT("None"));

    // Match the normal AAIController path setup.
    Path->EnableRecalculationOnInvalidation(true);

    // ------------------------------------------------------------
    // Attach a path observer BEFORE RequestMove.
    // ------------------------------------------------------------

    TArray<ENavPathEvent::Type> PathEvents;

    const FDelegateHandle PathObserverHandle =
        Path->AddObserver(
            FNavigationPath::FPathObserverDelegate::FDelegate::CreateLambda(
                [&PathEvents](
                    FNavigationPath* UpdatedPath,
                    ENavPathEvent::Type Event)
                {
                    PathEvents.Add(Event);
                }));

    // ------------------------------------------------------------
    // Listen for RequestMove finishing synchronously.
    // ------------------------------------------------------------

    bool bRequestFinished = false;

    EPathFollowingResult::Type FinishedResult =
        EPathFollowingResult::Invalid;

    uint16 FinishedFlags = 0;

    const FDelegateHandle FinishedHandle =
        PathFollowing->OnRequestFinished.AddLambda(
            [&bRequestFinished, &FinishedResult, &FinishedFlags](
                FAIRequestID RequestID,
                const FPathFollowingResult& Result)
            {
                bRequestFinished = true;
                FinishedResult = Result.Code;
                FinishedFlags = Result.Flags;
            });

    // ------------------------------------------------------------
    // NEW DIAGNOSTICS:
    // Check PathFollowing state immediately before RequestMove.
    // ------------------------------------------------------------

    DebugOutput += FString::Printf(
        TEXT("PathFollowing Active Before RequestMove: %s\n"),
        PathFollowing->IsActive()
            ? TEXT("True")
            : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("PathFollowing Allowed Before RequestMove: %s\n"),
        PathFollowing->IsPathFollowingAllowed()
            ? TEXT("True")
            : TEXT("False"));

    // ------------------------------------------------------------
    // Critical test: give the valid path directly to PathFollowing.
    // ------------------------------------------------------------

    DebugOutput += TEXT("Calling PathFollowing->RequestMove...\n");

    const FAIRequestID RequestID =
        PathFollowing->RequestMove(MoveRequest, Path);

    // ------------------------------------------------------------
    // NEW DIAGNOSTICS:
    // Check PathFollowing state immediately after RequestMove.
    // ------------------------------------------------------------

    DebugOutput += FString::Printf(
        TEXT("PathFollowing Active After RequestMove: %s\n"),
        PathFollowing->IsActive()
            ? TEXT("True")
            : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("PathFollowing Allowed After RequestMove: %s\n"),
        PathFollowing->IsPathFollowingAllowed()
            ? TEXT("True")
            : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("RequestID Valid: %s\n"),
        RequestID.IsValid()
            ? TEXT("True")
            : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("PathFollowing Status After RequestMove: %s\n"),
        *PathFollowing->GetStatusDesc());

    const FNavPathSharedPtr InstalledPath =
        PathFollowing->GetPath();

    DebugOutput += FString::Printf(
        TEXT("Installed Path After RequestMove: %s\n"),
        InstalledPath.IsValid()
            ? TEXT("Valid")
            : TEXT("None"));

    DebugOutput += FString::Printf(
        TEXT("Original Path IsValid After RequestMove: %s\n"),
        Path->IsValid()
            ? TEXT("True")
            : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("Original Path IsUpToDate: %s\n"),
        Path->IsUpToDate()
            ? TEXT("True")
            : TEXT("False"));

    DebugOutput += FString::Printf(
        TEXT("Original Path WaitingForRepath: %s\n"),
        Path->IsWaitingForRepath()
            ? TEXT("True")
            : TEXT("False"));

    // ------------------------------------------------------------
    // Report any path events that occurred during RequestMove.
    // ------------------------------------------------------------

    if (PathEvents.Num() == 0)
    {
        DebugOutput += TEXT("Path Events During RequestMove: NONE\n");
    }
    else
    {
        DebugOutput += FString::Printf(
            TEXT("Path Events During RequestMove: %d\n"),
            PathEvents.Num());

        for (int32 Index = 0; Index < PathEvents.Num(); ++Index)
        {
            DebugOutput += FString::Printf(
                TEXT("  Event %d: %d\n"),
                Index,
                static_cast<int32>(PathEvents[Index]));
        }
    }

    // ------------------------------------------------------------
    // Report whether RequestMove immediately aborted.
    // ------------------------------------------------------------

    if (bRequestFinished)
    {
        DebugOutput += TEXT("OnRequestFinished FIRED\n");

        DebugOutput += FString::Printf(
            TEXT("Result Code: %d\n"),
            static_cast<int32>(FinishedResult));

        DebugOutput += FString::Printf(
            TEXT("Result Flags: %u\n"),
            FinishedFlags);

        DebugOutput += TEXT("Decoded Flags:");

        if (FinishedFlags & FPathFollowingResultFlags::Success)
        {
            DebugOutput += TEXT(" Success");
        }

        if (FinishedFlags & FPathFollowingResultFlags::Blocked)
        {
            DebugOutput += TEXT(" Blocked");
        }

        if (FinishedFlags & FPathFollowingResultFlags::OffPath)
        {
            DebugOutput += TEXT(" OffPath");
        }

        if (FinishedFlags & FPathFollowingResultFlags::UserAbort)
        {
            DebugOutput += TEXT(" UserAbort");
        }

        if (FinishedFlags & FPathFollowingResultFlags::InvalidPath)
        {
            DebugOutput += TEXT(" InvalidPath");
        }

        if (FinishedFlags & FPathFollowingResultFlags::MovementStop)
        {
            DebugOutput += TEXT(" MovementStop");
        }

        if (FinishedFlags & FPathFollowingResultFlags::NewRequest)
        {
            DebugOutput += TEXT(" NewRequest");
        }

        if (FinishedFlags & FPathFollowingResultFlags::ForcedScript)
        {
            DebugOutput += TEXT(" ForcedScript");
        }

        DebugOutput += TEXT("\n");
    }
    else
    {
        DebugOutput += TEXT("OnRequestFinished: NOT FIRED\n");
    }

    // Remove our temporary diagnostic delegates.
    Path->RemoveObserver(PathObserverHandle);
    PathFollowing->OnRequestFinished.Remove(FinishedHandle);

    return DebugOutput;
}
