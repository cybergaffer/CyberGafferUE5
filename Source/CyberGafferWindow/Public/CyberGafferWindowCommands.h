#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CyberGafferWindowStyle.h"

class FCyberGafferWindowCommands : public TCommands<FCyberGafferWindowCommands> {
public:

	FCyberGafferWindowCommands() : TCommands<FCyberGafferWindowCommands>(TEXT("TemplateWindow"), NSLOCTEXT("Contexts", "TemplateWindow", "TemplateWindow Plugin"), NAME_None, FCyberGafferWindowStyle::GetStyleSetName()) {
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};