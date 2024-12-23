// ©David John Nolan, 2024


#include "UtilityBlueprintFunctionLibrary.h"

bool UUtilityBlueprintFunctionLibrary::TryRequestGameplayTag(FName TagName, FGameplayTag &Tag)
{
    Tag = UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);
	return Tag.IsValid();
}
