// ©David Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "GenericEnums.h"
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

	UFUNCTION(BlueprintPure)
		bool GetPlayTutorial() const;

	UFUNCTION(BlueprintCallable)
		void SetPlayTutorial(bool NewValue);

	UFUNCTION(BlueprintPure)
		bool GetShowFPS() const;

	UFUNCTION(BlueprintCallable)
		void SetShowFPS(bool NewValue);

	UFUNCTION(BlueprintPure)
		float GetVolumeMain() const;

	UFUNCTION(BlueprintCallable)
		void SetVolumeMain(float NewValue);

	UFUNCTION(BlueprintPure)
		float GetVolumeMusic() const;

	UFUNCTION(BlueprintCallable)
		void SetVolumeMusic(float NewValue);

	UFUNCTION(BlueprintPure)
		float GetVolumeFX() const;

	UFUNCTION(BlueprintCallable)
		void SetVolumeFX(float NewValue);
	
	UFUNCTION(BlueprintPure)
		float GetBuildCursorSpeed() const;

	UFUNCTION(BlueprintCallable)
		void SetBuildCursorSpeed(float NewValue);

protected:
	UPROPERTY(Config)
		bool bPlayTutorial = true;

	UPROPERTY(Config)
		bool bShowFPS = false;

	UPROPERTY(Config)
		float VolumeMain = 1.0f; 

	UPROPERTY(Config)
		float VolumeMusic = 1.0f;

	UPROPERTY(Config)
		float VolumeFX = 1.0f;

	UPROPERTY(BlueprintReadWrite, Config)
		TEnumAsByte<ELockCamera> LockCamera;

	UPROPERTY(Config)
		float BuildCursorSpeed = 2.5f; 


};


