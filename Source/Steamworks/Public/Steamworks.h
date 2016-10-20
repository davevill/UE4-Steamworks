//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "ModuleManager.h"

class FSteamworksModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(SteamworksLog, Log, All);
