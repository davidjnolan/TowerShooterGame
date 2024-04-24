// Fill out your copyright notice in the Description page of Project Settings.


#include "UserSettings.h"

UUserSettings::UUserSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bPlayTutorial = true;
}

UUserSettings* UUserSettings::GetCustomUserSettings()
{
    return Cast<UUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UUserSettings::SetPlayTutorial(bool NewValue)
{
    bPlayTutorial = NewValue;
}

bool UUserSettings::GetPlayTutorial() const
{
    return bPlayTutorial;
}