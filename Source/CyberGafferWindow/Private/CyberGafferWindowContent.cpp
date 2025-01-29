#include "CyberGafferWindowContent.h"
#include "CyberGafferWindow.h"
#include "CyberGafferLog.h"
#include "CyberGafferWindowStyle.h"
#include "CyberGafferSettingsVersion.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "SWarningOrErrorBox.h"
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
#include "CyberGafferSceneCaptureComponent2D.h"
#include "CyberGafferWindowAutomationCustomization.h"
#include "CyberGafferWindowSceneControlsCustomization.h"
#include "CyberGafferWindowShadersConfigCustomization.h"
#include "IStructureDetailsView.h"
#include "OpenColorIOConfiguration.h"
#include "OpenColorIOWrapper.h"
#include "SWarningOrErrorBox.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Dialogs/Dialogs.h"
#include "Engine/PostProcessVolume.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ObjectWriter.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

void SCyberGafferWindowContent::Construct(const FArguments& args) {
	_containingTab = args.__containingTab;
	TSharedPtr<SDockTab> tab = _containingTab.Pin();
	if (tab.IsValid()) {
		tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &SCyberGafferWindowContent::OnParentTabClosed));
	}

	_projectSettings = TStrongObjectPtr<UCyberGafferProjectSettings>(NewObject<UCyberGafferProjectSettings>());
	LoadSerializedSettings();
	_currentSceneChangedDelegateHandle = FEditorDelegates::OnMapOpened.AddRaw(this, &SCyberGafferWindowContent::OnSceneChanged);
	OnSceneChanged(FString(""), false);

	auto& propertyEditor = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	
	FStructureDetailsViewArgs structureDetailsViewArgs;
	FDetailsViewArgs detailsViewArgs;
	detailsViewArgs.bAllowSearch = false;
	detailsViewArgs.bShowScrollBar = false;

	_automationSettings = MakeShared<TStructOnScope<FCyberGafferAutomationSettings>>();
	_automationSettings->InitializeAs<FCyberGafferAutomationSettings>();
	_automationSettingsView = propertyEditor.CreateStructureDetailView(detailsViewArgs, structureDetailsViewArgs, TSharedPtr<TStructOnScope<FCyberGafferAutomationSettings>>());
	
	_automationSettingsView->GetDetailsView()->RegisterInstancedCustomPropertyLayout(
		FCyberGafferAutomationSettings::StaticStruct(),
		FOnGetDetailCustomizationInstance::CreateLambda([this] {
			return FCyberGafferWindowAutomationCustomization::MakeInstance(SharedThis(this));
		})
	);
	_automationSettingsView->SetStructureData(_automationSettings);
	_automationSettingsView->GetDetailsView()->ForceRefresh();
	
	_currentSceneSettingsUI = MakeShared<FStructOnScope>(FCyberGafferSceneSettings::StaticStruct());
	CastChecked<UScriptStruct>(_currentSceneSettingsUI->GetStruct())->CopyScriptStruct(_currentSceneSettingsUI->GetStructMemory(), _currentSceneSettings);
	_currentSceneSettingsView = propertyEditor.CreateStructureDetailView(detailsViewArgs, structureDetailsViewArgs, TSharedPtr<FStructOnScope>());
	_currentSceneSettingsView->GetDetailsView()->RegisterInstancedCustomPropertyLayout(
		FCyberGafferSceneSettings::StaticStruct(),
		FOnGetDetailCustomizationInstance::CreateLambda([this] {
			return FCyberGafferWindowSceneControlsCustomization::MakeInstance(SharedThis(this));
		})
	);
	_currentSceneSettingsView->SetStructureData(_currentSceneSettingsUI);
	_currentSceneSettingsView->GetOnFinishedChangingPropertiesDelegate().AddSP(this, &SCyberGafferWindowContent::OnScenePropertiesChanged);
	
	_shadersConfigUI = MakeShared<FStructOnScope>(FCyberGafferShadersConfig::StaticStruct());
	CastChecked<UScriptStruct>(_shadersConfigUI->GetStruct())->CopyScriptStruct(_shadersConfigUI->GetStructMemory(), &_projectSettings.Get()->ShadersConfig);
	_shadersConfigView = propertyEditor.CreateStructureDetailView(detailsViewArgs, structureDetailsViewArgs, TSharedPtr<FStructOnScope>());
	_shadersConfigView->GetDetailsView()->RegisterInstancedCustomPropertyLayout(
		FCyberGafferShadersConfig::StaticStruct(),
		FOnGetDetailCustomizationInstance::CreateLambda([this] {
			return FCyberGafferWindowShadersConfigCustomization::MakeInstance(SharedThis(this));
		})
	);
	_shadersConfigView->SetStructureData(_shadersConfigUI);
	_shadersConfigView->GetOnFinishedChangingPropertiesDelegate().AddSP(this, &SCyberGafferWindowContent::OnShaderConfigPropertiesChanged);
	_shadersConfigView->GetDetailsView()->ForceRefresh();
	
	const float verticalSlotPadding = 4.0f;

	FCyberGafferWindowStyle::Initialize();
	FCyberGafferWindowStyle::ReloadTextures();
	const FSlateBrush* cyberGafferLogo = FCyberGafferWindowStyle::Get().GetBrush("CyberGafferWindow.CyberGafferLogoWithText");

	ChildSlot[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() -> int32 {
				return this->_isTempScene;
			})
			+ SWidgetSwitcher::Slot()
			[
				SNullWidget::NullWidget
			]
			+ SWidgetSwitcher::Slot()
			[
				SNew(SWarningOrErrorBox)
				.Padding(verticalSlotPadding)
				.MessageStyle(EMessageStyle::Warning)
				.Message(LOCTEXT("UnsavedMapText",
				                 "The current map is unsaved, please, save it, otherwise your CyberGaffer settings will be not saved"))
			]
		]
		+ SScrollBox::Slot()
		[
			_automationSettingsView->GetWidget().ToSharedRef()
		]
		+ SScrollBox::Slot()
		[
			_currentSceneSettingsView->GetWidget().ToSharedRef()
		]
		+ SScrollBox::Slot()
		[
			_shadersConfigView->GetWidget().ToSharedRef()
		]
		+ SScrollBox::Slot()
		.FillSize(1.0f)
		[
			SNew(SBox)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Center)
			.Padding(10.0f)
			[
				SNew(SImage)
				.Image(cyberGafferLogo)
			]
		]
	];
}

