#include "CyberGafferWindowContent.h"

#include "CyberGaffer.h"
#include "CyberGafferWindow.h"
#include "CyberGafferLog.h"
#include "CyberGafferWindowStyle.h"
#include "CyberGafferSettingsVersion.h"
#include "CyberGafferSceneCaptureComponent2D.h"
#include "CyberGafferWindowAutomationCustomization.h"
#include "CyberGafferWindowSceneControlsCustomization.h"
// #include "CyberGafferWindowShadersConfigCustomization.h"

#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#include "IStructureDetailsView.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "SWarningOrErrorBox.h"
#include "Dialogs/Dialogs.h"
#include "Layout/Margin.h"
#include "Styling/AppStyle.h"

#include "UObject/ConstructorHelpers.h"

#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

#include "Editor/UnrealEd/Public/FileHelpers.h"

#include "Editor/LevelEditor/Public/LevelEditorActions.h"
#include "AssetToolsModule.h"
#include "IOpenColorIOModule.h"
#include "LevelEditorViewport.h"

#include "OpenColorIOConfiguration.h"
#include "OpenColorIODisplayManager.h"
#include "OpenColorIOWrapper.h"

#include "ScopedTransaction.h"

#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
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
	_automationSettingsView->SetStructureData(_automationSettings);
	_automationSettingsView->GetDetailsView()->RegisterInstancedCustomPropertyLayout(
		FCyberGafferAutomationSettings::StaticStruct(),
		FOnGetDetailCustomizationInstance::CreateLambda([this] {
			return FCyberGafferWindowAutomationCustomization::MakeInstance(SharedThis(this));
		})
	);
	
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
	
	// _shadersConfigUI = MakeShared<FStructOnScope>(FCyberGafferShadersConfig::StaticStruct());
	// CastChecked<UScriptStruct>(_shadersConfigUI->GetStruct())->CopyScriptStruct(_shadersConfigUI->GetStructMemory(), &_projectSettings.Get()->ShadersConfig);
	// _shadersConfigView = propertyEditor.CreateStructureDetailView(detailsViewArgs, structureDetailsViewArgs, TSharedPtr<FStructOnScope>());
	// _shadersConfigView->GetDetailsView()->RegisterInstancedCustomPropertyLayout(
	// 	FCyberGafferShadersConfig::StaticStruct(),
	// 	FOnGetDetailCustomizationInstance::CreateLambda([this] {
	// 		return FCyberGafferWindowShadersConfigCustomization::MakeInstance(SharedThis(this));
	// 	})
	// );
	// _shadersConfigView->SetStructureData(_shadersConfigUI);
	// _shadersConfigView->GetOnFinishedChangingPropertiesDelegate().AddSP(this, &SCyberGafferWindowContent::OnShaderConfigPropertiesChanged);
	// _shadersConfigView->GetDetailsView()->ForceRefresh();
	
	const float verticalSlotPadding = 4.0f;

	FCyberGafferWindowStyle::Initialize();
	FCyberGafferWindowStyle::ReloadTextures();
	const FSlateBrush* cyberGafferLogo = FCyberGafferWindowStyle::Get().GetBrush("CyberGafferWindow.CyberGafferLogoWithText");

	ChildSlot[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		// .FillSize(1.0f)
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

		// Unsaved map warning
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
				.Message(
					LOCTEXT(
						"UnsavedMap_Text",
						"The current map is unsaved, please, save it, otherwise your CyberGaffer settings will be not saved"
						)
					)
			]
		]

		// No CyberGafferSceneCapture warning
		+ SScrollBox::Slot()
		[
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() -> int32 {
				return this->_cyberGafferSceneCapture == nullptr;
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
				.Message(
					LOCTEXT(
						"NoCyberGafferSceneCapture_Text",
						"No Cyber Gaffer Scene Capture found in scene"
						)
					)
			]
		]


		// No PPV warning
		+ SScrollBox::Slot()
		[
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() -> int32 {
				return this->_postProcessVolume == nullptr;
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
				.Message(
					LOCTEXT(
						"NoPPV_Text",
						"No unbound Post Process Volume found in scene"
						)
					)
			]
		]
		
		+ SScrollBox::Slot()
		[
			_currentSceneSettingsView->GetWidget().ToSharedRef()
		]
		+ SScrollBox::Slot()
		[
			_automationSettingsView->GetWidget().ToSharedRef()
		]
		// + SScrollBox::Slot()
		// [
		// 	_shadersConfigView->GetWidget().ToSharedRef()
		// ]
		
	];
}

