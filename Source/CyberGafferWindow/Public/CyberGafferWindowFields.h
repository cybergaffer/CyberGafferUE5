#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstance.h"
#include "Math/Color.h"

#include "CyberGafferWindowFields.generated.h"

USTRUCT()
struct FCyberGafferWindowSceneSettings {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Materials")
	FString PostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	FString CameraPostProcessMaterial;

	FCyberGafferWindowSceneSettings() : PostProcessMaterial(""), CameraPostProcessMaterial("") {}
	FCyberGafferWindowSceneSettings(const FString& postProcessMaterial, const FString& cameraPostProcessMaterial)
		: PostProcessMaterial(postProcessMaterial), CameraPostProcessMaterial(cameraPostProcessMaterial) {}
};

UCLASS(Config=CyberGafferWindow)
class UCyberGafferWindowSettings : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere)
	TMap<FString, FCyberGafferWindowSceneSettings> ScenesSettings;

	TOptional<FCyberGafferWindowSceneSettings*> GetSettingsForScene(const FString& sceneName);
};
