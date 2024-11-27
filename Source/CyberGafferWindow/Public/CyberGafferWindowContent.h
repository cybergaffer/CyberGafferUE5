// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyCustomizationHelpers.h"
#include "CyberGafferWindowFields.h"
#include "Materials/MaterialParameterCollection.h"
#include "HAL/CriticalSection.h"

// class FStructOnScope;



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
	TObjectPtr<UCyberGafferWindowSettings> _settings;
	
	TWeakPtr<SDockTab> _containingTab;

	// TSharedPtr<FStructOnScope> _windowFields;
	// TSharedPtr<IStructureDetailsView> _detailsView;

	TOptional<FString> _currentSceneName;
	mutable FCriticalSection _currentSceneNameCriticalSection;

	// void OnPropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent);

	TWeakObjectPtr<UMaterialInstance> _postProcessMaterial;
	TWeakObjectPtr<UMaterialInstance> _cameraPostProcessMaterial;
	
	TSharedPtr<SObjectPropertyEntryBox> _postProcessMaterialSelector;
	TSharedPtr<SObjectPropertyEntryBox> _cameraPostProcessMaterialSelector;

	TWeakObjectPtr<UMaterialParameterCollection> _postProcessMaterialParameters;

	static UMaterialInstance* LoadMaterialUsingPath(const FString& path);

	TOptional<FString> ReadCurrentSceneName();
	TOptional<FString> GetCurrentSceneName() const;
	void SetCurrentSceneName(const FString& newSceneName);

	void SaveMaterialChanges(TWeakObjectPtr<UMaterialInstance> material);

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

	bool IsPostProcessMaterialValid() const;
};
