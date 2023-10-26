// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CoACDEditorStyle.h"

class FCoACDEditorCommands : public TCommands<FCoACDEditorCommands>
{
public:

	FCoACDEditorCommands()
		: TCommands<FCoACDEditorCommands>(TEXT("CoACDEditor"), NSLOCTEXT("Contexts", "CoACDEditor", "CoACDEditor Plugin"), NAME_None, FCoACDEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};