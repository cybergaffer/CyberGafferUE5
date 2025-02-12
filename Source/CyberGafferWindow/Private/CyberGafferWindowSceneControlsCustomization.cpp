#include "CyberGafferWindowSceneControlsCustomization.h"

#include "CyberGafferProjectSettings.h"
#include "CyberGafferLog.h"
#include "CyberGafferWindowContent.h"

#include "ColorPicker.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Colors/SColorWheel.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Widgets/Colors/SSimpleGradient.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

TSharedRef<IDetailCustomization> FCyberGafferWindowSceneControlsCustomization::MakeInstance(TSharedPtr<SCyberGafferWindowContent> content) {
	return MakeShareable(new FCyberGafferWindowSceneControlsCustomization(content));
}

FCyberGafferWindowSceneControlsCustomization::FCyberGafferWindowSceneControlsCustomization(
	TSharedPtr<SCyberGafferWindowContent> content) : _content(content) { }

void FCyberGafferWindowSceneControlsCustomization::CustomizeDetails(IDetailLayoutBuilder& detailLayout) {
	IDetailCategoryBuilder& materialsEdit = detailLayout.EditCategory(FName(TEXT("Materials")));
	materialsEdit.InitiallyCollapsed(true);
	
	IDetailCategoryBuilder& controlsEdit = detailLayout.EditCategory(
		FName(TEXT("Controls")),
		FText::GetEmpty(),
		ECategoryPriority::Important
	);
	controlsEdit.AddCustomRow(FText::FromString(TEXT("Exposure Compensation")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ExposureCompensation_Text", "Exposure Compensation"))
			.Font(detailLayout.GetDetailFont())
		]
		.ValueContent()
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
			.Value(_content.ToSharedRef(), &SCyberGafferWindowContent::GetExposureCompensation)
			.OnValueChanged(_content.ToSharedRef(), &SCyberGafferWindowContent::OnExposureCompensationValueChanged)
			.OnValueCommitted(_content.ToSharedRef(), &SCyberGafferWindowContent::OnExposureCompensationValueCommited)
			.IsEnabled(_content.ToSharedRef(), &SCyberGafferWindowContent::IsPostProcessMaterialValid)
			.MinDesiredValueWidth(75.0f)
		];

	controlsEdit.AddCustomRow(FText::FromString(TEXT("Multiplier")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Multiplier_Text", "Multiplier"))
		]
		.ValueContent()
		[
			SNew(SCustomColorPicker)
			._targetColorAttribute(_content.ToSharedRef(), &SCyberGafferWindowContent::GetMultiplierColor)
			._onColorChanged(_content.ToSharedRef(), &SCyberGafferWindowContent::OnMultiplierColorChanged)
			._onColorCommited(_content.ToSharedRef(), &SCyberGafferWindowContent::OnMultiplierColorCommited)
			.IsEnabled(_content.ToSharedRef(), &SCyberGafferWindowContent::IsPostProcessMaterialValid)
		];

	
	controlsEdit.AddCustomRow(FText::FromString(TEXT("LumenFinalGatherQuality")))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LumenFinalGatherQuality_Text", "Lumen Final Gather Quality"))
	]
	.ValueContent()
	[
		SNew(SNumericEntryBox<float>)
		.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
		.AllowSpin(true)
		.MinValue(0.0f)
		.MaxValue(5.0f)
		.MinSliderValue(0.5)
		.MaxSliderValue(2.0)
		.Delta(0.01f)
		.LinearDeltaSensitivity(0.001f)
		.Value(_content.ToSharedRef(), &SCyberGafferWindowContent::GetLumenFinalGatherQuality)
		.OnValueChanged(_content.ToSharedRef(), &SCyberGafferWindowContent::OnLumenFinalGatherQualityValueChanged)
		.OnValueCommitted(_content.ToSharedRef(), &SCyberGafferWindowContent::OnLumenFinalGatherQualityValueCommited)
		.ToolTipText(LOCTEXT("LumenFinalGatherQuality_Text", "Lumen Final Gather Quality"))
		.IsEnabled(_content.ToSharedRef(), &SCyberGafferWindowContent::IsCyberGafferSceneCaptureComponentValid)
		.MinDesiredValueWidth(75.0f)
	];
}

#undef LOCTEXT_NAMESPACE
