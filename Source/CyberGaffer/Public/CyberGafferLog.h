// Copyright (c) 2023 ALT LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(CyberGafferLog, Log, All);

static TAutoConsoleVariable<int32> CVarEnableCyberGafferLogging(
	TEXT("CyberGaffer.EnableDeveloperLogging"),
	false,                                 
	TEXT("Enables or disables CyberGaffer developer logging"),
	ECVF_Default
);

#define CYBERGAFFERLOG

#ifdef CYBERGAFFERLOG
#define CYBERGAFFER_LOG(verbosity, message, ...) UE_LOG(CyberGafferLog, verbosity, message, ##__VA_ARGS__)
#else
#define CYBERGAFFER_LOG(verbosity, message, ...)
#endif

#define CYBERGAFFERLOG_VERBOSE

#ifdef CYBERGAFFERLOG_VERBOSE
#define CYBERGAFFERVERB_LOG(verbosity, message, ...) \
	if(CVarEnableCyberGafferLogging.GetValueOnAnyThread() != 0) { \
		UE_LOG(CyberGafferLog, verbosity, message, ##__VA_ARGS__); \
		}
#else
#define CYBERGAFFERVERB_LOG(verbosity, message, ...)
#endif