SCyberGafferWindowContent::~SCyberGafferWindowContent() { }

FReply SCyberGafferWindowContent::OnExecuteAutomationClicked() {
	const FCyberGafferAutomationSettings* automation = _automationSettings->Cast<FCyberGafferAutomationSettings>();

	const bool addCyberGafferSceneCapture = automation->AddCyberGafferSceneCapture;
	const bool configurePostProcessVolume = automation->ConfigurePostProcessVolume;
	const bool createNewLinearPostProcessMaterial = automation->CreateNewLinearPostProcessMaterial;
	const bool createNewColorGradingPostProcessMaterial = automation->CreateNewColorGradingPostProcessMaterial;
	const bool createOCIOConfig = automation->CreateOCIOConfig;

	if (createNewLinearPostProcessMaterial) {
		CreatePostProcessMaterialInstance(PostProcessMaterialType::Linear);
	}

	if (createNewColorGradingPostProcessMaterial) {
		CreatePostProcessMaterialInstance(PostProcessMaterialType::ColorGrading);
	}

	if (configurePostProcessVolume) {
		if (_postProcessVolume == nullptr) {
			UWorld* world = GEditor->GetEditorWorldContext().World();
			if (world != nullptr) {
				// TODO: Currently spawn probe in persistent level
				const FVector location = FVector::ZeroVector;
				const FRotator rotation = FRotator::ZeroRotator;
				FActorSpawnParameters params;
				params.Name = MakeUniqueObjectName(world, APostProcessVolume::StaticClass(), FName(TEXT("CyberGafferPostProcessVolume")));
				CYBERGAFFER_LOG(Log, TEXT("New PPV name: %s"), *params.Name.ToString());
				AActor* postProcessVolumeActor = world->SpawnActor(APostProcessVolume::StaticClass(), &location, &rotation, params);
				_postProcessVolume = Cast<APostProcessVolume>(postProcessVolumeActor);
			} else {
				CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: world is null"));
			}
		}

		if (_postProcessVolume != nullptr) {
			_postProcessVolume->bEnabled = true;
			_postProcessVolume->bUnbound = true;
			if (_linearPostProcessMaterial.IsValid() && _linearPostProcessMaterial.Get() != nullptr) {
				_postProcessVolume->AddOrUpdateBlendable(_linearPostProcessMaterial.Get(), 1.0f);
			}
			if (_colorGradingPostProcessMaterial.IsValid() && _colorGradingPostProcessMaterial.Get() != nullptr) {
				_postProcessVolume->AddOrUpdateBlendable(_colorGradingPostProcessMaterial.Get(), 1.0f);
			}
		}
	}

	if (addCyberGafferSceneCapture) {
		if (_cyberGafferSceneCapture == nullptr) {
			UWorld* world = GEditor->GetEditorWorldContext().World();
			if (world != nullptr) {
				// TODO: Currently spawn probe in persistent level
				const FVector location = FVector::ZeroVector;
				const FRotator rotation = FRotator::ZeroRotator;
				FActorSpawnParameters params;
				params.Name = MakeUniqueObjectName(world, ACyberGafferSceneCapture::StaticClass(), FName(TEXT("CyberGafferSceneCapture")));
				CYBERGAFFER_LOG(Log, TEXT("New CGSC name: %s"), *params.Name.ToString());
				_cyberGafferSceneCapture = Cast<ACyberGafferSceneCapture>(world->SpawnActor(ACyberGafferSceneCapture::StaticClass(), &location, &rotation, params));

				_cyberGafferSceneCaptureComponent = FindCyberGafferSceneCaptureComponent();
				_cyberGafferSceneCaptureComponent->GlobalPostProcessVolume = _postProcessVolume;
				if (_colorGradingPostProcessMaterial.IsValid()) {
					_cyberGafferSceneCaptureComponent->ColorGradingPostProcessMaterial = _colorGradingPostProcessMaterial.Get();
				}
			} else {
				CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: world is null"));
			}
		}
	}

	if (createOCIOConfig) {
		IAssetTools& assetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		const auto cyberGafferProjectContentDir = FString("/Game/CyberGaffer");
		const FString assetName = "CyberGafferOpenColorIOConfig";

		
		UObject* asset = assetTools.CreateAsset(assetName, cyberGafferProjectContentDir, UOpenColorIOConfiguration::StaticClass(), nullptr);
		if (asset != nullptr) {
			UOpenColorIOConfiguration* config = Cast<UOpenColorIOConfiguration>(asset);
			if (config != nullptr) {
				IModuleInterface* generalModuleInterface = FModuleManager::Get().GetModule(TEXT("CyberGafferWindow"));
				FCyberGafferWindowModule* moduleInterface = reinterpret_cast<FCyberGafferWindowModule*>(generalModuleInterface);
				const bool isEnginePlugin = moduleInterface->IsEnginePlugin();
				
				FFilePath assetConfigPath;
				if (isEnginePlugin) {
					assetConfigPath.FilePath = "{Engine}/Plugins/Marketplace/CyberGaffer/Content/OCIO/OpenColorIO.ocio";
				} else {
					assetConfigPath.FilePath = "Plugins/CyberGaffer/Content/OCIO/OpenColorIO.ocio";
				}
				
				config->ConfigurationFile = assetConfigPath;
				config->ReloadExistingColorspaces();

				TArray<FOpenColorIOColorSpace> desiredColorSpaces;
				desiredColorSpaces.Reserve(2);
				
				FOpenColorIOWrapperConfig* wrapper = config->GetConfigWrapper();
				int32 spacesCount = wrapper->GetNumColorSpaces();
				
				for (int32 i = 0; i < spacesCount; ++i) {
					FString colorSpaceName = wrapper->GetColorSpaceName(i);
					CYBERGAFFER_LOG(Warning, TEXT("Color space name: %s"), *colorSpaceName);
					if (colorSpaceName != "Linear" && colorSpaceName != "sRGB") {
						continue;
					}
					
					FOpenColorIOColorSpace colorSpace;
					colorSpace.ColorSpaceIndex = i;
					colorSpace.ColorSpaceName = colorSpaceName;
					colorSpace.FamilyName = wrapper->GetColorSpaceFamilyName(*colorSpace.ColorSpaceName);

					desiredColorSpaces.Add(colorSpace);
					CYBERGAFFER_LOG(Warning, TEXT("Add color space: %s"), *colorSpaceName);
				}
				
				config->DesiredColorSpaces = desiredColorSpaces;

				// Save changes
				TArray<UPackage*> packagesToSave;
				config->PostEditChange();
				config->MarkPackageDirty();
				packagesToSave.Add(config->GetOutermost());

				FEditorFileUtils::PromptForCheckoutAndSave(packagesToSave, true, false);
			}
		}
	}
	_currentSceneSettings->LinearPostProcessMaterial = _linearPostProcessMaterial.Get();
	_currentSceneSettings->ColorGradingPostProcessMaterial = _colorGradingPostProcessMaterial.Get();
	_currentSceneSettings->PostProcessVolume = _postProcessVolume;
	_currentSceneSettings->CyberGafferSceneCapture = _cyberGafferSceneCapture;

	CastChecked<UScriptStruct>(_currentSceneSettingsUI->GetStruct())->CopyScriptStruct(_currentSceneSettingsUI->GetStructMemory(), _currentSceneSettings);

	SerializeSettings();

	return FReply::Handled();
}

