// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoACDEditor.h"
#include "CoACDEditorStyle.h"
#include "CoACDEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "SCoACDWidgetMenu.h"

static const FName CoACDEditorTabName("CoACDEditor");

#define LOCTEXT_NAMESPACE "FCoACDEditorModule"

void FCoACDEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FCoACDEditorStyle::Initialize();
	FCoACDEditorStyle::ReloadTextures();

	FCoACDEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCoACDEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FCoACDEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCoACDEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CoACDEditorTabName, FOnSpawnTab::CreateRaw(this, &FCoACDEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FCoACDEditorTabTitle", "CoACDEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FCoACDEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCoACDEditorStyle::Shutdown();

	FCoACDEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CoACDEditorTabName);
}

TSharedRef<SDockTab> FCoACDEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FCoACDEditorModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("CoACDEditor.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SCoACDWidgetMenu)
		];
}

void FCoACDEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(CoACDEditorTabName);
}

void FCoACDEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCoACDEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCoACDEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCoACDEditorModule, CoACDEditor)