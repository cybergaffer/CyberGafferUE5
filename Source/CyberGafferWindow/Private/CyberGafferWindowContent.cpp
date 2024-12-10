#include "CyberGafferWindowContent.h"
#include "CyberGafferWindow.h"
#include "CyberGafferLog.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Layout/Margin.h"
#include "Styling/AppStyle.h"
#include "UObject/ConstructorHelpers.h"

#include "Materials/MaterialInstanceConstant.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Colors/SColorWheel.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "CyberGaffer.h"
#include "Editor/LevelEditor/Public/LevelEditorActions.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Dialogs/Dialogs.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

// CyberGafferWindowContent::CyberGafferWindowContent() { }
// CyberGafferWindowContent::~CyberGafferWindowContent() { }

void SCyberGafferWindowContent::Construct(const FArguments& args) {
	// Code for DetailView autogenerated UI
	// FStructureDetailsViewArgs structureDetailsViewArgs;
	// FDetailsViewArgs detailsViewArgs;
	// detailsViewArgs.bAllowSearch = false;

	// auto& propertyEditor = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	// _windowFields = MakeShared<FStructOnScope>(FCyberGafferWindowFields::StaticStruct());
	// _detailsView = propertyEditor.CreateStructureDetailView(detailsViewArgs, structureDetailsViewArgs, _windowFields);
	// _detailsView->GetOnFinishedChangingPropertiesDelegate().AddSP(this, &SCyberGafferWindowContent::OnPropertiesChanged);

	_settings = TStrongObjectPtr<UCyberGafferWindowSettings>(NewObject<UCyberGafferWindowSettings>());

	OnSceneChanged(FString(""), false);

	_containingTab = args.__containingTab;

	_currentSceneChangedDelegateHandle = FEditorDelegates::OnMapOpened.AddRaw(this, &SCyberGafferWindowContent::OnSceneChanged);
	TSharedPtr<SDockTab> tab = _containingTab.Pin();
	if (tab.IsValid()) {
		tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &SCyberGafferWindowContent::OnParentTabClosed));
	}
	
	_linearPostProcessMaterialSelector = SNew(SObjectPropertyEntryBox)
		.ObjectPath(this, &SCyberGafferWindowContent::GetLinearPostProcessMaterialPath)
		.DisplayBrowse(true)
		.DisplayThumbnail(true)
		.AllowedClass(UMaterialInstance::StaticClass())
		.EnableContentPicker(true)
		.OnObjectChanged(FOnSetObject::CreateSP(this, &SCyberGafferWindowContent::OnLinearPostProcessMaterialSelectorValueChanged));
	
	_colorGradingPostProcessMaterialSelector = SNew(SObjectPropertyEntryBox)
		.ObjectPath(this, &SCyberGafferWindowContent::GetColorGradingPostProcessMaterialPath)
		.DisplayBrowse(true)
		.DisplayThumbnail(true)
		.AllowedClass(UMaterialInstance::StaticClass())
		.EnableContentPicker(true)
		.OnObjectChanged(FOnSetObject::CreateSP(this, &SCyberGafferWindowContent::OnColorGradingPostProcessMaterialSelectorValueChanged));

	TSharedRef<SSeparator> separator = SNew(SSeparator).Orientation(Orient_Vertical);

	const float headerWidth = 0.4f;
	const float valueWidth = 0.6f;

	const FMargin headerMargin = FMargin(5, 0, 10, 0);
	const FMargin valueMargin = FMargin(10, 0, 5, 0);
	const float verticalSlotPadding = 4.0f;
	
	ChildSlot [
		// Code for DetailView autogenerated UI
		// _detailsView->GetWidget().ToSharedRef()
		
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(headerMargin)
			[
				SNew(SButton)
				.Text(LOCTEXT("NewLinearPPMIText", "New post linear process material for scene"))
				.OnClicked(this, &SCyberGafferWindowContent::CreatePostProcessMaterialInstance, PostProcessMaterialType::Linear)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.Padding(headerMargin)
			[
				SNew(SButton)
				.Text(LOCTEXT("NewColorGradingPPMIText", "New color grading post process material for scene"))
				.OnClicked(this, &SCyberGafferWindowContent::CreatePostProcessMaterialInstance, PostProcessMaterialType::ColorGrading)
			]
			
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(headerWidth)
			.HAlign(HAlign_Left)
			.Padding(headerMargin)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LinearPostProcessMaterialText", "Linear Post Process Material"))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				separator
			]
			+SHorizontalBox::Slot()
			.FillWidth(valueWidth)
			.HAlign(HAlign_Right)
			.Padding(valueMargin)
			[
				_linearPostProcessMaterialSelector.ToSharedRef()
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(headerWidth)
			.HAlign(HAlign_Left)
			.Padding(headerMargin)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ColorGradingPostProcessMaterialText", "Color Grading Post Process Material"))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				separator
			]
			+SHorizontalBox::Slot()
			.FillWidth(valueWidth)
			.HAlign(HAlign_Right)
			.Padding(valueMargin)
			[
				_colorGradingPostProcessMaterialSelector.ToSharedRef()
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(headerWidth)
			.HAlign(HAlign_Left)
			.Padding(headerMargin)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExposureCompensationText", "ExposureCompensation"))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				separator
			]
			+SHorizontalBox::Slot()
			.FillWidth(valueWidth)
			.HAlign(HAlign_Right)
			.Padding(valueMargin)
			[
				SNew(SNumericEntryBox<float>)
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.AllowSpin(true)
				.MinValue(TOptional<float>())
				.MaxValue(TOptional<float>())
				.MinSliderValue(-10.0)
				.MaxSliderValue(10.0)
				.Delta(0.01f)
				.LinearDeltaSensitivity(0.001f)
				.Value(this, &SCyberGafferWindowContent::GetExposureCompensation)
				.OnValueChanged(this, &SCyberGafferWindowContent::OnExposureCompensationValueChanged)
				.OnValueCommitted(this, &SCyberGafferWindowContent::OnExposureCompensationValueCommited)
				.ToolTipText(LOCTEXT("ExposureCompensationText", "Exposure Compensation"))
				.IsEnabled(this, &SCyberGafferWindowContent::IsColorGradingPostProcessMaterialValid)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(headerWidth)
			.HAlign(HAlign_Left)
			.Padding(headerMargin)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ColorGradingText", "Color Grading"))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				separator
			]
			+SHorizontalBox::Slot()
			.FillWidth(valueWidth)
			.HAlign(HAlign_Right)
			.Padding(valueMargin)
			[
				SNew(SColorWheel)
				.SelectedColor(this, &SCyberGafferWindowContent::GetColorGradingColor)
				.OnValueChanged(this, &SCyberGafferWindowContent::OnColorGradingValueChanged)
				.OnMouseCaptureEnd(this, &SCyberGafferWindowContent::OnColorGradingCaptureEnd)
				.ToolTipText(LOCTEXT("ExposureCompensationText", "Exposure Compensation"))
				.IsEnabled(this, &SCyberGafferWindowContent::IsColorGradingPostProcessMaterialValid)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(headerWidth)
			.HAlign(HAlign_Left)
			.Padding(headerMargin)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ShadersIncludePathText", "Shaders Include Path"))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				separator
			]
			+SHorizontalBox::Slot()
			.FillWidth(valueWidth)
			.HAlign(HAlign_Right)
			.Padding(valueMargin)
			[
				SNew(SEditableTextBox)
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.Text(this, &SCyberGafferWindowContent::GetShadersIncludePath)
				.OnTextCommitted(this, &SCyberGafferWindowContent::OnShadersIncludePathCommitted)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(verticalSlotPadding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.Padding(headerMargin)
			[
				SNew(SButton)
				.Text(LOCTEXT("RecompileShadersText", "Recompile Shaders"))
				.OnClicked(this, &SCyberGafferWindowContent::RecompileShaders)
			]
			
		]
	];
}

