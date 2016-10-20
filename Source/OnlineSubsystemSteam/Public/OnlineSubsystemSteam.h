//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "ModuleManager.h"



class FOnlineSubsystemSteamModule : public IModuleInterface
{
private:

	class FOnlineFactorySteam* SteamFactory;

public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
};
