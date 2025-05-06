// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CrimItemSystem : ModuleRules
{
	public CrimItemSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"NetCore",
				"DeveloperSettings",
				"GameplayTags",
				"UMG",
				"ModelViewViewModel",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine", 
				"AIModule",
			}
			);
		
		SetupIrisSupport(Target);
	}
}
