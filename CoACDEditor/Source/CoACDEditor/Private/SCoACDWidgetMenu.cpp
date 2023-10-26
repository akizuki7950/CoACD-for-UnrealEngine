// Fill out your copyright notice in the Description page of Project Settings.


#include "SCoACDWidgetMenu.h"
#include "UObject/ReflectedTypeAccessors.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "SlateOptMacros.h"
#include "CoACDEditorTools.h"
#include "Widgets/Layout/SScrollBox.h"
#include "CoACDNotification.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCoACDWidgetMenu::Construct(const FArguments& InArgs)
{
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	ConfigPanel = PropertyModule.CreateDetailView(DetailsViewArgs);
	ConfigPanel->SetRootObjectCustomizationInstance(MakeShareable(new FRootObjectCustomization));
	ConfigPanel->SetObject(UCoACDEditorDetailsView::StaticClass()->GetDefaultObject(true), true);

	ChildSlot 
	[
		SNew(SScrollBox)
			+SScrollBox::Slot().Padding(10, 5)
			[
				SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(5, 5).AutoHeight()
					[
						ConfigPanel.ToSharedRef()
					]
					+SVerticalBox::Slot().Padding(5, 5).AutoHeight()
					[
						SNew(SButton)
							.Text(FText::FromString("Apply Convex Decomposition"))
							.VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateSP(this, &SCoACDWidgetMenu::OnApplyDecomp))
					]
					+ SVerticalBox::Slot().Padding(5, 5).AutoHeight()
					[
						SNew(SButton)
							.Text(FText::FromString("Add Offset to Colliders"))
							.VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateSP(this, &SCoACDWidgetMenu::OnApplyOffset))
					]
			]
	];
}

FReply SCoACDWidgetMenu::OnApplyDecomp() {
	UE_LOG(LogTemp, Warning, TEXT("CoACD: Apply Clicked"));

	auto CoACDObject = static_cast<UCoACDEditorDetailsView*>(UCoACDEditorDetailsView::StaticClass()->GetDefaultObject(true));
	double threshold = CoACDObject->threshold;
	int max_convex_hull = CoACDObject->max_convex_hull;
	int preprocess_mode = CoACDObject->preprocess_mode;
	int prep_resolution = CoACDObject->prep_resolution;
	int sample_resolution = CoACDObject->sample_resolution;
	int mcts_nodes = CoACDObject->mcts_nodes;
	int mcts_iteration = CoACDObject->mcts_iteration;
	int mcts_max_depth = CoACDObject->mcts_max_depth;
	bool pca = CoACDObject->PCA;
	bool merge = CoACDObject->merge;
	unsigned int seed = CoACDObject->seed;
	bool bUseCustomInput = CoACDObject->bUseCustomInputMesh;
	bool bAsyncProcess = CoACDObject->bAsyncProcess;

	UStaticMesh* SM = CoACDObject->StaticMesh.Get();
	if (!SM->IsValidLowLevel()) {
		SM = CoACDObject->StaticMesh.LoadSynchronous();
		if (!SM->IsValidLowLevel()) {
			return FReply::Handled();
		}
	}

	UStaticMesh* CSM = nullptr;
	if (bUseCustomInput) {
		CSM = CoACDObject->CustomInputMesh.Get();
		if (!CSM->IsValidLowLevel()) {
			CSM = CoACDObject->StaticMesh.LoadSynchronous();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("CoACD: Apply to mesh %s"), *(SM->GetFName().ToString()));
	bool bSuccess = false;
	if (bAsyncProcess) {
		if (ProcessCoACDAsync) {
			ProcessCoACDAsync->Release();
		}
		ProcessCoACDAsync = new IProcessCoACDAsync;
		bSuccess = ProcessCoACDAsync->ProcessCoACD_Async(
			SM,
			0,
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
			seed, // seed
			CSM
		);
	}
	else {
		 bSuccess = ProcessCoACD(
			 SM,
			 0,
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
			seed, // seed
			CSM
		);
	}
	if (!bSuccess) {
		UE_LOG(LogTemp, Error, TEXT("CoACD: Appy to mesh failed"));
	}

	return FReply::Handled();
}

FReply SCoACDWidgetMenu::OnApplyOffset() {

	auto CoACDObject = static_cast<UCoACDEditorDetailsView*>(UCoACDEditorDetailsView::StaticClass()->GetDefaultObject(true));
	UStaticMesh* SM = CoACDObject->StaticMesh.Get();
	if (!SM->IsValidLowLevel()) {
		SM = CoACDObject->StaticMesh.LoadSynchronous();
		if (!SM->IsValidLowLevel()) {
			return FReply::Handled();
		}
	}
	UBodySetup* bs = SM->GetBodySetup();
	if (bs) {
		FVector Offset = CoACDObject->Offset;
		for (FKConvexElem& ConvexElem : bs->AggGeom.ConvexElems) {
			for (FVector& Vert : ConvexElem.VertexData) {
				Vert += Offset;
			}
			ConvexElem.UpdateElemBox();
		}
		bs->InvalidatePhysicsData();
		SM->MarkPackageDirty();
	}

	return FReply::Handled();
}

void SCoACDWidgetMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	if (ProcessCoACDAsync)
	{
		if (ProcessCoACDAsync->IsComplete())
		{
			ProcessCoACDAsync->RestoreResult();
			ProcessCoACDAsync->Release();
			ProcessCoACDAsync = nullptr;
			GCoACDNotificationState->IsActive = false;
		}
		else if (GCoACDNotificationState)
		{
			GCoACDNotificationState->IsActive = true;
			GCoACDNotificationState->Status = ProcessCoACDAsync->GetCurrentStatus();
		}
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
