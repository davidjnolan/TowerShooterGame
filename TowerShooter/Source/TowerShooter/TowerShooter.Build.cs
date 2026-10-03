// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TowerShooter : ModuleRules
{
	public TowerShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"AIModule",
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"InputCore",
			"GameplayTags",
			"Json",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"RenderCore",
			"RHI"
		});

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate",
			"SlateCore"
		});
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
