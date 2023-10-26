// Fill out your copyright notice in the Description page of Project Settings.


#include "CoACDEditorTools.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"
#include "StaticMeshResources.h"
#include "RenderingThread.h"
#include "vector"
#include "coacd.h"
#include "CoACDSrc.h"

bool ProcessCoACD(UStaticMesh* UEMesh, int32 LODIndex, double threshold, int max_convex_hull, int preprocess_mode,
	int prep_resolution, int sample_resolution, int mcts_nodes, int mcts_iteration,
	int mcts_max_depth, bool pca, bool merge, unsigned int seed, UStaticMesh* CUEMesh) {
	
	UStaticMesh* MeshToProcess, * MeshToApply;
	MeshToProcess = MeshToApply = UEMesh;
	if (CUEMesh->IsValidLowLevel()) {
		MeshToProcess = CUEMesh;
	}

	CoACD_Mesh ACDMesh;

	FStaticMeshLODResources& LODModel = MeshToProcess->GetRenderData()->LODResources[0];
	int32 NumVerts = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices();
	ACDMesh.vertices_count = NumVerts;
	ACDMesh.vertices_ptr = new double[NumVerts * 3];
	for (int32 i = 0; i < NumVerts; i++)
	{
		const FVector3f& Vert = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
		ACDMesh.vertices_ptr[i * 3 + 0] = Vert.X;
		ACDMesh.vertices_ptr[i * 3 + 1] = Vert.Y;
		ACDMesh.vertices_ptr[i * 3 + 2] = Vert.Z;
	}

	// Grab all indices
	TArray<uint32> AllIndices;
	LODModel.IndexBuffer.GetCopy(AllIndices);

	// Only copy indices that have collision enabled
	TArray<uint32> CollidingIndices;
	for (const FStaticMeshSection& Section : LODModel.Sections)
	{
		if (Section.bEnableCollision)
		{
			for (uint32 IndexIdx = Section.FirstIndex; IndexIdx < Section.FirstIndex + (Section.NumTriangles * 3); IndexIdx++)
			{
				CollidingIndices.Add(AllIndices[IndexIdx]);
			}
		}
	}

	ACDMesh.triangles_count = CollidingIndices.Num() / 3;
	ACDMesh.triangles_ptr = new int[CollidingIndices.Num()];
	for (int i = 0; i < CollidingIndices.Num(); i++) {
		ACDMesh.triangles_ptr[i] = CollidingIndices[i];
	}

	FlushRenderingCommands();
		
	UBodySetup* bs = MeshToApply->GetBodySetup();
	if (bs)
	{
		bs->RemoveSimpleCollision();
	}
	else
	{
		// Otherwise, create one here.
		MeshToApply->CreateBodySetup();
		bs = MeshToApply->GetBodySetup();
	}
	//CoACD_setLogLevel("debug");
	UE_LOG(LogTemp, Warning, TEXT("CoACD: Mesh vertices: %d, triangles: %d"), ACDMesh.vertices_count, ACDMesh.triangles_count);
	int total_triangles = 0;

	if (ACDMesh.vertices_count >= 3 && ACDMesh.triangles_count >= 1)
	{
		FCoACDSrcModule CoACDModule = FModuleManager::GetModuleChecked<FCoACDSrcModule>("CoACDSrc");
		CoACD_MeshArray DecomposedMeshes = CoACDModule.ExecProcessCoACD(
			ACDMesh, 
			threshold,
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

		if (DecomposedMeshes.meshes_count <= 0) {
			UE_LOG(LogTemp, Error, TEXT("CoACD: Decomposition failed"));
			return false;
		}

		UE_LOG(LogTemp, Warning, TEXT("CoACD: Mesh decomposed, num hulls: %d"), DecomposedMeshes.meshes_count);
		for (int i = 0; i < DecomposedMeshes.meshes_count; i++) {

			CoACD_Mesh Mesh = DecomposedMeshes.meshes_ptr[i];

			FKConvexElem ConvexElem;

			UE_LOG(LogTemp, Warning, TEXT("CoACD: %d-th Mesh, num vertices: %d, num triangles: %d"), i, Mesh.vertices_count, Mesh.triangles_count);
			total_triangles += Mesh.triangles_count;

			for (int j = 0; j < Mesh.vertices_count; j++) {
				FVector Vert;
				Vert.X = Mesh.vertices_ptr[j * 3 + 0];
				Vert.Y = Mesh.vertices_ptr[j * 3 + 1];
				Vert.Z = Mesh.vertices_ptr[j * 3 + 2];

				ConvexElem.VertexData.Add(Vert);
			}
			for (int j = 0; j < Mesh.triangles_count; j++) {
				ConvexElem.IndexData.Add(Mesh.triangles_ptr[j * 3 + 0]);
				ConvexElem.IndexData.Add(Mesh.triangles_ptr[j * 3 + 1]);
				ConvexElem.IndexData.Add(Mesh.triangles_ptr[j * 3 + 2]);

				//UE_LOG(LogTemp, Warning, TEXT("%d %d %d"), Mesh.triangles_ptr[j * 3 + 0], Mesh.triangles_ptr[j * 3 + 1], Mesh.triangles_ptr[j * 3 + 2]);
			}

			ConvexElem.UpdateElemBox();
			bs->AggGeom.ConvexElems.Add(ConvexElem);
		}

		bs->InvalidatePhysicsData();

	}
	else {
		UE_LOG(LogTemp, Error, TEXT("CoACD: Not enough vertices"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("CoACD: Done with total %d triangles"), total_triangles);

	MeshToApply->MarkPackageDirty();

	// Update screen.
	//GetStaticMeshViewport()->RefreshViewport();
	
	MeshToApply->bCustomizedCollision = true;
	return true;
}

bool IProcessCoACDAsync::IsComplete() {
	return DecomposedMeshAsync.IsValid() && DecomposedMeshAsync.IsReady();
}

void IProcessCoACDAsync::Release() {
	DecomposedMeshAsync.Reset();
	delete this;
}

bool IProcessCoACDAsync::ProcessCoACD_Async(UStaticMesh* UEMesh, int32 LODIndex, double threshold, int max_convex_hull, int preprocess_mode,
	int prep_resolution, int sample_resolution, int mcts_nodes, int mcts_iteration,
	int mcts_max_depth, bool pca, bool merge, unsigned int seed, UStaticMesh* CUEMesh) 
{
	
	MeshToProcess = MeshToApply = UEMesh;
	if (CUEMesh->IsValidLowLevel()) {
		MeshToProcess = CUEMesh;
	}

	CoACD_Mesh ACDMesh;

	FStaticMeshLODResources& LODModel = MeshToProcess->GetRenderData()->LODResources[0];
	int32 NumVerts = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices();
	ACDMesh.vertices_count = NumVerts;
	ACDMesh.vertices_ptr = new double[NumVerts * 3];
	for (int32 i = 0; i < NumVerts; i++)
	{
		const FVector3f& Vert = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
		ACDMesh.vertices_ptr[i * 3 + 0] = Vert.X;
		ACDMesh.vertices_ptr[i * 3 + 1] = Vert.Y;
		ACDMesh.vertices_ptr[i * 3 + 2] = Vert.Z;
	}

	// Grab all indices
	TArray<uint32> AllIndices;
	LODModel.IndexBuffer.GetCopy(AllIndices);

	// Only copy indices that have collision enabled
	TArray<uint32> CollidingIndices;
	for (const FStaticMeshSection& Section : LODModel.Sections)
	{
		if (Section.bEnableCollision)
		{
			for (uint32 IndexIdx = Section.FirstIndex; IndexIdx < Section.FirstIndex + (Section.NumTriangles * 3); IndexIdx++)
			{
				CollidingIndices.Add(AllIndices[IndexIdx]);
			}
		}
	}

	ACDMesh.triangles_count = CollidingIndices.Num() / 3;
	ACDMesh.triangles_ptr = new int[CollidingIndices.Num()];
	for (int i = 0; i < CollidingIndices.Num(); i++) {
		ACDMesh.triangles_ptr[i] = CollidingIndices[i];
	}

	FlushRenderingCommands();

	UBodySetup* bs = MeshToApply->GetBodySetup();
	if (bs)
	{
		bs->RemoveSimpleCollision();
	}
	else
	{
		// Otherwise, create one here.
		MeshToApply->CreateBodySetup();
		bs = MeshToApply->GetBodySetup();
	}
	//CoACD_setLogLevel("debug");
	UE_LOG(LogTemp, Warning, TEXT("CoACD: Mesh vertices: %d, triangles: %d"), ACDMesh.vertices_count, ACDMesh.triangles_count);
	int total_triangles = 0;

	if (ACDMesh.vertices_count >= 3 && ACDMesh.triangles_count >= 1)
	{
		FCoACDSrcModule CoACDModule = FModuleManager::GetModuleChecked<FCoACDSrcModule>("CoACDSrc");
		DecomposedMeshAsync = CoACDModule.ExecProcessCoACD_Async(
			ACDMesh,
			threshold,
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
	}
	return true;
}

bool IProcessCoACDAsync::RestoreResult() {

	CoACD_MeshArray DecomposedMeshes = DecomposedMeshAsync.Get();
	UBodySetup* bs = MeshToApply->GetBodySetup();
	if (!bs) {
		MeshToApply->CreateBodySetup();
		bs = MeshToApply->GetBodySetup();
	}

	if (DecomposedMeshes.meshes_count <= 0) {
		UE_LOG(LogTemp, Error, TEXT("CoACD: Decomposition failed"));
		return false;
	}

	int total_triangles = 0;
	UE_LOG(LogTemp, Warning, TEXT("CoACD: Mesh decomposed, num hulls: %d"), DecomposedMeshes.meshes_count);
	for (int i = 0; i < DecomposedMeshes.meshes_count; i++) {

		CoACD_Mesh Mesh = DecomposedMeshes.meshes_ptr[i];

		FKConvexElem ConvexElem;

		UE_LOG(LogTemp, Warning, TEXT("CoACD: %d-th Mesh, num vertices: %d, num triangles: %d"), i, Mesh.vertices_count, Mesh.triangles_count);
		total_triangles += Mesh.triangles_count;

		for (int j = 0; j < Mesh.vertices_count; j++) {
			FVector Vert;
			Vert.X = Mesh.vertices_ptr[j * 3 + 0];
			Vert.Y = Mesh.vertices_ptr[j * 3 + 1];
			Vert.Z = Mesh.vertices_ptr[j * 3 + 2];

			ConvexElem.VertexData.Add(Vert);
		}
		for (int j = 0; j < Mesh.triangles_count; j++) {
			ConvexElem.IndexData.Add(Mesh.triangles_ptr[j * 3 + 0]);
			ConvexElem.IndexData.Add(Mesh.triangles_ptr[j * 3 + 1]);
			ConvexElem.IndexData.Add(Mesh.triangles_ptr[j * 3 + 2]);

			//UE_LOG(LogTemp, Warning, TEXT("%d %d %d"), Mesh.triangles_ptr[j * 3 + 0], Mesh.triangles_ptr[j * 3 + 1], Mesh.triangles_ptr[j * 3 + 2]);
		}

		ConvexElem.UpdateElemBox();
		bs->AggGeom.ConvexElems.Add(ConvexElem);
	}

	bs->InvalidatePhysicsData();

	UE_LOG(LogTemp, Warning, TEXT("CoACD: Done with total %d triangles"), total_triangles);

	MeshToApply->MarkPackageDirty();

	// Update screen.
	//GetStaticMeshViewport()->RefreshViewport();

	MeshToApply->bCustomizedCollision = true;

	return true;
}

FString IProcessCoACDAsync::GetCurrentStatus() {
	return FCoACDSrcModule::GetCurrentStatus();
}
