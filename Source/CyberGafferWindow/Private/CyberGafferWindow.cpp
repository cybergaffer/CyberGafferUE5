#include "CyberGafferWindow.h"
#include "CyberGafferWindowStyle.h"
#include "CyberGafferWindowCommands.h"
#include "CyberGafferWindowContent.h"

#include "LevelEditor.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"

static const FName CyberGafferWindowTabName("CyberGaffer");

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

void FCyberGafferWindowModule::StartupModule() {
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FCyberGafferWindowStyle::Initialize();
	FCyberGafferWindowStyle::ReloadTextures();

	FCyberGafferWindowCommands::Register();
	
	_pluginCommands = MakeShareable(new FUICommandList);

	_pluginCommands->MapAction(
		FCyberGafferWindowCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FCyberGafferWindowModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCyberGafferWindowModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CyberGafferWindowTabName, FOnSpawnTab::CreateRaw(this, &FCyberGafferWindowModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FCyberGafferWindowTabTitle", "CyberGafferWindow"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FCyberGafferWindowModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCyberGafferWindowStyle::Shutdown();

	FCyberGafferWindowCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CyberGafferWindowTabName);
}

TSharedRef<SDockTab> FCyberGafferWindowModule::OnSpawnPluginTab(const FSpawnTabArgs& spawnTabArgs) {
	TSharedRef<SDockTab> pluginTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab);

	TSharedRef<SCyberGafferWindowContent> tabContent = SNew(SCyberGafferWindowContent)
		._containingTab(pluginTab);

	pluginTab->SetContent(tabContent);

	return pluginTab;
}

void FCyberGafferWindowModule::PluginButtonClicked() {
	FGlobalTabmanager::Get()->TryInvokeTab(CyberGafferWindowTabName);
}

void FCyberGafferWindowModule::RegisterMenus() {
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCyberGafferWindowCommands::Get().OpenPluginWindow, _pluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCyberGafferWindowCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(_pluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCyberGafferWindowModule, CyberGafferWindow)