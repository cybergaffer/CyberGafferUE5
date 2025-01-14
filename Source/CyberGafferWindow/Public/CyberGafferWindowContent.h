#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyCustomizationHelpers.h"
#include "CyberGafferWindowSettings.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/ObjectSaveContext.h"
#include "CyberGafferSceneCapture.h"


#include "atomic"

enum PostProcessMaterialType {
	Linear,
	ColorGrading
};

class APostProcessVolume;
class UCyberGafferSceneCaptureComponent2D;

/**
 * 
 */
class SCyberGafferWindowContent : public SCompoundWidget {
public:

	SLATE_BEGIN_ARGS(SCyberGafferWindowContent) : __containingTab() {}
		SLATE_ARGUMENT(TSharedPtr<SDockTab>, _containingTab)
	SLATE_END_ARGS()

	// CyberGafferWindowContent();
	// ~CyberGafferWindowContent();
	
	void Construct(const FArguments& args);

private:
	TStrongObjectPtr<UCyberGafferWindowSettings> _settings = nullptr;
	
	TWeakPtr<SDockTab> _containingTab = nullptr;
	FDelegateHandle _currentSceneChangedDelegateHandle = {};

	FDelegateHandle _tempSceneSavedDelegateHandle = {};
	std::atomic_bool _isTempScene = false;
	
	void OnParentTabClosed(TSharedRef<SDockTab> parentTab);

	static UMaterialInstance* LoadMaterialUsingPath(const FString& path);

	TOptional<FString> GetCurrentSceneName();
	void OnSceneChanged(const FString& filename, bool asTemplate);
	void OnTempSceneSaved(UWorld* world, FObjectPostSaveContext postSaveContext);

	FReply CreatePostProcessMaterialInstance(const PostProcessMaterialType type);

	
	// Linear post process material controls
	TWeakObjectPtr<UMaterialInstance> _linearPostProcessMaterial;
	
	FString GetLinearPostProcessMaterialPath() const;
	void OnLinearPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);
	// Linear post process material controls end

	
	// Color grading post process material controls
	TWeakObjectPtr<UMaterialInstance> _colorGradingPostProcessMaterial;
	
	FString GetColorGradingPostProcessMaterialPath() const;
	void OnColorGradingPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);

	TOptional<float> GetExposureCompensation() const;
	void OnExposureCompensationValueChanged(float value);
	void OnExposureCompensationValueCommited(const float newValue, ETextCommit::Type commitType);

	FLinearColor GetColorGradingColor() const;
	void OnColorGradingValueChanged(FLinearColor color);
	void OnColorGradingCaptureEnd();

	bool IsColorGradingPostProcessMaterialValid() const;
	// Color grading post process material controls end

	
	// Post Process Volume
	TObjectPtr<APostProcessVolume> _postProcessVolume;
	// FString _postProcessVolumePath = "";
	FString GetPostProcessVolumePath() const;
	void OnPostProcessVolumePathChanged(const FAssetData& assetData);
	void SavePostProcessVolumePath();
	// Post Process Volume end


	// CyberGaffer Scene Capture
	TObjectPtr<ACyberGafferSceneCapture> _cyberGafferSceneCapture;
	TObjectPtr<UCyberGafferSceneCaptureComponent2D> _cyberGafferSceneCaptureComponent;
	bool IsCyberGafferSceneCaptureComponentValid() const;
	UCyberGafferSceneCaptureComponent2D* FindCyberGafferSceneCaptureComponent() const;
	// FString _cyberGafferSceneCapturePath = "";
	FString GetCyberGafferSceneCapturePath() const;
	void OnCyberGafferSceneCaptureChanged(const FAssetData& assetData);
	void SaveCyberGafferSceneCapturePath();

	TOptional<float> GetLumenCacheResolution() const;
	void OnLumenCacheResolutionValueChanged(float value);
	// CyberGaffer Scene Capture end

	FText GetShadersIncludePath() const;
	void OnShadersIncludePathCommitted(const FText& newText, ETextCommit::Type commitType);

	void SaveMaterialChanges(UMaterialInterface* material);
	void SaveMaterialsChanges(const TArray<UMaterialInterface*>& materials);

	FReply RecompileShaders();
};
