#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyCustomizationHelpers.h"
#include "CyberGafferProjectSettings.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/ObjectSaveContext.h"
#include "CyberGafferSceneCapture.h"
#include "IDetailsView.h"


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
	
	void Construct(const FArguments& args);
	virtual ~SCyberGafferWindowContent() override;

private:
	TWeakPtr<SDockTab> _containingTab = nullptr;
	FDelegateHandle _currentSceneChangedDelegateHandle = {};

	FDelegateHandle _tempSceneSavedDelegateHandle = {};
	std::atomic_bool _isTempScene = false;
	
	TStrongObjectPtr<UCyberGafferProjectSettings> _projectSettings = nullptr;
	FCyberGafferSceneSettings* _currentSceneSettings = nullptr;
	FCyberGafferShadersConfig* _shaderConfig = nullptr;

	TSharedPtr<TStructOnScope<FCyberGafferAutomationSettings>> _automationSettings;
	TSharedPtr<IStructureDetailsView> _automationSettingsView;
	
	TSharedPtr<FStructOnScope> _currentSceneSettingsUI;
	TSharedPtr<IStructureDetailsView> _currentSceneSettingsView;

	TSharedPtr<FStructOnScope> _shadersConfigUI;
	TSharedPtr<IStructureDetailsView> _shadersConfigView;

	void LoadSerializedSettings();
	void SerializeSettings();

	void OnScenePropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent);
	void OnShaderConfigPropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent);
	
	void OnParentTabClosed(TSharedRef<SDockTab> parentTab);

	static UMaterialInstance* LoadMaterialUsingPath(const FString& path);

	TOptional<FString> GetCurrentSceneName();
	void OnSceneChanged(const FString& filename, bool asTemplate);
	void OnTempSceneSaved(UWorld* world, FObjectPostSaveContext postSaveContext);

	FReply CreatePostProcessMaterialInstance(const PostProcessMaterialType type);

	
	TWeakObjectPtr<UMaterialInstance> _linearPostProcessMaterial;
	
	FString GetLinearPostProcessMaterialPath() const;
	// void OnLinearPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);

	
	TWeakObjectPtr<UMaterialInstance> _colorGradingPostProcessMaterial;
	
	FString GetColorGradingPostProcessMaterialPath() const;
	// void OnColorGradingPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);

public:
	FReply OnExecuteAutomationClicked();
	
	TOptional<float> GetExposureCompensation() const;
	void OnExposureCompensationValueChanged(float value);
	void OnExposureCompensationValueCommited(const float newValue, ETextCommit::Type commitType);

	FLinearColor GetColorGradingColor() const;
	void OnColorGradingValueChanged(FLinearColor color);
	void OnColorGradingCaptureEnd();

	bool IsColorGradingPostProcessMaterialValid() const;


	
	// Post Process Volume
	TObjectPtr<APostProcessVolume> _postProcessVolume;
	// FString _postProcessVolumePath = "";
	FString GetPostProcessVolumePath() const;
	void OnPostProcessVolumePathChanged(const FAssetData& assetData);
	// void SavePostProcessVolumePath();
	// Post Process Volume end


	// CyberGaffer Scene Capture
	TObjectPtr<ACyberGafferSceneCapture> _cyberGafferSceneCapture;
	TObjectPtr<UCyberGafferSceneCaptureComponent2D> _cyberGafferSceneCaptureComponent;
	bool IsCyberGafferSceneCaptureComponentValid() const;
	UCyberGafferSceneCaptureComponent2D* FindCyberGafferSceneCaptureComponent() const;
	// FString _cyberGafferSceneCapturePath = "";
	FString GetCyberGafferSceneCapturePath() const;
	void OnCyberGafferSceneCaptureChanged(const FAssetData& assetData);
	// void SaveCyberGafferSceneCapturePath();

	TOptional<float> GetLumenCacheResolution() const;
	void OnLumenCacheResolutionValueChanged(float value);
	// CyberGaffer Scene Capture end

	FText GetShadersIncludePath() const;
	void OnShadersIncludePathCommitted();

	void SaveMaterialChanges(UMaterialInterface* material);
	void SaveMaterialsChanges(const TArray<UMaterialInterface*>& materials);

	FReply RecompileShaders();
};