void SCyberGafferWindowContent::LoadSerializedSettings() {
	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::LoadSerializedSettings"));

	const FString filePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("CyberGafferProjectSettings.data"));

	TArray<uint8> data;
	if (!FFileHelper::LoadFileToArray(data, *filePath)) {
		CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::LoadSerializedSettings: file does not exists"));
		return;
	}
	
	FMemoryReader reader(data, true);
	FObjectAndNameAsStringProxyArchive proxyArchive(reader, true);
	const int32 archiveVersion = proxyArchive.CustomVer(FCyberGafferSettingsVersion::GUID);

	// TODO: If we update settings object structure, implement what should we do with old version
	if (archiveVersion < FCyberGafferSettingsVersion::LatestVersion) {
		CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::LoadSerializedSettings: Settings for this project have been saved using previous version"));
		return;
	}
	
	UCyberGafferProjectSettings* settings = NewObject<UCyberGafferProjectSettings>();
	settings->Serialize(proxyArchive);

	_projectSettings = TStrongObjectPtr<UCyberGafferProjectSettings>(settings);

	proxyArchive.Flush();
	proxyArchive.Close();
}

void SCyberGafferWindowContent::SerializeSettings() {
	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::SerializeSettings"));

	if (!_projectSettings.IsValid() || _projectSettings.Get() == nullptr) {
		CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::SerializeSettings: settings not valid"));
	}

	const FString filePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("CyberGafferProjectSettings.data"));
	FArchive* archive = IFileManager::Get().CreateFileWriter(*filePath);
	FObjectAndNameAsStringProxyArchive proxyArchive(*archive, false);
	_projectSettings->Serialize(proxyArchive);

	proxyArchive.Flush();
	proxyArchive.Close();
}

