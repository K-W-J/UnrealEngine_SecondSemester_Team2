// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SecondSemester_Team : ModuleRules
{
	public SecondSemester_Team(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"Niagara",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"SecondSemester_Team",
			"SecondSemester_Team/CSH",
			"SecondSemester_Team/Variant_Horror",
			"SecondSemester_Team/Variant_Horror/UI",
			"SecondSemester_Team/Variant_Shooter",
			"SecondSemester_Team/Variant_Shooter/AI",
			"SecondSemester_Team/Variant_Shooter/UI",
			"SecondSemester_Team/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
