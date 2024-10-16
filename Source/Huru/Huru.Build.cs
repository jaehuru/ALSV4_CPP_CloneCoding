// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Huru : ModuleRules
{
	public Huru(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange
			(new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"EnhancedInput",
					"NavigationSystem",
					"AIModule",
					"GameplayTasks",
					"PhysicsCore",
					"Niagara"
				}
			);
		
		PrivateDependencyModuleNames.AddRange
			(new string[]
				{
					"Slate",
					"SlateCore"
				}
			);
	}
}
