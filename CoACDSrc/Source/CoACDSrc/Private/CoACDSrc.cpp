// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoACDSrc.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "coacd.h"

#define LOCTEXT_NAMESPACE "FCoACDSrcModule"

void FCoACDSrcModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("CoACDSrc")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/CoACDSrcLibrary/Win64/lib_coacd.dll"));
#elif PLATFORM_MAC
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/CoACDSrcLibrary/Mac/Release/libExampleLibrary.dylib"));
#elif PLATFORM_LINUX
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/CoACDSrcLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
#endif // PLATFORM_WINDOWS

	CoACDLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (CoACDLibraryHandle)
	{
		// Call the test function in the third party library that opens a message box
		//ExampleLibraryFunction();
		UE_LOG(LogTemp, Warning, TEXT("COACD Loaded"));
		CoACD_setLogLevel("debug");
	}
	else
	{
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}
}

void FCoACDSrcModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(CoACDLibraryHandle);
	CoACDLibraryHandle = nullptr;
}

CoACD_MeshArray FCoACDSrcModule::ExecProcessCoACD(CoACD_Mesh Mesh, double threshold, int max_convex_hull, int preprocess_mode,
	int prep_resolution, int sample_resolution, int mcts_nodes, int mcts_iteration,
	int mcts_max_depth, bool pca, bool merge, unsigned int seed) {

	CoACD_MeshArray ret;
	ret.meshes_count = 0;
	ret.meshes_ptr = NULL;

	if (!CoACDLibraryHandle)return ret;
	
	ret = CoACD_run(
		Mesh,
		threshold, // threshold
		max_convex_hull, // max_convex_hull
		preprocess_mode, // preprocess mode, 1 = on, 2 = off, else auto
		prep_resolution, // preprocess resolution
		sample_resolution, // sample resolution
		mcts_nodes, // mcts_nodes
		mcts_iteration, // mcts_iteration
		mcts_max_depth, // mcts_max_depth
		pca, // pca
		merge, // merge
		seed // seed
	);
	
	return ret;
}

TFuture<CoACD_MeshArray> FCoACDSrcModule::ExecProcessCoACD_Async(CoACD_Mesh Mesh, double threshold, int max_convex_hull, int preprocess_mode,
	int prep_resolution, int sample_resolution, int mcts_nodes, int mcts_iteration,
	int mcts_max_depth, bool pca, bool merge, unsigned int seed ) {


	CoACDParam Param;
	Param.Mesh = Mesh;
	Param.threshold = threshold;
	Param.max_convex_hull = max_convex_hull;
	Param.preprocess_mode = preprocess_mode;
	Param.prep_resolution = prep_resolution;
	Param.sample_resolution = sample_resolution;
	Param.mcts_nodes = mcts_nodes;
	Param.mcts_iteration = mcts_iteration;
	Param.mcts_max_depth = mcts_max_depth;
	Param.pca = pca;
	Param.merge = merge;
	Param.seed = seed;

	return Async(EAsyncExecution::Thread, [Param]() {
			return CoACD_run(
				Param.Mesh,
				Param.threshold,
				Param.max_convex_hull,
				Param.preprocess_mode,
				Param.prep_resolution,
				Param.sample_resolution,
				Param.mcts_nodes,
				Param.mcts_iteration,
				Param.mcts_max_depth,
				Param.pca,
				Param.merge,
				Param.seed
			);
		});

}

FString FCoACDSrcModule::GetCurrentStatus() {
	return FString::Printf(TEXT("Iter %d --- Waiting Pool: %d, Processing %.1f %%"), CoACD_GetIteration(), CoACD_GetPoolSize(), CoACD_GetProgress());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCoACDSrcModule, CoACDSrc)
