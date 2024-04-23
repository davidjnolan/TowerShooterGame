// ©David Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "UserSettings.generated.h"

/**
 * 
 */
UCLASS(Config=GameUserSettings, meta = (DisplayName="User Settings"))
class TOWERSHOOTER_API UUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UUserSettings* GetCustomUserSettings();

	UFUNCTION(BlueprintCallable)
	void SetSaveSlotName(FString NewValue);

	UFUNCTION(BlueprintPure)
	FString GetSaveSlotName() const;

protected:
	UPROPERTY(Config)
	FString SaveSlotName;
};


