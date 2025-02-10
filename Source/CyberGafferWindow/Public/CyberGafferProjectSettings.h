#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstance.h"
#include "Math/Color.h"

#include "CyberGafferSceneCapture.h"

#include "CyberGafferProjectSettings.generated.h"

USTRUCT()
struct CYBERGAFFERWINDOW_API FCyberGafferAutomationSettings {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Automation", meta = (DisplayPriority = 1))
	bool AddCyberGafferSceneCapture;

	UPROPERTY(EditAnywhere, Category = "Automation", meta = (DisplayPriority = 1))
	bool ConfigurePostProcessVolume;

	UPROPERTY(EditAnywhere, Category = "Automation", meta = (DisplayPriority = 1))
	bool CreateNewPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Automation", meta = (DisplayPriority = 1))
	bool CreateOCIOConfig;

	FCyberGafferAutomationSettings() :
		AddCyberGafferSceneCapture(true),
		ConfigurePostProcessVolume(true),
		CreateNewPostProcessMaterial(true),
		CreateOCIOConfig(true) {}
	
	FCyberGafferAutomationSettings(
		const bool addCyberGafferSceneCapture,
		const bool configurePostProcessVolume,
		const bool createNewPostProcessMaterial,
		const bool createOCIOConfig) :
	
		AddCyberGafferSceneCapture(addCyberGafferSceneCapture),
		ConfigurePostProcessVolume(configurePostProcessVolume),
		CreateNewPostProcessMaterial(createNewPostProcessMaterial),
		CreateOCIOConfig(createOCIOConfig) {}
};

// Temporary remove shaders include
/*
USTRUCT()
struct CYBERGAFFERWINDOW_API FCyberGafferShadersConfig {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Shaders")
	FDirectoryPath ShadersIncludePath;
	
	UPROPERTY(EditAnywhere, Category = "Shaders", SaveGame)
	bool AutoRecompileShadersIfIncludeChange;

	FCyberGafferShadersConfig() : ShadersIncludePath(TEXT("")), AutoRecompileShadersIfIncludeChange(false) {}

	FCyberGafferShadersConfig(const FString& shadersIncludePath, const bool autoRecompileShadersIfIncludeChange) :
		ShadersIncludePath(shadersIncludePath),
		AutoRecompileShadersIfIncludeChange(autoRecompileShadersIfIncludeChange) { }
};*/

// If you change this struct, update FCyberGafferSettingsVersion
USTRUCT()
struct CYBERGAFFERWINDOW_API FCyberGafferSceneSettings {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* PostProcessMaterial;

	FCyberGafferSceneSettings() : PostProcessMaterial(nullptr) { }

	FCyberGafferSceneSettings(UMaterialInstance* linearPostProcessMaterial) : PostProcessMaterial(linearPostProcessMaterial) { }
};

// If you change this struct, update FCyberGafferSettingsVersion
UCLASS()
class CYBERGAFFERWINDOW_API UCyberGafferProjectSettings : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CyberGaffer")
	TMap<FString, FCyberGafferSceneSettings> ScenesSettings;

	// UPROPERTY(EditAnywhere, Category = "CyberGaffer")
	// FCyberGafferShadersConfig ShadersConfig;

	TOptional<FCyberGafferSceneSettings*> GetSettingsForScene(const FString& sceneName);
	TOptional<FCyberGafferSceneSettings*> GetSettingsForCurrentScene();

	virtual void Serialize(FArchive& archive) override;
};