void SCyberGafferWindowContent::OnScenePropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent) {
	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnScenePropertiesChanged"));

	CastChecked<UScriptStruct>(_currentSceneSettingsUI->GetStruct())->CopyScriptStruct(_currentSceneSettings, _currentSceneSettingsUI->GetStructMemory());

	_linearPostProcessMaterial = _currentSceneSettings->LinearPostProcessMaterial;
	_colorGradingPostProcessMaterial = _currentSceneSettings->ColorGradingPostProcessMaterial;
	_postProcessVolume = _currentSceneSettings->PostProcessVolume;
	_cyberGafferSceneCapture = _currentSceneSettings->CyberGafferSceneCapture;
	_cyberGafferSceneCaptureComponent = FindCyberGafferSceneCaptureComponent();

	SerializeSettings();
}

void SCyberGafferWindowContent::OnShaderConfigPropertiesChanged(const FPropertyChangedEvent& PropertyChangedEvent) {
	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnShaderConfigPropertiesChanged"));
	
	const auto propName = PropertyChangedEvent.GetPropertyName();
	CYBERGAFFER_LOG(Warning, TEXT("PROP NAME: %s"), *propName.ToString());

	CastChecked<UScriptStruct>(_shadersConfigUI->GetStruct())->CopyScriptStruct(&_projectSettings.Get()->ShadersConfig, _shadersConfigUI->GetStructMemory());
	SerializeSettings();
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

	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::LoadMaterialUsingPath: failed to load asset at path %s"),
	                *path);
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
	_isTempScene = FPackageName::IsTempPackage(worldPackageName);

	// FPackageName::Name
	// CYBERGAFFER_LOG(Warning, TEXT(" SCyberGafferWindowContent::GetCurrentSceneName: outermost file path: %s, temp: %i"), *mapName, isTemp);
	if (_isTempScene) {
		if (!_tempSceneSavedDelegateHandle.IsValid()) {
			_tempSceneSavedDelegateHandle = FEditorDelegates::PostSaveWorldWithContext.AddRaw(
				this, &SCyberGafferWindowContent::OnTempSceneSaved);
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
	CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::GetCurrentSceneName: package name: %s"),
	                *shortPackageName);

	return shortPackageName;
}

