#include "CyberGafferWindowCommands.h"

#define LOCTEXT_NAMESPACE "FCyberGafferWindowModule"

void FCyberGafferWindowCommands::RegisterCommands() {
	UI_COMMAND(OpenPluginWindow, "CyberGafferWindow", "Bring up CyberGaffer window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
