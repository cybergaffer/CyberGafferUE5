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
	bool CreateNewLinearPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Automation", meta = (DisplayPriority = 1))
	bool CreateNewColorGradingPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Automation", meta = (DisplayPriority = 1))
	bool CreateOCIOConfig;

	FCyberGafferAutomationSettings() :
		AddCyberGafferSceneCapture(true),
		ConfigurePostProcessVolume(true),
		CreateNewLinearPostProcessMaterial(true),
		CreateNewColorGradingPostProcessMaterial(true),
		CreateOCIOConfig(true) {}
	
	FCyberGafferAutomationSettings(
		const bool addCyberGafferSceneCapture,
		const bool configurePostProcessVolume,
		const bool createNewLinearPostProcessMaterial,
		const bool createNewColorGradingPostProcessMaterial,
		const bool createOCIOConfig) :
	
		AddCyberGafferSceneCapture(addCyberGafferSceneCapture),
		ConfigurePostProcessVolume(configurePostProcessVolume),
		CreateNewLinearPostProcessMaterial(createNewLinearPostProcessMaterial),
		CreateNewColorGradingPostProcessMaterial(createNewColorGradingPostProcessMaterial),
		CreateOCIOConfig(createOCIOConfig) {}
};

// If you change this struct, update FCyberGafferSettingsVersion
USTRUCT()
struct CYBERGAFFERWINDOW_API FCyberGafferShadersConfig {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Shaders")
	FDirectoryPath ShadersIncludePath;

	// TODO: Do we really need that?
	// UPROPERTY(EditAnywhere, Category = "Shaders", SaveGame)
	// bool AutoRecompileShadersIfIncludeChange;

	FCyberGafferShadersConfig() : ShadersIncludePath(TEXT(""))/*, AutoRecompileShadersIfIncludeChange(false)*/ {}

	FCyberGafferShadersConfig(const FString& shadersIncludePath/*, const bool autoRecompileShadersIfIncludeChange*/) :
		ShadersIncludePath(shadersIncludePath)/*,
		AutoRecompileShadersIfIncludeChange(autoRecompileShadersIfIncludeChange)*/ { }
};

// If you change this struct, update FCyberGafferSettingsVersion
USTRUCT()
struct CYBERGAFFERWINDOW_API FCyberGafferSceneSettings {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* LinearPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* ColorGradingPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Scene Components")
	APostProcessVolume* PostProcessVolume;

	UPROPERTY(EditAnywhere, Category = "Scene Components")
	ACyberGafferSceneCapture* CyberGafferSceneCapture;

	FCyberGafferSceneSettings() :
		LinearPostProcessMaterial(nullptr),
		ColorGradingPostProcessMaterial(nullptr),
		PostProcessVolume(nullptr),
		CyberGafferSceneCapture(nullptr) { }

	FCyberGafferSceneSettings(
		UMaterialInstance* linearPostProcessMaterial,
		UMaterialInstance* colorGradingPostProcessMaterial,
		APostProcessVolume* postProcessVolume,
		ACyberGafferSceneCapture* cyberGafferSceneCapture) :

		LinearPostProcessMaterial(linearPostProcessMaterial),
		ColorGradingPostProcessMaterial(colorGradingPostProcessMaterial),
		PostProcessVolume(postProcessVolume),
		CyberGafferSceneCapture(cyberGafferSceneCapture) { }
};

// If you change this struct, update FCyberGafferSettingsVersion
UCLASS()
class CYBERGAFFERWINDOW_API UCyberGafferProjectSettings : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CyberGaffer")
	TMap<FString, FCyberGafferSceneSettings> ScenesSettings;

	UPROPERTY(EditAnywhere, Category = "CyberGaffer")
	FCyberGafferShadersConfig ShadersConfig;

	TOptional<FCyberGafferSceneSettings*> GetSettingsForScene(const FString& sceneName);
	TOptional<FCyberGafferSceneSettings*> GetSettingsForCurrentScene();

	virtual void Serialize(FArchive& archive) override;
};
