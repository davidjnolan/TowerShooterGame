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

bool UUserSettings::GetPlayTutorial() const
{
    return bPlayTutorial;
}

void UUserSettings::SetPlayTutorial(bool NewValue)
{
    bPlayTutorial = NewValue;
}

bool UUserSettings::GetShowFPS() const
{
    return bShowFPS;
}

void UUserSettings::SetShowFPS(bool NewValue)
{
    bShowFPS = NewValue;
}

float UUserSettings::GetVolumeMain() const
{
    return VolumeMain;
}

void UUserSettings::SetVolumeMain(float NewValue)
{
    VolumeMain = NewValue;
}

float UUserSettings::GetVolumeMusic() const
{
    return VolumeMusic;
}

void UUserSettings::SetVolumeMusic(float NewValue)
{
    VolumeMusic = NewValue;
}

float UUserSettings::GetVolumeFX() const
{
    return VolumeFX;
}

void UUserSettings::SetVolumeFX(float NewValue)
{
    VolumeFX = NewValue;
}