void SCyberGafferWindowContent::OnParentTabClosed(TSharedRef<SDockTab> parentTab) {
	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnParentTabClosed"));

	TSharedPtr<SDockTab> currentTab = _containingTab.Pin();
	if (currentTab.IsValid()) {
		if (currentTab == parentTab) {
			FEditorDelegates::OnMapOpened.Remove(_currentSceneChangedDelegateHandle);
		} else {
			CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnParentTabClosed: tab mismatch"));
		}
	} else {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnParentTabClosed: current tab is invalid"));
	}
}

UMaterialInstance* SCyberGafferWindowContent::LoadMaterialUsingPath(const FString& path) {
	if (path.IsEmpty()) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::LoadMaterialUsingPath: path is empty"));
		return nullptr;
	}
	
	FAssetRegistryModule& module = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FAssetData assetData;
	FSoftObjectPath objectPath(path);
	if (module.Get().TryGetAssetByObjectPath(objectPath, assetData) == UE::AssetRegistry::EExists::Exists) {
		return Cast<UMaterialInstance>(assetData.GetAsset());
	}
	
	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::LoadMaterialUsingPath: failed to load asset at path %s"), *path);
	return nullptr;
}

TOptional<FString> SCyberGafferWindowContent::GetCurrentSceneName() {
	UWorld* world = GEditor->GetEditorWorldContext().World();
	if (!world) {
		CYBERGAFFER_LOG(Warning, TEXT(" SCyberGafferWindowContent::GetCurrentSceneName: world is null"));
		return TOptional<FString>();
	}

	const UPackage* worldPackage = world->GetPackage();
	const FString worldPackageName = worldPackage->GetName();
	const auto isTemp = FPackageName::IsTempPackage(worldPackageName);

	// FPackageName::Name
	// CYBERGAFFER_LOG(Warning, TEXT(" SCyberGafferWindowContent::GetCurrentSceneName: outermost file path: %s, temp: %i"), *mapName, isTemp);
	if (isTemp) {
		if (!_tempSceneSavedDelegateHandle.IsValid()) {
			_tempSceneSavedDelegateHandle = FEditorDelegates::PostSaveWorldWithContext.AddRaw(this, &SCyberGafferWindowContent::OnTempSceneSaved);
		}
		
		CYBERGAFFER_LOG(Warning, TEXT(" SCyberGafferWindowContent::GetCurrentSceneName: scene file doesn't saved yet"));
		return TOptional<FString>();
	} else {
		if (_tempSceneSavedDelegateHandle.IsValid()) {
			FEditorDelegates::PostSaveWorldWithContext.Remove(_tempSceneSavedDelegateHandle);
			_tempSceneSavedDelegateHandle.Reset();
		}
	}
	
	const auto shortPackageName = FPackageName::GetShortName(worldPackageName);
	CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::GetCurrentSceneName: package name: %s"), *shortPackageName);

	return shortPackageName;
}

