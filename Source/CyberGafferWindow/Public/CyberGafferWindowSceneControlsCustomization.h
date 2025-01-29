#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class SCyberGafferWindowContent;

class FCyberGafferWindowSceneControlsCustomization : public IDetailCustomization {
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<SCyberGafferWindowContent> content);
	FCyberGafferWindowSceneControlsCustomization(TSharedPtr<SCyberGafferWindowContent> content);
	
	virtual void CustomizeDetails(IDetailLayoutBuilder &detailLayout) override;

private:
	TSharedPtr<SCyberGafferWindowContent> _content;
};
