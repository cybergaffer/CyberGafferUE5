#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class SCyberGafferWindowContent; 

class FCyberGafferWindowShadersConfigCustomization : public IDetailCustomization {
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<SCyberGafferWindowContent> content);
	FCyberGafferWindowShadersConfigCustomization(TSharedPtr<SCyberGafferWindowContent> content);
	
	virtual void CustomizeDetails(IDetailLayoutBuilder &detailLayout) override;

private:
	TSharedPtr<SCyberGafferWindowContent> _content;
};
