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
	Linear,
	ColorGrading
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
	FDelegateHandle _currentSceneChangedDelegateHandle;

	void OnParentTabClosed(TSharedRef<SDockTab> parentTab);

	static UMaterialInstance* LoadMaterialUsingPath(const FString& path);

	TOptional<FString> GetCurrentSceneName();
	void OnSceneChanged(const FString& filename, bool asTemplate);

	FReply CreatePostProcessMaterialInstance(const PostProcessMaterialType type);

	// Linear post process material controls
	TWeakObjectPtr<UMaterialInstance> _linearPostProcessMaterial;
	TSharedPtr<SObjectPropertyEntryBox> _linearPostProcessMaterialSelector;
	
	FString GetLinearPostProcessMaterialPath() const;
	void OnLinearPostProcessMaterialSelectorValueChanged(const FAssetData& assetData);
	// Linear post process material controls end
	
	// Color grading post process material controls
	TWeakObjectPtr<UMaterialInstance> _colorGradingPostProcessMaterial;
	TSharedPtr<SObjectPropertyEntryBox> _colorGradingPostProcessMaterialSelector;
	
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

	FText GetShadersIncludePath() const;
	void OnShadersIncludePathCommitted(const FText& newText, ETextCommit::Type commitType);

	void SaveMaterialChanges(UMaterialInterface* material);
	void SaveMaterialsChanges(const TArray<UMaterialInterface*>& materials);

	FReply RecompileShaders();
};