SCyberGafferWindowContent::~SCyberGafferWindowContent() { }

FReply SCyberGafferWindowContent::OnExecuteAutomationClicked() {
	const FCyberGafferAutomationSettings* automation = _automationSettings->Cast<FCyberGafferAutomationSettings>();

	const bool addCyberGafferSceneCapture = automation->AddCyberGafferSceneCapture;
	const bool configurePostProcessVolume = automation->ConfigurePostProcessVolume;
	const bool createNewPostProcessMaterial = automation->CreateNewPostProcessMaterial;
	const bool createOCIOConfig = automation->CreateOCIOConfig;

	if (createNewPostProcessMaterial) {
		CreatePostProcessMaterialInstance();
	}

	if (configurePostProcessVolume) {
		if (_postProcessVolume == nullptr) {
			UWorld* world = GEditor->GetEditorWorldContext().World();
			if (world != nullptr) {
				_postProcessVolume = FindPostProcessVolume(world);
				if (_postProcessVolume == nullptr) {
					UClass* actorClass = APostProcessVolume::StaticClass();
					UActorFactory* factory = GEditor->FindActorFactoryForActorClass(actorClass);
					if (factory != nullptr) {
						AActor* postProcessVolumeActor = GEditor->UseActorFactory(factory, FAssetData(actorClass), &FTransform::Identity);
						_postProcessVolume = Cast<APostProcessVolume>(postProcessVolumeActor);
					} else {
						CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: can't find factory for APostProcessVolume"));
					}
				}
			} else {
				CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: world is null"));
			}
		}

		if (_postProcessVolume != nullptr) {
			_postProcessVolume->bEnabled = true;
			_postProcessVolume->bUnbound = true;
			if (_postProcessMaterial.IsValid() && _postProcessMaterial.Get() != nullptr) {
				_postProcessVolume->AddOrUpdateBlendable(_postProcessMaterial.Get(), 1.0f);
			}
		}
	}

	if (addCyberGafferSceneCapture) {
		if (_cyberGafferSceneCapture == nullptr) {
			UWorld* world = GEditor->GetEditorWorldContext().World();
			if (world != nullptr) {
				_cyberGafferSceneCapture = FindCyberGafferSceneCapture(world);
				if (_cyberGafferSceneCapture == nullptr) {
					UClass* actorClass = ACyberGafferSceneCapture::StaticClass();
					UActorFactory* factory = GEditor->FindActorFactoryForActorClass(actorClass);
					if (factory != nullptr) {
						AActor* cyberGafferSceneCaptureActor = GEditor->UseActorFactory(factory, FAssetData(actorClass), &FTransform::Identity);
						_cyberGafferSceneCapture = Cast<ACyberGafferSceneCapture>(cyberGafferSceneCaptureActor);
					} else {
						GUnrealEd->Exec(world, *FString::Printf( TEXT("ACTOR ADD CLASS=%s"), *actorClass->GetName()));
						_cyberGafferSceneCapture = FindCyberGafferSceneCapture(world);
						if (_cyberGafferSceneCapture != nullptr && GCurrentLevelEditingViewportClient) {
							GEditor->MoveActorInFrontOfCamera(*_cyberGafferSceneCapture,
								GCurrentLevelEditingViewportClient->GetViewLocation(),
								GCurrentLevelEditingViewportClient->GetViewRotation().Vector()
							);
						}
					}
				}
				_cyberGafferSceneCaptureComponent = FindCyberGafferSceneCaptureComponent();
			} else {
				CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: world is null"));
			}
		}
	}

	if (createOCIOConfig) {
		UOpenColorIOConfiguration* config;

		const auto cyberGafferProjectContentDir = FString("/Game/CyberGaffer");
		const FString assetName = "CyberGafferOpenColorIOConfig";

		FAssetRegistryModule& module = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		FAssetData assetData;
		FSoftObjectPath objectPath(FPaths::Combine(cyberGafferProjectContentDir, FString::Format(TEXT("{0}.{0}"), {assetName})));

		if (module.Get().TryGetAssetByObjectPath(objectPath, assetData) == UE::AssetRegistry::EExists::Exists) {
			CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: config already exists"));
			config = Cast<UOpenColorIOConfiguration>(assetData.GetAsset());
		} else {
			CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: config not exist, create it"));
			IAssetTools& assetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			UObject* asset = assetTools.CreateAsset(assetName, cyberGafferProjectContentDir, UOpenColorIOConfiguration::StaticClass(), nullptr);
			config = Cast<UOpenColorIOConfiguration>(asset);
		}
		
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

			FOpenColorIOColorSpace linearSpace;
			FOpenColorIOColorSpace srgbSpace;
			
			for (int32 i = 0; i < spacesCount; ++i) {
				FString colorSpaceName = wrapper->GetColorSpaceName(i);

				if (colorSpaceName != "Linear" && colorSpaceName != "sRGB") {
					continue;
				}
				
				FOpenColorIOColorSpace colorSpace;
				colorSpace.ColorSpaceIndex = i;
				colorSpace.ColorSpaceName = colorSpaceName;
				colorSpace.FamilyName = wrapper->GetColorSpaceFamilyName(*colorSpace.ColorSpaceName);

				if (colorSpaceName == "Linear") {
					linearSpace = colorSpace;
				}

				if (colorSpaceName == "sRGB") {
					srgbSpace = colorSpace;
				}

				desiredColorSpaces.Add(colorSpace);
			}

			FProperty* property = FindFProperty<FProperty>(UOpenColorIOConfiguration::StaticClass(), GET_MEMBER_NAME_CHECKED(UOpenColorIOConfiguration, DesiredColorSpaces));
			if (property != nullptr) {
				// Use EPropertyChangeType::ValueSet to add new transformations AND remove not used.
				config->DesiredColorSpaces = desiredColorSpaces;            
				FPropertyChangedEvent changeEvent(property, EPropertyChangeType::ValueSet);
				config->PostEditChangeProperty(changeEvent);
			} else {
				CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: failed to find property"));
			}

			// Save changes
			TArray<UPackage*> packagesToSave;
			config->PostEditChange();
			config->MarkPackageDirty();
			packagesToSave.Add(config->GetOutermost());
			FEditorFileUtils::PromptForCheckoutAndSave(packagesToSave, true, false);
			

			// Apply OCIO transformation to the active viewport
			FViewport* activeViewport = GEditor->GetActiveViewport();
			if (activeViewport != nullptr) {
				FOpenColorIOColorConversionSettings conversionSettings;
				conversionSettings.ConfigurationSource = config;
				conversionSettings.SourceColorSpace = linearSpace;
				conversionSettings.DestinationColorSpace = srgbSpace;
		
				FOpenColorIODisplayConfiguration newConfiguration;
				newConfiguration.bIsEnabled = true;
				newConfiguration.ColorConfiguration = conversionSettings;
		
				FOpenColorIODisplayConfiguration& currentConfiguration = IOpenColorIOModule::Get().GetDisplayManager().FindOrAddDisplayConfiguration(activeViewport->GetClient());
				currentConfiguration = newConfiguration;
				activeViewport->Invalidate();
			} else {
				CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::OnExecuteAutomationClicked: failed to get active viewport, please, apply OCIO transformation manually to the viewport"));
			}
		}
	}
	_currentSceneSettings->PostProcessMaterial = _postProcessMaterial.Get();

	CastChecked<UScriptStruct>(_currentSceneSettingsUI->GetStruct())->CopyScriptStruct(_currentSceneSettingsUI->GetStructMemory(), _currentSceneSettings);

	SerializeSettings();

	return FReply::Handled();
}

