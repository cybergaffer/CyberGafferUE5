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

	FCyberGafferWindowSceneSettings() : LinearPostProcessMaterial(""), ColorGradingPostProcessMaterial("") {}
	FCyberGafferWindowSceneSettings(const FString& linearPostProcessMaterial, const FString& colorGradingPostProcessMaterial)
		: LinearPostProcessMaterial(linearPostProcessMaterial), ColorGradingPostProcessMaterial(colorGradingPostProcessMaterial) {}
};

UCLASS(Config=CyberGafferWindow)
class UCyberGafferWindowSettings : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "CyberGaffer|Materials")
	TMap<FString, FCyberGafferWindowSceneSettings> ScenesSettings;

	UPROPERTY(Config, EditAnywhere, Category = "CyberGaffer|Materials")
	FText ShadersIncludePath;

	TOptional<FCyberGafferWindowSceneSettings*> GetSettingsForScene(const FString& sceneName);
};
