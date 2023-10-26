// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Async/Future.h"
#include "Async/Async.h"
#include "coacd.h"

struct CoACDParam {
	CoACD_Mesh Mesh;
	double threshold;
	int max_convex_hull;
	int preprocess_mode;
	int prep_resolution;
	int sample_resolution;
	int mcts_nodes;
	int mcts_iteration;
	int mcts_max_depth;
	bool pca;
	bool merge;
	unsigned int seed;
};

class COACDSRC_API FCoACDSrcModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	CoACD_MeshArray ExecProcessCoACD(CoACD_Mesh Mesh, double threshold = 0.05, int max_convex_hull = -1, int preprocess_mode = 0,
		int prep_resolution = 50, int sample_resolution = 2000, int mcts_nodes = 20, int mcts_iteration = 150,
		int mcts_max_depth = 3, bool pca = false, bool merge = true, unsigned int seed = 0);

	TFuture<CoACD_MeshArray> ExecProcessCoACD_Async(CoACD_Mesh Mesh, double threshold = 0.05, int max_convex_hull = -1, int preprocess_mode = 0,
		int prep_resolution = 50, int sample_resolution = 2000, int mcts_nodes = 20, int mcts_iteration = 150,
		int mcts_max_depth = 3, bool pca = false, bool merge = true, unsigned int seed = 0);

	static FString GetCurrentStatus();

private:
	/** Handle to the test dll we will load */
	void*	CoACDLibraryHandle;
};
