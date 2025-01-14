#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstance.h"
#include "Math/Color.h"

#include "CyberGafferWindowSettings.generated.h"

USTRUCT()
struct FCyberGafferWindowSceneSettings {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "CyberGaffer|Materials")
	FString LinearPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "CyberGaffer|Materials")
	FString ColorGradingPostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "CyberGaffer")
	FString PostProcessVolumePath;

	UPROPERTY(EditAnywhere, Category = "CyberGaffer")
	FString CyberGafferSceneCapturePath;


	FCyberGafferWindowSceneSettings() :
		LinearPostProcessMaterial(""),
		ColorGradingPostProcessMaterial(""),
		PostProcessVolumePath(""),
		CyberGafferSceneCapturePath("") {}
	
	FCyberGafferWindowSceneSettings(
		const FString& linearPostProcessMaterial,
		const FString& colorGradingPostProcessMaterial,
		const FString& postProcessVolumePath,
		const FString& cyberGafferSceneCapturePath) :

		LinearPostProcessMaterial(linearPostProcessMaterial),
		ColorGradingPostProcessMaterial(colorGradingPostProcessMaterial),
		PostProcessVolumePath(postProcessVolumePath),
		CyberGafferSceneCapturePath(cyberGafferSceneCapturePath) {}
};

UCLASS(Config=Editor, DefaultConfig)
class CYBERGAFFERWINDOW_API UCyberGafferWindowSettings : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "CyberGaffer|Materials")
	TMap<FString, FCyberGafferWindowSceneSettings> ScenesSettings;

	UPROPERTY(Config, EditAnywhere, Category = "CyberGaffer|Materials")
	FText ShadersIncludePath;

	TOptional<FCyberGafferWindowSceneSettings*> GetSettingsForScene(const FString& sceneName);
	TOptional<FCyberGafferWindowSceneSettings*> GetSettingsForCurrentScene();
};
