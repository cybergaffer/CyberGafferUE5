#include "CyberGafferProjectSettings.h"
#include "CyberGafferLog.h"
#include "CyberGafferSettingsVersion.h"

#include "CyberGafferWindowContent.h"

// void UCyberGafferWindowSceneControlsUI::SetWindowContent(TWeakPtr<SCyberGafferWindowContent> content) {
// 	_windowContent = content;
// }

// float UCyberGafferWindowSceneControlsUI::GetExposureCompensation() const {
// 	CYBERGAFFER_LOG(Log, TEXT("UCyberGafferWindowSceneControlsUI::GetExposureCompensation"))
// 	
// 	if (_windowContent == nullptr) {
// 		return 0.0f;
// 	}
//
// 	auto windowContentPinned = _windowContent.Pin();
// 	if (windowContentPinned == nullptr) {
// 		return 0.0f;
// 	}
//
// 	auto currnetValue = windowContentPinned->GetExposureCompensation();
// 	if (currnetValue.IsSet()) {
// 		return currnetValue.GetValue();
// 	}
//
// 	return 0.0f;
// }
//
// void UCyberGafferWindowSceneControlsUI::SetExposureCompensation(const float exposureCompensation) {
// 	if (_windowContent == nullptr) {
// 		return;
// 	}
//
// 	auto windowContentPinned = _windowContent.Pin();
// 	if (windowContentPinned == nullptr) {
// 		return;
// 	}
//
// 	windowContentPinned->OnExposureCompensationValueChanged(exposureCompensation);
// }

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

// FString UCyberGafferProjectSettings::ToJson() {
// 	FJsonSerializable
// }
