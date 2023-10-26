// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoACDEditorCommands.h"

#define LOCTEXT_NAMESPACE "FCoACDEditorModule"

void FCoACDEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "CoACDEditor", "Bring up CoACDEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
