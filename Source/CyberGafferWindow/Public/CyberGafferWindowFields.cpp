#include "CyberGafferWindowFields.h"

TOptional<FCyberGafferWindowSceneSettings*> UCyberGafferWindowSettings::GetSettingsForScene(const FString& sceneName) {
	auto* sceneSettings = ScenesSettings.Find(sceneName);
	if (sceneSettings == nullptr) {
		return TOptional<FCyberGafferWindowSceneSettings*>();
	}

	return TOptional<FCyberGafferWindowSceneSettings*>(sceneSettings);
}