void SCyberGafferWindowContent::OnSceneChanged(const FString& filename, bool asTemplate) {
	const TOptional<FString> sceneName = GetCurrentSceneName();
	if (sceneName.IsSet()) {
		const auto sceneSettings = _settings->ScenesSettings.Find(sceneName.GetValue());
		if (sceneSettings) {
			_linearPostProcessMaterial = LoadMaterialUsingPath(sceneSettings->LinearPostProcessMaterial);
			_colorGradingPostProcessMaterial = LoadMaterialUsingPath(sceneSettings->ColorGradingPostProcessMaterial);
		} else {
			_settings->ScenesSettings.Add(sceneName.GetValue(), FCyberGafferWindowSceneSettings());
			_linearPostProcessMaterial = nullptr;
			_colorGradingPostProcessMaterial = nullptr;
			_settings->SaveConfig();
		}
	}
}

void SCyberGafferWindowContent::OnTempSceneSaved(UWorld* world, FObjectPostSaveContext postSaveContext) {
	OnSceneChanged(FString(), false);
}

FReply SCyberGafferWindowContent::CreatePostProcessMaterialInstance(const PostProcessMaterialType type) {
	IAssetTools& assetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UMaterialInstanceConstantFactoryNew* materialFactory = NewObject<UMaterialInstanceConstantFactoryNew>();

	FString initialParentPath;
	switch (type) {
	case Linear: {
			initialParentPath = "Script/Engine.Material'/CyberGaffer/Materials/CyberGafferLinearPostProcess.CyberGafferLinearPostProcess'";
			break;
		}
	case ColorGrading: {
			initialParentPath = "Script/Engine.Material'/CyberGaffer/Materials/CyberGafferColorGradingPostProcess.CyberGafferColorGradingPostProcess'";
			break;
		}
	}
	
	UMaterialInterface* initialParent = Cast<UMaterialInterface>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *initialParentPath));
	if (initialParent == nullptr) {
		return FReply::Unhandled();
	}
	
	const auto cyberGafferProjectContentDir = FString("/Game/CyberGaffer");
	const TOptional<FString> sceneName = GetCurrentSceneName();
	const auto newAssetName = FString::Printf(TEXT("%s_%s"), *initialParent->GetName(), *sceneName.GetValue());

	const FString packagePath = UPackageTools::SanitizePackageName(cyberGafferProjectContentDir + TEXT("/") + newAssetName);
	
	const FString newAssetPath = FPaths::Combine(FPaths::ProjectContentDir(), "CyberGaffer", *newAssetName) + ".uasset";
	if (FPaths::FileExists(newAssetPath)) {
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FileExistsText", "File already exists"));
		return FReply::Handled();
	}
	
	materialFactory->InitialParent = initialParent;
	UObject* asset = assetTools.CreateAsset(newAssetName, cyberGafferProjectContentDir, UMaterialInstanceConstant::StaticClass(), materialFactory);
	if (asset == nullptr) {
		return FReply::Unhandled();
	}

	auto newMaterail = Cast<UMaterialInstanceConstant>(asset);

	switch (type) {
	case Linear: {
		_linearPostProcessMaterial = newMaterail;
		OnLinearPostProcessMaterialSelectorValueChanged(newMaterail);
		break;
	}
	case ColorGrading: {
		_colorGradingPostProcessMaterial = newMaterail;
		OnColorGradingPostProcessMaterialSelectorValueChanged(newMaterail);
		break;
	}
	}

	SaveMaterialChanges(newMaterail);

	return FReply::Handled();
}