void SCyberGafferWindowContent::LoadSerializedSettings() {
	const FString filePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("CyberGafferProjectSettings.data"));

	TArray<uint8> data;
	if (!FFileHelper::LoadFileToArray(data, *filePath)) {
		CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::LoadSerializedSettings: file does not exists"));
		return;
	}
	
	FMemoryReader reader(data, true);
	FObjectAndNameAsStringProxyArchive proxyArchive(reader, true);
	const int32 archiveVersion = proxyArchive.CustomVer(FCyberGafferSettingsVersion::GUID);

	UCyberGafferProjectSettings* settings = NewObject<UCyberGafferProjectSettings>();

	// TODO: If we update settings object structure, implement what should we do with old version
	if (archiveVersion < FCyberGafferSettingsVersion::LatestVersion) {
		CYBERGAFFER_LOG(Error, TEXT("SCyberGafferWindowContent::LoadSerializedSettings: Settings for this project have been saved using previous version"));
	} else {
		settings->Serialize(proxyArchive);
	}

	_projectSettings = TStrongObjectPtr<UCyberGafferProjectSettings>(settings);

	proxyArchive.Flush();
	proxyArchive.Close();
}

void SCyberGafferWindowContent::SerializeSettings() {
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
	CastChecked<UScriptStruct>(_currentSceneSettingsUI->GetStruct())->CopyScriptStruct(_currentSceneSettings, _currentSceneSettingsUI->GetStructMemory());

	_postProcessMaterial = _currentSceneSettings->PostProcessMaterial;
	SerializeSettings();
}