void SCyberGafferWindowContent::OnSceneChanged(const FString& filename, bool asTemplate) {
	const TOptional<FString> sceneName = GetCurrentSceneName();
	if (sceneName.IsSet()) {
		const auto sceneSettings = _projectSettings->GetSettingsForScene(sceneName.GetValue());
		if (sceneSettings && sceneSettings.GetValue() != nullptr) {
			_currentSceneSettings = sceneSettings.GetValue();
			
			_linearPostProcessMaterial = _currentSceneSettings->LinearPostProcessMaterial;
			_colorGradingPostProcessMaterial = _currentSceneSettings->ColorGradingPostProcessMaterial;

			_postProcessVolume = _currentSceneSettings->PostProcessVolume;

			_cyberGafferSceneCapture = _currentSceneSettings->CyberGafferSceneCapture;
			if (_cyberGafferSceneCapture) {
				_cyberGafferSceneCaptureComponent = FindCyberGafferSceneCaptureComponent();
			}
		} else {
			auto& newSettings = _projectSettings->ScenesSettings.Add(sceneName.GetValue(), FCyberGafferSceneSettings());
			_currentSceneSettings = _projectSettings->GetSettingsForScene(sceneName.GetValue()).GetValue();
			
			_linearPostProcessMaterial = nullptr;
			_colorGradingPostProcessMaterial = nullptr;
			_postProcessVolume = nullptr;

			SerializeSettings();
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
			initialParentPath =	"Script/Engine.Material'/CyberGaffer/Materials/CyberGafferLinearPostProcess.CyberGafferLinearPostProcess'";
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

	// const FString packagePath = UPackageTools::SanitizePackageName(cyberGafferProjectContentDir + TEXT("/") + newAssetName);

	const FString newAssetPath = FPaths::Combine(FPaths::ProjectContentDir(), "CyberGaffer", *newAssetName) + ".uasset";
	if (FPaths::FileExists(newAssetPath)) {
		FString existingMaterialInstancePath;
		switch (type) {
			case Linear: {
				existingMaterialInstancePath= FString::Printf(
					TEXT("/Game/CyberGaffer/CyberGafferLinearPostProcess_%s.CyberGafferLinearPostProcess_%s"),
					*sceneName.GetValue(),
					*sceneName.GetValue()
				);
				break;
			}
			case ColorGrading: {
				existingMaterialInstancePath= FString::Printf(
					TEXT("/Game/CyberGaffer/CyberGafferColorGradingPostProcess_%s.CyberGafferColorGradingPostProcess_%s"),
					*sceneName.GetValue(),
					*sceneName.GetValue()
				);
				break;
			}
		}
		UMaterialInstance* existingMaterialInstance = Cast<UMaterialInstance>(
			StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *existingMaterialInstancePath)
		);
		switch (type) {
			case Linear: {
				_linearPostProcessMaterial = existingMaterialInstance;
				break;
			}
			case ColorGrading: {
				_colorGradingPostProcessMaterial = existingMaterialInstance;
				break;
			}
		}
		
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FileExists_Text", "Material instance already created for this scene"));
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
			break;
		}
		case ColorGrading: {
			_colorGradingPostProcessMaterial = newMaterail;
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

FString SCyberGafferWindowContent::GetColorGradingPostProcessMaterialPath() const {
	if (_colorGradingPostProcessMaterial.IsValid()) {
		return _colorGradingPostProcessMaterial->GetPathName();
	}

	return FString();
}

TOptional<float> SCyberGafferWindowContent::GetExposureCompensation() const {
	if (!IsColorGradingPostProcessMaterialValid()) {
		return 0.0f;
	}

	FMemoryImageMaterialParameterInfo parameterInfo(TEXT("Expose Compensation"));
	FMaterialParameterMetadata metadata;
	const bool callResult = _colorGradingPostProcessMaterial->GetParameterValue(
		EMaterialParameterType::Scalar, parameterInfo, metadata);

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

void SCyberGafferWindowContent::OnExposureCompensationValueCommited(const float newValue,
                                                                    ETextCommit::Type commitType) {
	OnExposureCompensationValueChanged(newValue);
	SaveMaterialChanges(_colorGradingPostProcessMaterial.Get());
}

FLinearColor SCyberGafferWindowContent::GetColorGradingColor() const {
	if (!IsColorGradingPostProcessMaterialValid()) {
		return FLinearColor::White;
	}

	FMemoryImageMaterialParameterInfo parameterInfo(TEXT("Multiplier"));
	FMaterialParameterMetadata metadata;
	const bool callResult = _colorGradingPostProcessMaterial->GetParameterValue(
		EMaterialParameterType::Vector, parameterInfo, metadata);

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

FString SCyberGafferWindowContent::GetPostProcessVolumePath() const {
	return _postProcessVolume == nullptr ? "" : _postProcessVolume->GetPathName();
}

void SCyberGafferWindowContent::OnPostProcessVolumePathChanged(const FAssetData& assetData) {
	const auto objPath = assetData.GetSoftObjectPath();
	const auto obj = objPath.ResolveObject();

	if (obj == nullptr) {
		_postProcessVolume = nullptr;
	} else {
		_postProcessVolume = Cast<APostProcessVolume>(obj);
	}
}


bool SCyberGafferWindowContent::IsCyberGafferSceneCaptureComponentValid() const {
	return _cyberGafferSceneCaptureComponent != nullptr;
}

UCyberGafferSceneCaptureComponent2D* SCyberGafferWindowContent::FindCyberGafferSceneCaptureComponent() const {
	if (_cyberGafferSceneCapture == nullptr) {
		return nullptr;
	}

	TArray<UCyberGafferSceneCaptureComponent2D*> components;
	_cyberGafferSceneCapture->GetComponents(components);
	if (components.Num() == 0) {
		return nullptr;
	}
	UCyberGafferSceneCaptureComponent2D* sceneCaptureComponent = components[0];
	if (sceneCaptureComponent == nullptr) {
		return nullptr;
	}

	return sceneCaptureComponent;
}

FString SCyberGafferWindowContent::GetCyberGafferSceneCapturePath() const {
	// return _cyberGafferSceneCapturePath;
	return _cyberGafferSceneCapture == nullptr ? "" : _cyberGafferSceneCapture->GetPathName();
}

void SCyberGafferWindowContent::OnCyberGafferSceneCaptureChanged(const FAssetData& assetData) {
	auto objPath = assetData.GetSoftObjectPath();
	auto obj = objPath.ResolveObject();

	if (obj == nullptr) {
		_cyberGafferSceneCapture = nullptr;
		_cyberGafferSceneCaptureComponent = nullptr;
	} else {
		_cyberGafferSceneCapture = Cast<ACyberGafferSceneCapture>(obj);
		_cyberGafferSceneCaptureComponent = FindCyberGafferSceneCaptureComponent();
	}
}

TOptional<float> SCyberGafferWindowContent::GetLumenCacheResolution() const {
	if (_cyberGafferSceneCaptureComponent == nullptr) {
		return TOptional<float>();
	}

	return _cyberGafferSceneCaptureComponent->PostProcessSettings.LumenSurfaceCacheResolution;
}

void SCyberGafferWindowContent::OnLumenCacheResolutionValueChanged(float value) {
	if (_cyberGafferSceneCaptureComponent == nullptr) {
		return;
	}

	_cyberGafferSceneCaptureComponent->PostProcessSettings.LumenSurfaceCacheResolution = value;
}

FText SCyberGafferWindowContent::GetShadersIncludePath() const {
	if (!_projectSettings) {
		return FText();
	}
	
	return FText::FromString(_projectSettings->ShadersConfig.ShadersIncludePath.Path);
}

void SCyberGafferWindowContent::OnShadersIncludePathCommitted() {
	if (!_projectSettings) {
		return;
	}

	const FString newPath = _projectSettings->ShadersConfig.ShadersIncludePath.Path;

	if (!FPaths::DirectoryExists(newPath)) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnShadersIncludePathCommitted: invalid path"));
		return;
	}

	const FString shadersDir = FModuleManager::Get().GetModuleChecked<FCyberGafferModule>("CyberGaffer").GetShaderDirectory();
	FString shadersIncludePath = FPaths::Combine(shadersDir, "Include");
	shadersIncludePath = shadersIncludePath.Replace(TEXT("/"), TEXT("\\"));
	if (FPaths::DirectoryExists(shadersIncludePath)) {
		const FString command = FString::Printf(TEXT("/c rd \"%s\""), *shadersIncludePath);
		FProcHandle result = FPlatformProcess::CreateProc(
			TEXT("cmd.exe"), *command, false, true, false, nullptr, 0, nullptr, nullptr, nullptr);

		while (FPlatformProcess::IsProcRunning(result)) {
			FPlatformProcess::Sleep(0.1f);
		}
	}

	if (FPaths::DirectoryExists(shadersIncludePath)) {
		CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnShadersIncludePathCommitted: failed to delete junction"));
		return;
	}

	const FString command = FString::Printf(
		TEXT("/c mklink /j \"%s\" \"%s\""), *shadersIncludePath, *newPath.Replace(TEXT("/"), TEXT("\\")));
	FPlatformProcess::CreateProc(TEXT("cmd.exe"), *command, false, true, false, nullptr, 0, nullptr, nullptr, nullptr);

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

		FLevelEditorActionCallbacks::ExecuteExecCommand(
			FString::Printf(
				TEXT("RECOMPILESHADERS MATERIAL %s"), *_linearPostProcessMaterial->GetMaterial()->GetName()));
		FLevelEditorActionCallbacks::ExecuteExecCommand(
			FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_linearPostProcessMaterial->GetName()));
	}
	if (_colorGradingPostProcessMaterial != nullptr) {
		materials.Add(_colorGradingPostProcessMaterial->GetMaterial());
		materials.Add(_colorGradingPostProcessMaterial.Get());

		FLevelEditorActionCallbacks::ExecuteExecCommand(FString::Printf(
			TEXT("RECOMPILESHADERS MATERIAL %s"), *_colorGradingPostProcessMaterial->GetMaterial()->GetName()));
		FLevelEditorActionCallbacks::ExecuteExecCommand(
			FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_colorGradingPostProcessMaterial->GetName()));
	}

	SaveMaterialsChanges(materials);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
