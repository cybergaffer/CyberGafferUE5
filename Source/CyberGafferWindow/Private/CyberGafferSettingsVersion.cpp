#include "CyberGafferSettingsVersion.h"
#include "Serialization/CustomVersion.h"

const FGuid FCyberGafferSettingsVersion::GUID(0x43F30DF9, 0x6EAC4098, 0xABC987DC, 0x822B38BA);

FCustomVersionRegistration GRegisterCyberGafferSettingsVersion(FCyberGafferSettingsVersion::GUID, FCyberGafferSettingsVersion::LatestVersion, TEXT("CyberGafferSettingsVersion"));