FString SCyberGafferWindowContent::GetLinearPostProcessMaterialPath() const {
	if (_linearPostProcessMaterial.IsValid()) {
		return _linearPostProcessMaterial->GetPathName();
	}

	return FString();
}

void SCyberGafferWindowContent::OnLinearPostProcessMaterialSelectorValueChanged(const FAssetData& assetData) {
	const TOptional<FString> currentSceneName = GetCurrentSceneName();
	if (!currentSceneName.IsSet()) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnLinearPostProcessMaterialSelectorValueChanged: current scene name is null"));
		return;
	}
	
	TOptional<FCyberGafferWindowSceneSettings*> sceneSettings = _settings->GetSettingsForScene(currentSceneName.GetValue());
	if (!sceneSettings.IsSet()) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnLinearPostProcessMaterialSelectorValueChanged: current scene settings is null, scene: %s"), *currentSceneName.GetValue());
		return;
	}
	
	UMaterialInstance* materialInstance = Cast<UMaterialInstance>(assetData.GetAsset());
	if (materialInstance) {
		auto materialPath = materialInstance->GetPathName();
		sceneSettings.GetValue()->LinearPostProcessMaterial = materialPath;
	} else {
		sceneSettings.GetValue()->LinearPostProcessMaterial = "";
	}
	_settings->SaveConfig();
	
	_linearPostProcessMaterial = materialInstance;
}

FString SCyberGafferWindowContent::GetColorGradingPostProcessMaterialPath() const {
	if (_colorGradingPostProcessMaterial.IsValid()) {
		return _colorGradingPostProcessMaterial->GetPathName();
	}

	return FString();
}

