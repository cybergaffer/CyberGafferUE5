#include "CyberGafferWindowAutomationCustomization.h"

#include "CyberGafferWindowContent.h"
#include "CyberGafferLog.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

TSharedRef<IDetailCustomization> FCyberGafferWindowAutomationCustomization::MakeInstance(TSharedPtr<SCyberGafferWindowContent> content) {
	return MakeShareable(new FCyberGafferWindowAutomationCustomization(content));
}

FCyberGafferWindowAutomationCustomization::FCyberGafferWindowAutomationCustomization(TSharedPtr<SCyberGafferWindowContent> content)  : _content(content) { }

void FCyberGafferWindowAutomationCustomization::CustomizeDetails(IDetailLayoutBuilder& detailLayout) {
	IDetailCategoryBuilder& automationBuilder = detailLayout.EditCategory(
		FName(TEXT("Automation")),
		FText::GetEmpty(),
		ECategoryPriority::Uncommon
	);
	
	FDetailWidgetRow& row = automationBuilder.AddCustomRow(LOCTEXT("ExecuteAutomation_Text", "ExecuteAutomation"), false)
	.WholeRowContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("ExecuteButton_Text", "Execute"))
			.OnClicked(_content.ToSharedRef(), &SCyberGafferWindowContent::OnExecuteAutomationClicked)
		]
	];
	
}

#undef LOCTEXT_NAMESPACE