// void SCyberGafferWindowContent::OnShaderConfigPropertiesChanged(const FPropertyChangedEvent& propertyChangedEvent) {
// 	CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::OnShaderConfigPropertiesChanged"));
//
// 	CastChecked<UScriptStruct>(_shadersConfigUI->GetStruct())->CopyScriptStruct(&_projectSettings.Get()->ShadersConfig, _shadersConfigUI->GetStructMemory());
// 	SerializeSettings();
// 	
// 	const auto propMemberName = propertyChangedEvent.GetMemberPropertyName();
// 	
// 	if (propMemberName.ToString() == FString("ShadersIncludePath")) {
// 		OnShadersIncludePathCommitted();
// 	}
// }

void SCyberGafferWindowContent::OnParentTabClosed(TSharedRef<SDockTab> parentTab) {
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
	_isTempScene = FPackageName::IsTempPackage(worldPackageName);
	
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

	return shortPackageName;
}

void SCyberGafferWindowContent::OnSceneChanged(const FString& filename, bool asTemplate) {
	const TOptional<FString> sceneName = GetCurrentSceneName();
	if (sceneName.IsSet()) {
		const auto sceneSettings = _projectSettings->GetSettingsForScene(sceneName.GetValue());
		if (sceneSettings && sceneSettings.GetValue() != nullptr) {
			_currentSceneSettings = sceneSettings.GetValue();
			
			_postProcessMaterial = _currentSceneSettings->PostProcessMaterial;
		} else {
			auto& newSettings = _projectSettings->ScenesSettings.Add(sceneName.GetValue(), FCyberGafferSceneSettings());
			_currentSceneSettings = _projectSettings->GetSettingsForScene(sceneName.GetValue()).GetValue();
			
			_postProcessMaterial = nullptr;

			SerializeSettings();
		}
	}
	UWorld* world = GEditor->GetEditorWorldContext().World();
	if (world != nullptr) {
		_postProcessVolume = FindPostProcessVolume(world);
		_cyberGafferSceneCapture = FindCyberGafferSceneCapture(world);
		if (_cyberGafferSceneCapture.IsValid()) {
			_cyberGafferSceneCaptureComponent = FindCyberGafferSceneCaptureComponent();
		}
	} else {
		CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::OnSceneChanged: world is null"));
	}
}

