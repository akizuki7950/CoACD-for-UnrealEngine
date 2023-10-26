// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "IDetailRootObjectCustomization.h"
#include "IDetailCustomization.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "SCoACDWidgetMenu.generated.h"

/**
 * 
 */

class COACDEDITOR_API FRootObjectCustomization : public IDetailRootObjectCustomization
{
public:
	virtual TSharedPtr<SWidget> CustomizeObjectHeader(const UObject* InRootObject) override
	{
		return SNullWidget::NullWidget;
	}
	
	virtual bool IsObjectVisible(const UObject* InRootObject) const override { return true; }
	virtual bool ShouldDisplayHeader(const UObject* InRootObject) const override { return false;  }
};


UCLASS()
class COACDEDITOR_API UCoACDEditorDetailsView : public UObject
{
	GENERATED_BODY()

public:

	UCoACDEditorDetailsView() {
		return;
	};

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters")
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters", meta = ( ClampMin = "0.01", ClampMax = "1.0"))
	double threshold = 0.05;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters")
	int max_convex_hull = -1;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters")
	int preprocess_mode = 0;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters", meta = (ClampMin = "10", ClampMax = "100"))
	int prep_resolution = 50;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters", meta = (ClampMin = "1000", ClampMax = "10000"))
	int sample_resolution = 2000;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters", meta = (ClampMin = "10", ClampMax = "40"))
	int mcts_nodes = 20;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters", meta = (ClampMin = "60", ClampMax = "2000"))
	int mcts_iteration = 150;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters", meta = (ClampMin = "2", ClampMax = "7"))
	int mcts_max_depth = 3;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters")
	bool PCA = false;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters")
	bool merge = true;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters")
	unsigned int seed = 0;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters|System")
	bool bAsyncProcess = true;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters|Custom Input")
	bool bUseCustomInputMesh = false;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters|Custom Input", meta = (EditCondition = "bUseCustomInputMesh"))
	TSoftObjectPtr<UStaticMesh> CustomInputMesh;

	UPROPERTY(EditDefaultsOnly, Category = "CoACD Parameters|Offset")
	FVector Offset = FVector::ZeroVector;

};

class COACDEDITOR_API SCoACDWidgetMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCoACDWidgetMenu)
	{}
	SLATE_END_ARGS()


	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FReply OnApplyDecomp();
	FReply OnApplyOffset();

private:
	TSharedPtr<IDetailsView> ConfigPanel;
	
};
