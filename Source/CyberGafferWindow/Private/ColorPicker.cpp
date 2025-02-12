#include "ColorPicker.h"

#include "CyberGafferLog.h"

#include "Widgets/Colors/SSimpleGradient.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Colors/SColorWheel.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

const double SCustomColorPicker::MAX_ALLOWED_UPDATE_TIME = 0.1;

void SCustomColorPicker::Construct(const FArguments& inArgs) {
	_targetColorAttribute = inArgs.__targetColorAttribute;
	_currentColorHSV = _targetColorAttribute.Get().LinearRGBToHSV();
	_currentColorRGB = _targetColorAttribute.Get();
	_onColorChanged = inArgs.__onColorChanged;
	_onColorCommited = inArgs.__onColorCommited;

	_isInteractive = false;
	_perfIsTooSlowToUpdate = false;

	const FSlateFontInfo smallLayoutFont = FAppStyle::Get().GetFontStyle("ColorPicker.Font");

	this->ChildSlot
	[
		SNew(SVerticalBox)

		// Color wheel
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 4.0f)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SNew(SColorWheel)
				.SelectedColor(this, &SCustomColorPicker::GetCurrentColorHSV)
				.OnValueChanged(this, &SCustomColorPicker::HandleColorSpectrumValueChanged)
				.OnMouseCaptureBegin(this, &SCustomColorPicker::HandleInteractiveChangeBegin)
				.OnMouseCaptureEnd(this, &SCustomColorPicker::HandleInteractiveChangeEnd)
			]
		]

		// Saturation slider
		+SVerticalBox::Slot()
		.Padding(0.0f, 4.0f, 0.0f, 16.0f)
		[
			MakeColorSpinBox(EColorPickerChannels::Saturation)
		]

		// Red channel slider
		+SVerticalBox::Slot()
		.Padding(0.0f, 4.0f, 0.0f, 4.0f)
		[
			MakeColorSpinBox(EColorPickerChannels::Red)
		]

		// Green channel slider
		+SVerticalBox::Slot()
		.Padding(0.0f, 4.0f, 0.0f, 4.0f)
		[
			MakeColorSpinBox(EColorPickerChannels::Green)
		]

		// Blue channel slider
		+SVerticalBox::Slot()
		.Padding(0.0f, 4.0f, 0.0f, 8.0f)
		[
			MakeColorSpinBox(EColorPickerChannels::Blue)
		]
	];
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> SCustomColorPicker::MakeColorSlider(EColorPickerChannels channel) const {
	FText sliderTooltip;

	switch (channel) {
		case EColorPickerChannels::Red: {
			sliderTooltip  = LOCTEXT("RedSliderTooltip_TEXT", "Red");
			break;
		}
		case EColorPickerChannels::Green: {
			sliderTooltip  = LOCTEXT("GreenSliderTooltip_TEXT", "Green");
			break;
		}
		case EColorPickerChannels::Blue: {
			sliderTooltip  = LOCTEXT("BlueSliderTooltip_TEXT", "Blue");
			break;
		}
		case EColorPickerChannels::Saturation: {
			sliderTooltip  = LOCTEXT("SaturationSliderTooltip_TEXT", "Saturation");
			break;
		}
		default: {
			return SNullWidget::NullWidget;
		}
	}
	
	return SNew(SOverlay)
		.ToolTipText(sliderTooltip)
		+SOverlay::Slot()
		
		.Padding(FMargin(4.0f, 0.0f))
		[
			SNew(SSimpleGradient)
			.EndColor(this, &SCustomColorPicker::HandleColorSliderEndColor, channel)
			.StartColor(this, &SCustomColorPicker::HandleColorSliderStartColor, channel)
			.Orientation(Orient_Horizontal)
		]
		
		+SOverlay::Slot()
		[
			SNew(SSlider)
			.IndentHandle(false)
			.Orientation(Orient_Vertical)
			.SliderBarColor(FLinearColor::Transparent)
			.Style(&FAppStyle::Get().GetWidgetStyle<FSliderStyle>("ColorPicker.Slider"))
			.Value(this, &SCustomColorPicker::HandleColorSpinBoxValue, channel)
			.OnMouseCaptureBegin(const_cast<SCustomColorPicker*>(this), &SCustomColorPicker::HandleInteractiveChangeBegin)
			.OnMouseCaptureEnd(const_cast<SCustomColorPicker*>(this), &SCustomColorPicker::HandleInteractiveChangeEnd)
			.OnValueChanged(const_cast<SCustomColorPicker*>(this), &SCustomColorPicker::HandleColorSpinBoxValueChanged, channel)
		];
}