void SCyberGafferWindowContent::OnTempSceneSaved(UWorld* world, FObjectPostSaveContext postSaveContext) {
	OnSceneChanged(FString(), false);
}

FReply SCyberGafferWindowContent::CreatePostProcessMaterialInstance() {
	IAssetTools& assetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UMaterialInstanceConstantFactoryNew* materialFactory = NewObject<UMaterialInstanceConstantFactoryNew>();

	const FString initialParentPath = "Script/Engine.Material'/CyberGaffer/Materials/CyberGafferPostProcess.CyberGafferPostProcess'";

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
		const FString existingMaterialInstancePath = FString::Printf(TEXT("/Game/CyberGaffer/%s"), *newAssetName);
		
		UMaterialInstance* existingMaterialInstance = Cast<UMaterialInstance>(
			StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *existingMaterialInstancePath)
		);
		
		_postProcessMaterial = existingMaterialInstance;
		
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FileExists_Text", "Material instance already created for this scene"));
		return FReply::Handled();
	}

	materialFactory->InitialParent = initialParent;
	UObject* asset = assetTools.CreateAsset(newAssetName, cyberGafferProjectContentDir, UMaterialInstanceConstant::StaticClass(), materialFactory);
	if (asset == nullptr) {
		return FReply::Unhandled();
	}

	auto newMaterial = Cast<UMaterialInstanceConstant>(asset);
	_postProcessMaterial = newMaterial;
	
	SaveMaterialChanges(newMaterial);

	return FReply::Handled();
}

FString SCyberGafferWindowContent::GetLinearPostProcessMaterialPath() const {
	if (_postProcessMaterial.IsValid()) {
		return _postProcessMaterial->GetPathName();
	}

	return FString();
}

APostProcessVolume* SCyberGafferWindowContent::FindPostProcessVolume(UWorld* world) {
	APostProcessVolume* result = nullptr;
	
	TArray<AActor*> postProcessVolumes;
	UGameplayStatics::GetAllActorsOfClass(world, APostProcessVolume::StaticClass(), postProcessVolumes);
	if (postProcessVolumes.Num() == 0) {
		return result;
	}

	UObject* postProcessMaterialObject = nullptr;
	if (_postProcessMaterial.IsValid()) {
		postProcessMaterialObject = Cast<UObject>(_postProcessMaterial);
	} else {
		CYBERGAFFER_LOG(Warning, TEXT("SCyberGafferWindowContent::FindPostProcessVolume: post process material object is null"));
	}

	// Assume that the first unbound post process volume with post process material for this scene is what we need.
	// If no such PPV found, return just first unbound PPV.
	for (int32 i = 0; i < postProcessVolumes.Num(); ++i) {
		auto postProcessVolume = Cast<APostProcessVolume>(postProcessVolumes[i]);
		if (postProcessVolume) {
			if (!postProcessVolume->bUnbound) {
				continue;
			}

			if (postProcessMaterialObject == nullptr) {
				result = postProcessVolume;
				break;
			}
			
			if (result == nullptr) {
				result = postProcessVolume;
			}

			bool blendableFound = false;
			const FWeightedBlendables blendables = postProcessVolume->Settings.WeightedBlendables;
			for (auto blendable : blendables.Array) {
				if (blendable.Object && blendable.Object == postProcessMaterialObject) {
					blendableFound = true;
					result = postProcessVolume;
					break;
				}
			}

			if (blendableFound) {
				break;
			}
		}
	}
	if (result == nullptr) {
		CYBERGAFFER_LOG(
			Log,
			TEXT("UCyberGafferSceneCaptureComponent2D::FindPostProcessVolume: post process volume not found")
		);
	} else {
		CYBERGAFFER_LOG(
			Log,
			TEXT("UCyberGafferSceneCaptureComponent2D::FindPostProcessVolume: post process volume found, name: %s"),
			*result->GetName()
		);
	}
	
	return result;
}

