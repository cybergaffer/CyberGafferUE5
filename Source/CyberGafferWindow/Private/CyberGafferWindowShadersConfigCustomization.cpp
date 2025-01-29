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

	TArray<TSharedRef<IPropertyHandle>> defaultProperties;
	shadersConfigBuilder.GetDefaultProperties(defaultProperties);
	TSharedPtr<IPropertyHandle> handle;
	for (auto prop : defaultProperties) {
		FStringView propPath = prop.Get().GetPropertyPath();
		
		if (propPath.Equals("ShadersIncludePath")) {
			auto pathPropertyHandle = prop.Get().GetChildHandle(TEXT("Path"));
			if (pathPropertyHandle.IsValid() && pathPropertyHandle.Get() != nullptr) {
				handle = pathPropertyHandle.ToSharedRef();
			} else {
				CYBERGAFFER_LOG(Error, TEXT("FCyberGafferWindowShadersConfigCustomization::CustomizeDetails: Cannot find Path child property of ShadersIncludePath."));
				return;
			}
		}
	}

	FSimpleDelegate onChanged = FSimpleDelegate::CreateSP(_content.ToSharedRef(), &SCyberGafferWindowContent::OnShadersIncludePathCommitted);
	handle.Get()->SetOnPropertyValueChanged(onChanged);
	
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
