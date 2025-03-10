﻿#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

CYBERGAFFER_API DECLARE_LOG_CATEGORY_EXTERN(CyberGafferLog, Log, All);

static TAutoConsoleVariable<int32> CVarEnableCyberGafferLogging(
	TEXT("CyberGaffer.EnableDeveloperLogging"),
	0,                                 
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


