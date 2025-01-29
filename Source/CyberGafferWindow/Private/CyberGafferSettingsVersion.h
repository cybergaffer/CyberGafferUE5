#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

struct FCyberGafferSettingsVersion {
	enum Type {
		Initial = 1,

		// add new versions above this line
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	const static FGuid GUID;

private:
	FCyberGafferSettingsVersion() {}
};