TSharedRef<SWidget> SCustomColorPicker::MakeColorSpinBox(EColorPickerChannels channel) const {
	const int32 gradientHeight = 6;
	const float hdrMaxValue = 1.0f;
	const FSlateFontInfo smallLayoutFont = FAppStyle::Get().GetFontStyle("ColorPicker.Font");

	TSharedPtr<SWidget> gradientWidget = SNew(SSimpleGradient)
		.StartColor(this, &SCustomColorPicker::GetGradientStartColor, channel)
		.EndColor(this, &SCustomColorPicker::GetGradientEndColor, channel)
		.HasAlphaBackground(false);

	float maxValue;
	FText sliderLabel;
	FText sliderTooltip;

	switch (channel) {
		case EColorPickerChannels::Red: {
			maxValue = hdrMaxValue;
			sliderLabel = LOCTEXT("RedSliderLabel_TEXT", "R");
			sliderTooltip  = LOCTEXT("RedSliderTooltip_TEXT", "Red");
			break;
		}
		case EColorPickerChannels::Green: {
			maxValue = hdrMaxValue;
			sliderLabel = LOCTEXT("GreenSliderLabel_TEXT", "G");
			sliderTooltip  = LOCTEXT("GreenSliderTooltip_TEXT", "Green");
			break;
		}
		case EColorPickerChannels::Blue: {
			maxValue = hdrMaxValue;
			sliderLabel = LOCTEXT("BlueSliderLabel_TEXT", "B");
			sliderTooltip  = LOCTEXT("BlueSliderTooltip_TEXT", "Blue");
			break;
		}
		case EColorPickerChannels::Saturation: {
			maxValue = 1.0f;
			sliderLabel = LOCTEXT("SaturationSliderLabel_TEXT", "S");
			sliderTooltip  = LOCTEXT("SaturationSliderTooltip_TEXT", "Saturation");
			break;
		}
		default: {
			return SNullWidget::NullWidget;
		}
	}

	static const float maxSpinBoxSize = 192.0f;

	return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(sliderLabel)
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.MaxWidth(maxSpinBoxSize)
		[
			SNew(SVerticalBox)
			.ToolTipText(sliderTooltip)

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpinBox<float>)
				.MinValue(0.0f)
				.MaxValue(maxValue)
				.MinSliderValue(0.0f)
				.MaxSliderValue(1.0f)
				.Delta(0.001f)
				.Font(smallLayoutFont)
				.Value(this, &SCustomColorPicker::HandleColorSpinBoxValue, channel)
				.OnBeginSliderMovement(const_cast<SCustomColorPicker*>(this), &SCustomColorPicker::HandleInteractiveChangeBegin)
				.OnEndSliderMovement(const_cast<SCustomColorPicker*>(this), &SCustomColorPicker::HandleInteractiveChangeEnd)
				.OnValueChanged(const_cast<SCustomColorPicker*>(this), &SCustomColorPicker::HandleColorSpinBoxValueChanged, channel)
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(gradientHeight)
				[
					gradientWidget.ToSharedRef()
				]
			]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FLinearColor SCustomColorPicker::GetGradientStartColor(EColorPickerChannels channel) const {
	switch (channel) {
		case EColorPickerChannels::Red: {
			return FLinearColor(0.0f, _currentColorRGB.G, _currentColorRGB.B, 1.0f);
		}
		case EColorPickerChannels::Green: {
			return FLinearColor(_currentColorRGB.R, 0.0f, _currentColorRGB.B, 1.0f);
		}
		case EColorPickerChannels::Blue: {
			return FLinearColor(_currentColorRGB.R, _currentColorRGB.G, 0.0f, 1.0f);
		}
		case EColorPickerChannels::Saturation: {
			return FLinearColor(_currentColorHSV.R, 0.0f, _currentColorHSV.B, 1.0f).HSVToLinearRGB();
		}
		default: {
			return FLinearColor();
		}
	}
}

FLinearColor SCustomColorPicker::GetGradientEndColor(EColorPickerChannels channel) const {
	switch (channel) {
		case EColorPickerChannels::Red: {
			return FLinearColor(1.0f, _currentColorRGB.G, _currentColorRGB.B, 1.0f);
		}
		case EColorPickerChannels::Green: {
			return FLinearColor(_currentColorRGB.R, 1.0f, _currentColorRGB.B, 1.0f);
		}
		case EColorPickerChannels::Blue: {
			return FLinearColor(_currentColorRGB.R, _currentColorRGB.G, 1.0f, 1.0f);
		}
		case EColorPickerChannels::Saturation: {
			return FLinearColor(_currentColorHSV.R, 1.0f, _currentColorHSV.B, 1.0f).HSVToLinearRGB();
		}
		default: {
			return FLinearColor();
		}
	}
}

FLinearColor SCustomColorPicker::HandleColorSliderStartColor(EColorPickerChannels channel) const {
	switch (channel) {
		case EColorPickerChannels::Red: {
			return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
		}
		case EColorPickerChannels::Green: {
			return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
		}
		case EColorPickerChannels::Blue: {
			return FLinearColor(0.0f, 0.0f, 1.0f, 1.0f);
		}
		case EColorPickerChannels::Saturation: {
			return FLinearColor(_currentColorHSV.R, 1.0f, 1.0f, 1.0f).HSVToLinearRGB();
		}
		default: {
			return FLinearColor();
		}
	}
}

