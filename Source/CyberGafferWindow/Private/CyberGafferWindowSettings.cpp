#include "CyberGafferWindowSettings.h"

TOptional<FCyberGafferWindowSceneSettings*> UCyberGafferWindowSettings::GetSettingsForScene(const FString& sceneName) {
	FCyberGafferWindowSceneSettings* sceneSettings = ScenesSettings.Find(sceneName);
	if (sceneSettings == nullptr) {
		return TOptional<FCyberGafferWindowSceneSettings*>();
	}

	return TOptional<FCyberGafferWindowSceneSettings*>(sceneSettings);
}

TOptional<FCyberGafferWindowSceneSettings*> UCyberGafferWindowSettings::GetSettingsForCurrentScene() {
	const UWorld* world = GEditor->GetEditorWorldContext().World();
	if (!world) {
		return nullptr;
	}
	
	const FString sceneName = world->GetMapName();
	if (sceneName.IsEmpty()) {
		return nullptr;
	}
	
	const FString sceneFileName =  FPaths::GetBaseFilename(sceneName);
	if (sceneFileName.IsEmpty()) {
		return nullptr;
	}

	return GetSettingsForScene(sceneFileName);
}