ACyberGafferSceneCapture* SCyberGafferWindowContent::FindCyberGafferSceneCapture(UWorld* world) {
	ACyberGafferSceneCapture* result = nullptr;
	
	TArray<AActor*> sceneCaptures;
	UGameplayStatics::GetAllActorsOfClass(world, ACyberGafferSceneCapture::StaticClass(), sceneCaptures);
	if (sceneCaptures.Num() == 0) {
		return result;
	}

	if (sceneCaptures.Num() > 1) {
		CYBERGAFFER_LOG(
			Warning,
			TEXT("SCyberGafferWindowContent::FindSceneCapture: found multiple Cyber Gaffer Scene Captures actors in world. We expects only one.")
		);
	}

	for (AActor* sceneCapture : sceneCaptures) {
		if (!IsValidChecked(sceneCapture)) {
			continue;
		}
		result = Cast<ACyberGafferSceneCapture>(sceneCapture);
		break;
	}

	if (result == nullptr) {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::FindSceneCapture: Cyber Gaffer Scene Capture not found"));
	} else {
		CYBERGAFFER_LOG(Log, TEXT("SCyberGafferWindowContent::FindSceneCapture: Cyber Gaffer Scene Capture found, name: %s"), *result->GetName());
	}

	return result;
}

TOptional<float> SCyberGafferWindowContent::GetExposureCompensation() const {
	if (!_postProcessMaterial.IsValid()) {
		return 0.0f;
	}

	FMemoryImageMaterialParameterInfo parameterInfo(TEXT("Exposure Compensation"));
	FMaterialParameterMetadata metadata;
	const bool callResult = _postProcessMaterial->GetParameterValue(EMaterialParameterType::Scalar, parameterInfo, metadata);

	if (callResult) {
		return metadata.Value.AsScalar();
	}
	
	return 0.0f;
}

void SCyberGafferWindowContent::OnExposureCompensationValueChanged(float value) {
	if (!_postProcessMaterial.IsValid()) {
		return;
	}

	auto instanceConstant = Cast<UMaterialInstanceConstant>(_postProcessMaterial);
	if (instanceConstant) {
		FMaterialParameterInfo parameterInfo(TEXT("Exposure Compensation"));
		instanceConstant->SetScalarParameterValueEditorOnly(parameterInfo, value);
	}
}

void SCyberGafferWindowContent::OnExposureCompensationValueCommited(const float newValue, ETextCommit::Type commitType) {
	OnExposureCompensationValueChanged(newValue);
	SaveMaterialChanges(_postProcessMaterial.Get());
}

FLinearColor SCyberGafferWindowContent::GetMultiplierColor() const {
	if (!IsPostProcessMaterialValid()) {
		return FLinearColor::White;
	}

	FMemoryImageMaterialParameterInfo parameterInfo(TEXT("Multiplier"));
	FMaterialParameterMetadata metadata;
	const bool callResult = _postProcessMaterial->GetParameterValue(EMaterialParameterType::Vector, parameterInfo, metadata);

	if (callResult) {
		return metadata.Value.AsLinearColor();
	}

	return FLinearColor::White;
}

void SCyberGafferWindowContent::OnMultiplierColorChanged(FLinearColor color) {
	if (!IsPostProcessMaterialValid()) {
		return;
	}

	auto instanceConstant = Cast<UMaterialInstanceConstant>(_postProcessMaterial);
	FMaterialParameterInfo parameterInfo(TEXT("Multiplier"));
	if (instanceConstant) {
		instanceConstant->SetVectorParameterValueEditorOnly(parameterInfo, color);
	}
}

