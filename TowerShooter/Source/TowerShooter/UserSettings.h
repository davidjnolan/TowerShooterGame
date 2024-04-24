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
	void SetPlayTutorial(bool NewValue);

	UFUNCTION(BlueprintPure)
	bool GetPlayTutorial() const;

protected:
	UPROPERTY(Config)
	bool bPlayTutorial;

};


