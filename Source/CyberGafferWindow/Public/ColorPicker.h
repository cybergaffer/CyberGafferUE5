#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Runtime/Slate/Public/Framework/SlateDelegates.h"

DECLARE_DELEGATE(FOnColorCommited)

enum class EColorPickerChannels {
	Red,
	Green,
	Blue,
	Saturation
};

// Custom color picker. Code is based on a basic SColorPicker from UE sources. 
class SCustomColorPicker : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SCustomColorPicker) :
		__targetColorAttribute(FLinearColor(ForceInit)),
		__onColorChanged(),
		__onColorCommited()
		{}
		
		SLATE_ATTRIBUTE(FLinearColor, _targetColorAttribute)
		SLATE_EVENT(FOnLinearColorValueChanged, _onColorChanged)
		SLATE_EVENT(FOnColorCommited, _onColorCommited)
		
	SLATE_END_ARGS()

private:
	TAttribute<FLinearColor> _targetColorAttribute;
	FOnLinearColorValueChanged _onColorChanged;
	FOnColorCommited _onColorCommited;

	FLinearColor _currentColorRGB;
	FLinearColor _currentColorHSV;
	
	bool _isInteractive;
	bool _perfIsTooSlowToUpdate;

	static CYBERGAFFERWINDOW_API const double MAX_ALLOWED_UPDATE_TIME;

public:
	CYBERGAFFERWINDOW_API void Construct(const FArguments& inArgs);

private:
	CYBERGAFFERWINDOW_API TSharedRef<SWidget> MakeColorSlider(EColorPickerChannels channel) const;
	CYBERGAFFERWINDOW_API TSharedRef<SWidget> MakeColorSpinBox(EColorPickerChannels channel) const;
	
	CYBERGAFFERWINDOW_API FLinearColor GetGradientStartColor(EColorPickerChannels channel) const;
	CYBERGAFFERWINDOW_API FLinearColor GetGradientEndColor(EColorPickerChannels channel) const;

	CYBERGAFFERWINDOW_API FLinearColor HandleColorSliderStartColor(EColorPickerChannels channel) const;
	CYBERGAFFERWINDOW_API FLinearColor HandleColorSliderEndColor(EColorPickerChannels channel) const;

	CYBERGAFFERWINDOW_API float HandleColorSpinBoxValue(EColorPickerChannels channel) const;
	CYBERGAFFERWINDOW_API void HandleInteractiveChangeBegin();
	CYBERGAFFERWINDOW_API void HandleInteractiveChangeEnd();
	CYBERGAFFERWINDOW_API void HandleInteractiveChangeEnd(float newValue);
	CYBERGAFFERWINDOW_API void HandleColorSpinBoxValueChanged(float newValue, EColorPickerChannels channel);

	CYBERGAFFERWINDOW_API bool SetNewTargetColorRGB(const FLinearColor& newValue, bool forceUpdate = false);
	CYBERGAFFERWINDOW_API bool SetNewTargetColorHSV(const FLinearColor& newValue, bool forceUpdate = false);
	CYBERGAFFERWINDOW_API bool ApplyNewTargetColor(bool forceUpdate = false);

	CYBERGAFFERWINDOW_API void UpdateColorPickMouseUp();
	CYBERGAFFERWINDOW_API void UpdateColorPick();

	CYBERGAFFERWINDOW_API void HandleColorSpectrumValueChanged(FLinearColor newValue);

	FLinearColor GetCurrentColorRGB() const {
		return _currentColorRGB;
	}

	FLinearColor GetCurrentColorHSV() const {
		return _currentColorHSV;
	}
};