void SCyberGafferWindowContent::OnMultiplierColorCommited() {
	if (!IsPostProcessMaterialValid()) {
		return;
	}
	
	SaveMaterialChanges(_postProcessMaterial.Get());
}

bool SCyberGafferWindowContent::IsPostProcessMaterialValid() const {
	return _postProcessMaterial.IsValid();
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

TOptional<float> SCyberGafferWindowContent::GetLumenFinalGatherQuality() const {
	if (_cyberGafferSceneCaptureComponent == nullptr) {
		return TOptional<float>();
	}

	return _cyberGafferSceneCaptureComponent->PostProcessSettings.LumenFinalGatherQuality;
}

void SCyberGafferWindowContent::OnLumenFinalGatherQualityValueChanged(float value) {
	if (_cyberGafferSceneCaptureComponent == nullptr) {
		return;
	}
	
	FProperty* parentProp = FindFProperty<FProperty>(UCyberGafferSceneCaptureComponent2D::StaticClass(), GET_MEMBER_NAME_CHECKED(UCyberGafferSceneCaptureComponent2D, PostProcessSettings));
	_cyberGafferSceneCaptureComponent->PostProcessSettings.LumenFinalGatherQuality = value;
}

void SCyberGafferWindowContent::OnLumenFinalGatherQualityValueCommited(float value, ETextCommit::Type commitType) {
	if (_cyberGafferSceneCaptureComponent == nullptr) {
		return;
	}
	
	GEditor->BeginTransaction(LOCTEXT("LumenFinalGatherQualityUndo_Text", "Change Lumen Final Gather Quality"));
	_cyberGafferSceneCaptureComponent->Modify();
	FProperty* parentProp = FindFProperty<FProperty>(UCyberGafferSceneCaptureComponent2D::StaticClass(), GET_MEMBER_NAME_CHECKED(UCyberGafferSceneCaptureComponent2D, PostProcessSettings));
	_cyberGafferSceneCaptureComponent->PostProcessSettings.LumenFinalGatherQuality = value;
	GEditor->EndTransaction();
}

/*FText SCyberGafferWindowContent::GetShadersIncludePath() const {
	if (!_projectSettings) {
		return FText();
	}
	
	return FText::FromString(_projectSettings->ShadersConfig.ShadersIncludePath.Path);
}*/

/*void SCyberGafferWindowContent::OnShadersIncludePathCommitted() {
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
}*/

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

// FReply SCyberGafferWindowContent::RecompileShaders() {
// 	TArray<UMaterialInterface*> materials;
// 	if (_postProcessMaterial != nullptr) {
// 		materials.Add(_postProcessMaterial->GetMaterial());
// 		materials.Add(_postProcessMaterial.Get());
//
// 		FLevelEditorActionCallbacks::ExecuteExecCommand(
// 			FString::Printf(
// 				TEXT("RECOMPILESHADERS MATERIAL %s"), *_postProcessMaterial->GetMaterial()->GetName()));
// 		FLevelEditorActionCallbacks::ExecuteExecCommand(
// 			FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_postProcessMaterial->GetName()));
// 	}
// 	if (_colorGradingPostProcessMaterial != nullptr) {
// 		materials.Add(_colorGradingPostProcessMaterial->GetMaterial());
// 		materials.Add(_colorGradingPostProcessMaterial.Get());
//
// 		FLevelEditorActionCallbacks::ExecuteExecCommand(FString::Printf(
// 			TEXT("RECOMPILESHADERS MATERIAL %s"), *_colorGradingPostProcessMaterial->GetMaterial()->GetName()));
// 		FLevelEditorActionCallbacks::ExecuteExecCommand(
// 			FString::Printf(TEXT("RECOMPILESHADERS MATERIAL %s"), *_colorGradingPostProcessMaterial->GetName()));
// 	}
//
// 	SaveMaterialsChanges(materials);
//
// 	return FReply::Handled();
// }

#undef LOCTEXT_NAMESPACE
