// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCyberGafferModule : public IModuleInterface {
private:
	inline static const char* _pluginName = "CyberGaffer";
public:
	static inline FCyberGafferModule& Get() {
		return FModuleManager::LoadModuleChecked<FCyberGafferModule>(_pluginName);
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FString GetPluginName();
	static FString GetPluginVersion();
	static FString GetPluginDirectory();
};
