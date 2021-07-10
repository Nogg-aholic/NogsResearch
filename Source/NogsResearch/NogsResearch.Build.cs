// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using Tools.DotNETCommon;

public class NogsResearch : ModuleRules
{
	public NogsResearch(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		var factoryGamePchPath = new DirectoryReference(Path.Combine(Target.ProjectFile.Directory.ToString(), "Source", "FactoryGame", "Public", "FactoryGame.h"));
        PrivatePCHHeaderFile = factoryGamePchPath.MakeRelativeTo(new DirectoryReference(ModuleDirectory));

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "CoreUObject",
	            "Engine",
	            "InputCore","UMG",
				"FactoryGame", "SML"
				// ... add other public dependencies that you statically link with here ...
			}
			);
	}
}
