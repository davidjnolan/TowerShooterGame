// Fill out your copyright notice in the Description page of Project Settings.


#include "UserSettings.h"

UUserSettings::UUserSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SaveSlotName = TEXT("testytesttest");
}

UUserSettings* UUserSettings::GetCustomUserSettings()
{
    return Cast<UUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UUserSettings::SetSaveSlotName(FString NewValue)
{
    SaveSlotName = NewValue;
}

FString UUserSettings::GetSaveSlotName() const
{
    return SaveSlotName;
}