void SCyberGafferWindowContent::OnColorGradingPostProcessMaterialSelectorValueChanged(const FAssetData& assetData) {
	const TOptional<FString> currentSceneName = GetCurrentSceneName();
	if (!currentSceneName.IsSet()) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnColorGradingPostProcessMaterialSelectorValueChanged: current scene name is null"));
		return;
	}
	
	TOptional<FCyberGafferWindowSceneSettings*> sceneSettings = _settings->GetSettingsForScene(currentSceneName.GetValue());
	if (!sceneSettings.IsSet()) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnColorGradingPostProcessMaterialSelectorValueChanged: current scene settings is null"));
		return;
	}
	
	UMaterialInstance* materialInstance = Cast<UMaterialInstance>(assetData.GetAsset());
	if (materialInstance) {
		sceneSettings.GetValue()->ColorGradingPostProcessMaterial = materialInstance->GetPathName();
	} else {
		sceneSettings.GetValue()->ColorGradingPostProcessMaterial = "";
	}
	_settings->SaveConfig();
	
	_colorGradingPostProcessMaterial = materialInstance;
}

TOptional<float> SCyberGafferWindowContent::GetExposureCompensation() const {
	if (!IsColorGradingPostProcessMaterialValid()) {
		return 0.0f;
	}
	
	FMemoryImageMaterialParameterInfo parameterInfo(TEXT("Expose Compensation"));
	FMaterialParameterMetadata metadata;
	const bool callResult = _colorGradingPostProcessMaterial->GetParameterValue(EMaterialParameterType::Scalar, parameterInfo, metadata);
	
	if (callResult) {
		return metadata.Value.AsScalar();
	}
	return 0.0f;
}

void SCyberGafferWindowContent::OnExposureCompensationValueChanged(float value) {
	if (!IsColorGradingPostProcessMaterialValid()) {
		return;
	}

	auto instanceConstant = Cast<UMaterialInstanceConstant>(_colorGradingPostProcessMaterial);
	if (instanceConstant) {
		FMaterialParameterInfo parameterInfo(TEXT("Expose Compensation"));
		instanceConstant->SetScalarParameterValueEditorOnly(parameterInfo, value);
	}
}

void SCyberGafferWindowContent::OnExposureCompensationValueCommited(const float newValue, ETextCommit::Type commitType) {
	OnExposureCompensationValueChanged(newValue);
	SaveMaterialChanges(_colorGradingPostProcessMaterial.Get());
}

FLinearColor SCyberGafferWindowContent::GetColorGradingColor() const {
	if (!IsColorGradingPostProcessMaterialValid()) {
		return FLinearColor::White;
	}

	FMemoryImageMaterialParameterInfo parameterInfo(TEXT("Multiplier"));
	FMaterialParameterMetadata metadata;
	const bool callResult = _colorGradingPostProcessMaterial->GetParameterValue(EMaterialParameterType::Vector, parameterInfo, metadata);
	
	if (callResult) {
		return metadata.Value.AsLinearColor().LinearRGBToHSV();
	}

	return FLinearColor::White;
}

void SCyberGafferWindowContent::OnColorGradingValueChanged(FLinearColor color) {
	if (!IsColorGradingPostProcessMaterialValid()) {
		return;
	}

	auto instanceConstant = Cast<UMaterialInstanceConstant>(_colorGradingPostProcessMaterial);
	FMaterialParameterInfo parameterInfo(TEXT("Multiplier"));
	if (instanceConstant) {
		instanceConstant->SetVectorParameterValueEditorOnly(parameterInfo, color.HSVToLinearRGB());
	}
}

void SCyberGafferWindowContent::OnColorGradingCaptureEnd() {
	SaveMaterialChanges(_colorGradingPostProcessMaterial.Get());
}

bool SCyberGafferWindowContent::IsColorGradingPostProcessMaterialValid() const {
	return _colorGradingPostProcessMaterial.IsValid();
}

