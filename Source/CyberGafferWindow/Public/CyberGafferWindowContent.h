// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyCustomizationHelpers.h"
#include "CyberGafferWindowSettings.h"
#include "UObject/StrongObjectPtr.h"
#include "HAL/CriticalSection.h"

enum PostProcessMaterialType {
	Global,
	Camera
};

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
	TStrongObjectPtr<UCyberGafferWindowSettings> _settings;
	
	TWeakPtr<SDockTab> _containingTab;

	// TOptional<FString> _currentSceneName;
	// mutable FCriticalSection _currentSceneNameCriticalSection;
	FDelegateHandle _currentSceneChangedDelegateHandle;

	TWeakObjectPtr<UMaterialInstance> _postProcessMaterial;
	TWeakObjectPtr<UMaterialInstance> _cameraPostProcessMaterial;
	
	TSharedPtr<SObjectPropertyEntryBox> _postProcessMaterialSelector;
	TSharedPtr<SObjectPropertyEntryBox> _cameraPostProcessMaterialSelector;

	void OnParentTabClosed(TSharedRef<SDockTab> parentTab);

	static UMaterialInstance* LoadMaterialUsingPath(const FString& path);

	TOptional<FString> ReadCurrentSceneName();
	// TOptional<FString> GetCurrentSceneName() const;
	// void SetCurrentSceneName(const FString& newSceneName);
	void OnSceneChanged(const FString& filename, bool asTemplate);

	void SaveMaterialChanges(UMaterialInterface* material);
	void SaveMaterialsChanges(TArray<UMaterialInterface*> materials);

	FString GetPostProcessMaterialPath() const;
	void OnPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);
	
	FString GetCameraPostProcessMaterialPath() const;
	void OnCameraPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);

	TOptional<float> GetExposureCompensation() const;
	void OnExposureCompensationValueChanged(float value);
	void OnExposureCompensationValueCommited(const float newValue, ETextCommit::Type commitType);

	FLinearColor GetColorGradingColor() const;
	void OnColorGradingValueChanged(FLinearColor color);
	void OnColorGradingCaptureEnd();

	FText GetShadersIncludePath() const;
	void OnShadersIncludePathCommitted(const FText& newText, ETextCommit::Type commitType);

	bool IsPostProcessMaterialValid() const;

	FReply CreatePostProcessMaterialInstance(const PostProcessMaterialType type);
};
