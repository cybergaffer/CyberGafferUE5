#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"

class SCyberGafferWindowContent; 

class FCyberGafferWindowAutomationCustomization : public IDetailCustomization {
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<SCyberGafferWindowContent> content);
	FCyberGafferWindowAutomationCustomization(TSharedPtr<SCyberGafferWindowContent> content);
	
	virtual void CustomizeDetails(IDetailLayoutBuilder &detailLayout) override;

private:
	TSharedPtr<SCyberGafferWindowContent> _content;
};

// class FCyberGafferWindowAutomationCustomization : public IPropertyTypeCustomization {
// public:
// 	static TSharedRef<IPropertyTypeCustomization> MakeInstance(TSharedPtr<SCyberGafferWindowContent> content);
// 	FCyberGafferWindowAutomationCustomization(TSharedPtr<SCyberGafferWindowContent> content);
//
// 	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
// 	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
//
// private:
// 	TSharedPtr<SCyberGafferWindowContent> _content;
// };
