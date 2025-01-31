#include "CyberGafferWindowShadersConfigCustomization.h"

#include "CyberGafferWindowContent.h"
#include "CyberGafferLog.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

TSharedRef<IDetailCustomization> FCyberGafferWindowShadersConfigCustomization::MakeInstance(TSharedPtr<SCyberGafferWindowContent> content) {
	return MakeShareable(new FCyberGafferWindowShadersConfigCustomization(content));
}

FCyberGafferWindowShadersConfigCustomization::FCyberGafferWindowShadersConfigCustomization(TSharedPtr<SCyberGafferWindowContent> content)  : _content(content) { }

void FCyberGafferWindowShadersConfigCustomization::CustomizeDetails(IDetailLayoutBuilder& detailLayout) {
	IDetailCategoryBuilder& shadersConfigBuilder = detailLayout.EditCategory(
		FName(TEXT("Shaders")),
		FText::GetEmpty(),
		ECategoryPriority::Uncommon
	);

	TArray<TSharedRef<IPropertyHandle>> handles;
    handles.Empty();
    shadersConfigBuilder.GetDefaultProperties(handles);

    for (auto handle : handles) {
    	detailLayout.HideProperty(handle);
    	shadersConfigBuilder.AddProperty(handle.ToSharedPtr());
    }
	
	shadersConfigBuilder.InitiallyCollapsed(true);
	shadersConfigBuilder.SetShowAdvanced(true);
	
	shadersConfigBuilder.AddCustomRow(LOCTEXT("RecompileShaders_Text", "Recompile Shaders"), false)
	.WholeRowContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("RecompileShaders_Text", "Recompile Shaders"))
			.OnClicked(_content.ToSharedRef(), &SCyberGafferWindowContent::RecompileShaders)
		]
	];
}

#undef LOCTEXT_NAMESPACE
