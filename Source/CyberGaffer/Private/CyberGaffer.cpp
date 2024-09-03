#include "CyberGaffer.h"

#include "Interfaces/IPluginManager.h"
#include "Runtime/RenderCore/Public/ShaderCore.h"

#include "CyberGafferLog.h"

#define LOCTEXT_NAMESPACE "FCyberGafferModule"

void FCyberGafferModule::StartupModule() {
	const auto shadersDir = FPaths::Combine(GetPluginDirectory(), TEXT("Shaders"));
	if (!FPaths::DirectoryExists(shadersDir)) {
		CYBERGAFFER_LOG(Error, TEXT("FCyberGafferModule::StartupModule: Failed to find plugin's shaders directory at path %s"), *shadersDir);
		return;
	}

	AddShaderSourceDirectoryMapping(TEXT("/CyberGaffer"), shadersDir);

	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferModule::StartupModule: Module initialized"))
}

void FCyberGafferModule::ShutdownModule() {}

FString FCyberGafferModule::GetPluginName() {
	return { _pluginName };
}

FString FCyberGafferModule::GetPluginVersion() {
	return IPluginManager::Get().FindPlugin(GetPluginName())->GetDescriptor().VersionName;
}

FString FCyberGafferModule::GetPluginDirectory() {
	return IPluginManager::Get().FindPlugin(GetPluginName())->GetBaseDir();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCyberGafferModule, CyberGaffer)