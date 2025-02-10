#include "CyberGafferProjectSettings.h"
#include "CyberGafferLog.h"
#include "CyberGafferSettingsVersion.h"

#include "CyberGafferWindowContent.h"

TOptional<FCyberGafferSceneSettings*> UCyberGafferProjectSettings::GetSettingsForScene(const FString& sceneName) {
	FCyberGafferSceneSettings* sceneSettings = ScenesSettings.Find(sceneName);
	if (sceneSettings == nullptr) {
		return TOptional<FCyberGafferSceneSettings*>();
	}

	return TOptional<FCyberGafferSceneSettings*>(sceneSettings);
}

TOptional<FCyberGafferSceneSettings*> UCyberGafferProjectSettings::GetSettingsForCurrentScene() {
	const UWorld* world = GEditor->GetEditorWorldContext().World();
	if (!world) {
		return TOptional<FCyberGafferSceneSettings*>();
	}
	
	const FString sceneName = world->GetMapName();
	if (sceneName.IsEmpty()) {
		return TOptional<FCyberGafferSceneSettings*>();
	}
	
	const FString sceneFileName =  FPaths::GetBaseFilename(sceneName);
	if (sceneFileName.IsEmpty()) {
		return TOptional<FCyberGafferSceneSettings*>();
	}

	return GetSettingsForScene(sceneFileName);
}

void UCyberGafferProjectSettings::Serialize(FArchive& archive) {
	Super::Serialize(archive);
	archive.UsingCustomVersion(FCyberGafferSettingsVersion::GUID);
}
