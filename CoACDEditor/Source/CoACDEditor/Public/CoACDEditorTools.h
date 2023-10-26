// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
/**
 * 
 */

struct CoACD_MeshArray;

COACDEDITOR_API bool ProcessCoACD(UStaticMesh* UEMesh, int32 LODIndex, double threshold = 0.05, int max_convex_hull = -1, int preprocess_mode = 0,
	int prep_resolution = 50, int sample_resolution = 2000, int mcts_nodes = 20, int mcts_iteration = 150,
    int mcts_max_depth = 3, bool pca = false, bool merge = true, unsigned int seed = 0, UStaticMesh* CUEMesh = nullptr);

class COACDEDITOR_API IProcessCoACDAsync {
public:
	bool IsComplete();
	void Release();
	bool RestoreResult();
	FString GetCurrentStatus();

	bool ProcessCoACD_Async(UStaticMesh* UEMesh, int32 LODIndex, double threshold = 0.05, int max_convex_hull = -1, int preprocess_mode = 0,
		int prep_resolution = 50, int sample_resolution = 2000, int mcts_nodes = 20, int mcts_iteration = 150,
		int mcts_max_depth = 3, bool pca = false, bool merge = true, unsigned int seed = 0, UStaticMesh* CUEMesh = nullptr);
private:
	TFuture<CoACD_MeshArray> DecomposedMeshAsync;
	UStaticMesh* MeshToProcess;
	UStaticMesh* MeshToApply;
};
IProcessCoACDAsync* ProcessCoACDAsync = nullptr;