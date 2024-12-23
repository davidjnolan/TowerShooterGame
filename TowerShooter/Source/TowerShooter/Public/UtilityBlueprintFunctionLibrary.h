// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "UtilityBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TOWERSHOOTER_API UUtilityBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintPure, Category = "GameplayTags")
		static bool TryRequestGameplayTag(FName TagName, FGameplayTag& Tag);
};