FText SCyberGafferWindowContent::GetShadersIncludePath() const {
	if (!_settings) {
		return FText();
	}

	return _settings->ShadersIncludePath;
}

void SCyberGafferWindowContent::OnShadersIncludePathCommitted(const FText& newText, ETextCommit::Type commitType) {
	if (!_settings) {
		return;
	}
	
	if (newText.CompareTo(_settings->ShadersIncludePath) == 0) {
		return;
	}

	if (!FPaths::DirectoryExists(newText.ToString())) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnShadersIncludePathCommitted: invalid path"));
		return;
	}

	const FString shadersDir = FModuleManager::Get().GetModuleChecked<FCyberGafferModule>("CyberGaffer").GetShaderDirectory();
	FString shadersIncludePath = FPaths::Combine(shadersDir, "Include");
	shadersIncludePath = shadersIncludePath.Replace(TEXT("/"), TEXT("\\"));
	if (FPaths::DirectoryExists(shadersIncludePath)) {
		const FString command = FString::Printf(TEXT("/c rd \"%s\""), *shadersIncludePath);
		FProcHandle result = FPlatformProcess::CreateProc(TEXT("cmd.exe"), *command, false, true, false, nullptr, 0, nullptr, nullptr, nullptr);

		while (FPlatformProcess::IsProcRunning(result)) {
			FPlatformProcess::Sleep(0.1f);
		}
	}

	if (FPaths::DirectoryExists(shadersIncludePath)) {
		CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnShadersIncludePathCommitted: failed to delete junction"));
		return;
	}

	const FString command = FString::Printf(TEXT("/c mklink /j \"%s\" \"%s\""), *shadersIncludePath, *newText.ToString().Replace(TEXT("/"), TEXT("\\")));
	FPlatformProcess::CreateProc(TEXT("cmd.exe"), *command, false, true, false, nullptr, 0, nullptr, nullptr, nullptr);

	_settings->ShadersIncludePath = newText;
	_settings->SaveConfig();

	RecompileShaders();
	
	// FLevelEditorActionCallbacks::ExecuteExecCommand(TEXT("RECOMPILESHADERS CHANGED"));
}

void SCyberGafferWindowContent::SaveMaterialChanges(UMaterialInterface* material) {
	if (material == nullptr) {
		return;
	}

	TArray<UMaterialInterface*> materials;
	materials.Add(material);
	SaveMaterialsChanges(materials);
}

void SCyberGafferWindowContent::SaveMaterialsChanges(const TArray<UMaterialInterface*>& materials) {
	if (materials.IsEmpty()) {
		return;
	}
	
	TArray<UPackage*> packagesToSave;

	for (auto material : materials) {
		material->PostEditChange();
		material->MarkPackageDirty();
		packagesToSave.Add(material->GetOutermost());
	}

	FEditorFileUtils::PromptForCheckoutAndSave(packagesToSave, true, false);
}

FReply SCyberGafferWindowContent::RecompileShaders() {
	TArray<UMaterialInterface*> materials;
	if (_linearPostProcessMaterial != nullptr) {
		materials.Add(_linearPostProcessMaterial->GetMaterial());
		materials.Add(_linearPostProcessMaterial.Get());

		FLevelEditorActionCallbacks::ExecuteExecCommand(FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_linearPostProcessMaterial->GetMaterial()->GetName()));
		FLevelEditorActionCallbacks::ExecuteExecCommand(FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_linearPostProcessMaterial->GetName()));
	}
	if (_colorGradingPostProcessMaterial != nullptr) {
		materials.Add(_colorGradingPostProcessMaterial->GetMaterial());
		materials.Add(_colorGradingPostProcessMaterial.Get());

		FLevelEditorActionCallbacks::ExecuteExecCommand(FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_colorGradingPostProcessMaterial->GetMaterial()->GetName()));
		FLevelEditorActionCallbacks::ExecuteExecCommand(FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_colorGradingPostProcessMaterial->GetName()));
	}

	SaveMaterialsChanges(materials);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