FLinearColor SCustomColorPicker::HandleColorSliderEndColor(EColorPickerChannels channel) const {
	switch (channel) {
		case EColorPickerChannels::Red: {
			return FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		case EColorPickerChannels::Green: {
			return FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		case EColorPickerChannels::Blue: {
			return FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		case EColorPickerChannels::Saturation: {
			return FLinearColor(_currentColorHSV.R, 0.0f, 1.0f, 1.0f).HSVToLinearRGB();
		}
		default: {
			return FLinearColor();
		}
	}
}

float SCustomColorPicker::HandleColorSpinBoxValue(EColorPickerChannels channel) const {
	switch (channel) {
		case EColorPickerChannels::Red: {
			return _currentColorRGB.R;
		}
		case EColorPickerChannels::Green: {
			return _currentColorRGB.G;
		}
		case EColorPickerChannels::Blue: {
			return _currentColorRGB.B;
		}
		case EColorPickerChannels::Saturation: {
			return _currentColorHSV.G;
		}
		default: {
			return 0.0f;
		}
	}
}

void SCustomColorPicker::HandleColorSpinBoxValueChanged(float newValue, EColorPickerChannels channel) {
	int32 componentIndex;
	bool isHSV = false;
	
	switch (channel) {
		case EColorPickerChannels::Red: {
			componentIndex = 0;
			break;
		}
		case EColorPickerChannels::Green: {
			componentIndex = 1;
			break;
		}
		case EColorPickerChannels::Blue: {
			componentIndex = 2;
			break;
		}
		case EColorPickerChannels::Saturation: {
			componentIndex = 1;
			isHSV = true;
			break;
		}
		default: {
			return;
		}
	}

	FLinearColor& newColor = isHSV ? _currentColorHSV : _currentColorRGB;

	if (FMath::IsNearlyEqual(newValue, newColor.Component(componentIndex), KINDA_SMALL_NUMBER)) {
		return;
	}

	newColor.Component(componentIndex) = newValue;

	if (isHSV) {
		SetNewTargetColorHSV(newColor, !_isInteractive);
	} else {
		// Save value at 1.0
		FLinearColor hsvValue = newColor.LinearRGBToHSV();
		hsvValue.B = 1.0;
		newColor = hsvValue.HSVToLinearRGB();
		
		SetNewTargetColorRGB(newColor, !_isInteractive);
	}
}

bool SCustomColorPicker::SetNewTargetColorRGB(const FLinearColor& newValue, bool forceUpdate) {
	_currentColorRGB = newValue;
	_currentColorHSV = newValue.LinearRGBToHSV();

	return ApplyNewTargetColor(forceUpdate);
}

bool SCustomColorPicker::SetNewTargetColorHSV(const FLinearColor& newValue, bool forceUpdate) {
	_currentColorRGB = newValue.HSVToLinearRGB();
	_currentColorHSV = newValue;

	return ApplyNewTargetColor(forceUpdate);
}

bool SCustomColorPicker::ApplyNewTargetColor(bool forceUpdate) {
	bool updated = false;
	if (forceUpdate && !_perfIsTooSlowToUpdate) {
		double startUpdateTime = FPlatformTime::Seconds();
		UpdateColorPickMouseUp();
		double endUpdateTime = FPlatformTime::Seconds();

		if (endUpdateTime - startUpdateTime > MAX_ALLOWED_UPDATE_TIME) {
			_perfIsTooSlowToUpdate = true;
		}

		if (!_isInteractive) {
			_onColorCommited.ExecuteIfBound();
		}

		updated = true;
	}

	return updated;
}

void SCustomColorPicker::UpdateColorPickMouseUp() {
	UpdateColorPick();
}

void SCustomColorPicker::UpdateColorPick() {
	_perfIsTooSlowToUpdate = false;
	
	FLinearColor outColor = _currentColorRGB;

	_onColorChanged.ExecuteIfBound(outColor);
}

void SCustomColorPicker::HandleColorSpectrumValueChanged(FLinearColor newValue) {
	if (FMath::IsNearlyZero(newValue.B)) {
		newValue.B = 1.0f;
	}
	SetNewTargetColorHSV(newValue);
}

void SCustomColorPicker::HandleInteractiveChangeBegin() {
	_isInteractive = true;
}

void SCustomColorPicker::HandleInteractiveChangeEnd() {
	HandleInteractiveChangeEnd(0.0f);
}

void SCustomColorPicker::HandleInteractiveChangeEnd(float newValue) {
	_isInteractive = false;
	UpdateColorPickMouseUp();
}


#undef LOCTEXT_NAMESPACE
