#include "CyberGafferWindowSceneControlsCustomization.h"

#include "CyberGafferProjectSettings.h"
#include "CyberGafferLog.h"
#include "CyberGafferWindowContent.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Colors/SColorWheel.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

TSharedRef<IDetailCustomization> FCyberGafferWindowSceneControlsCustomization::MakeInstance(TSharedPtr<SCyberGafferWindowContent> content) {
	return MakeShareable(new FCyberGafferWindowSceneControlsCustomization(content));
}

FCyberGafferWindowSceneControlsCustomization::FCyberGafferWindowSceneControlsCustomization(
	TSharedPtr<SCyberGafferWindowContent> content) : _content(content) { }

void FCyberGafferWindowSceneControlsCustomization::CustomizeDetails(IDetailLayoutBuilder& detailLayout) {
	IDetailCategoryBuilder& controlsEdit = detailLayout.EditCategory(
		FName(TEXT("Controls")),
		FText::GetEmpty(),
		ECategoryPriority::Uncommon
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
			.IsEnabled(_content.ToSharedRef(), &SCyberGafferWindowContent::IsColorGradingPostProcessMaterialValid)
			.MinDesiredValueWidth(75.0f)
		];

	controlsEdit.AddCustomRow(FText::FromString(TEXT("Color Grading")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ColorGrading_Text", "Color Grading"))
		]
		.ValueContent()
		[
			SNew(SColorWheel)
			.SelectedColor(_content.ToSharedRef(), &SCyberGafferWindowContent::GetColorGradingColor)
			.OnValueChanged(_content.ToSharedRef(), &SCyberGafferWindowContent::OnColorGradingValueChanged)
			.OnMouseCaptureEnd(_content.ToSharedRef(), &SCyberGafferWindowContent::OnColorGradingCaptureEnd)
			.ToolTipText(LOCTEXT("ColorGrading_Text", "Color Grading"))
			.IsEnabled(_content.ToSharedRef(), &SCyberGafferWindowContent::IsColorGradingPostProcessMaterialValid)
		];

	controlsEdit.AddCustomRow(FText::FromString(TEXT("LumenCacheResolutionText")))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LumenCacheResolution_Text", "Lumen Cache Resolution"))
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
		.Value(_content.ToSharedRef(), &SCyberGafferWindowContent::GetLumenCacheResolution)
		.OnValueChanged(_content.ToSharedRef(), &SCyberGafferWindowContent::OnLumenCacheResolutionValueChanged)
		.ToolTipText(LOCTEXT("LumenCacheResolution_Text", "Lumen Cache Resolution"))
		.IsEnabled(_content.ToSharedRef(), &SCyberGafferWindowContent::IsCyberGafferSceneCaptureComponentValid)
		.MinDesiredValueWidth(75.0f)
	];
}

#undef LOCTEXT_NAMESPACE