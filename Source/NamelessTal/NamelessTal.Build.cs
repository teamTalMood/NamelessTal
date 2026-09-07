// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NamelessTal : ModuleRules
{
	public NamelessTal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"NavigationSystem", "AIModule", "GameplayTasks",
            "MotionWarping"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"NamelessTal",
			"NamelessTal/Variant_Platforming",
			"NamelessTal/Variant_Platforming/Animation",
			"NamelessTal/Variant_Combat",
			"NamelessTal/Variant_Combat/AI",
			"NamelessTal/Variant_Combat/Animation",
			"NamelessTal/Variant_Combat/Gameplay",
			"NamelessTal/Variant_Combat/Interfaces",
			"NamelessTal/Variant_Combat/UI",
			"NamelessTal/Variant_SideScrolling",
			"NamelessTal/Variant_SideScrolling/AI",
			"NamelessTal/Variant_SideScrolling/Gameplay",
			"NamelessTal/Variant_SideScrolling/Interfaces",
			"NamelessTal/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
