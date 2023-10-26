// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class CoACDSrcLibrary : ModuleRules
{
	public CoACDSrcLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/Public");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			//PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "ExampleLibrary.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "_coacd.lib"));
			
			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("lib_coacd.dll");

			// Ensure that the DLL is staged along with the executable
			//RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/CoACDSrcLibrary/Win64/ExampleLibrary.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/CoACDSrcLibrary/Win64/lib_coacd.dll");
		}
		/*
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/CoACDSrcLibrary/Mac/Release/libExampleLibrary.dylib");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "CoACDSrcLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
		}
		*/
	}
}
