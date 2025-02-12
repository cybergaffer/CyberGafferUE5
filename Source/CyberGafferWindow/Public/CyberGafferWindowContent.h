#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CyberGafferProjectSettings.h"
#include "UObject/StrongObjectPtr.h"
#include "CyberGafferSceneCapture.h"
#include "IDetailsView.h"
#include "UObject/ObjectSaveContext.h"


#include "atomic"

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
	// FCyberGafferShadersConfig* _shaderConfig = nullptr;

	TSharedPtr<TStructOnScope<FCyberGafferAutomationSettings>> _automationSettings;
	TSharedPtr<IStructureDetailsView> _automationSettingsView;
	
	TSharedPtr<FStructOnScope> _currentSceneSettingsUI;
	TSharedPtr<IStructureDetailsView> _currentSceneSettingsView;

	// TSharedPtr<FStructOnScope> _shadersConfigUI;
	// TSharedPtr<IStructureDetailsView> _shadersConfigView;

	void LoadSerializedSettings();
	void SerializeSettings();

	void OnScenePropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent);
	// void OnShaderConfigPropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent);
	
	void OnParentTabClosed(TSharedRef<SDockTab> parentTab);

	static UMaterialInstance* LoadMaterialUsingPath(const FString& path);

	TOptional<FString> GetCurrentSceneName();
	void OnSceneChanged(const FString& filename, bool asTemplate);
	void OnTempSceneSaved(UWorld* world, FObjectPostSaveContext postSaveContext);

	FReply CreatePostProcessMaterialInstance();

	
	TWeakObjectPtr<UMaterialInstance> _postProcessMaterial;
	
	FString GetLinearPostProcessMaterialPath() const;

	APostProcessVolume* FindPostProcessVolume(UWorld* world);
	ACyberGafferSceneCapture* FindCyberGafferSceneCapture(UWorld* world);
	
	// void OnLinearPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);

	
	// TWeakObjectPtr<UMaterialInstance> _colorGradingPostProcessMaterial;
	
	// FString GetColorGradingPostProcessMaterialPath() const;
	// void OnColorGradingPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);
	
	

public:
	FReply OnExecuteAutomationClicked();

	bool IsPostProcessMaterialValid() const;
	
	TOptional<float> GetExposureCompensation() const;
	void OnExposureCompensationValueChanged(float value);
	void OnExposureCompensationValueCommited(const float newValue, ETextCommit::Type commitType);

	FLinearColor GetMultiplierColor() const;
	void OnMultiplierColorChanged(FLinearColor color);
	void OnMultiplierColorCommited();
	
	// Post Process Volume
	TWeakObjectPtr<APostProcessVolume> _postProcessVolume;
	FString GetPostProcessVolumePath() const;
	void OnPostProcessVolumePathChanged(const FAssetData& assetData);
	// Post Process Volume end


	// CyberGaffer Scene Capture
	TWeakObjectPtr<ACyberGafferSceneCapture> _cyberGafferSceneCapture;
	TWeakObjectPtr<UCyberGafferSceneCaptureComponent2D> _cyberGafferSceneCaptureComponent;
	bool IsCyberGafferSceneCaptureComponentValid() const;
	UCyberGafferSceneCaptureComponent2D* FindCyberGafferSceneCaptureComponent() const;
	FString GetCyberGafferSceneCapturePath() const;
	void OnCyberGafferSceneCaptureChanged(const FAssetData& assetData);

	TOptional<float> GetLumenFinalGatherQuality() const;
	void OnLumenFinalGatherQualityValueChanged(float value);
	void OnLumenFinalGatherQualityValueCommited(float value, ETextCommit::Type commitType);
	// CyberGaffer Scene Capture end

	// FText GetShadersIncludePath() const;
	// void OnShadersIncludePathCommitted();

	void SaveMaterialChanges(UMaterialInterface* material);
	void SaveMaterialsChanges(const TArray<UMaterialInterface*>& materials);

	// FReply RecompileShaders();
};
