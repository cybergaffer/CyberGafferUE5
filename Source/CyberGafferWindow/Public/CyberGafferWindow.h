#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Materials/MaterialInstance.h"

class FToolBarBuilder;
class FMenuBuilder;

class FCyberGafferWindowModule : public IModuleInterface {
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& spawnTabArgs);

private:
	TSharedPtr<class FUICommandList> _pluginCommands;

	TWeakPtr<UMaterialInstance> _postProcessMaterialInstance = nullptr;
	TWeakPtr<UMaterialInstance> _cameraPostProcessMaterialInstance = nullptr;
};
