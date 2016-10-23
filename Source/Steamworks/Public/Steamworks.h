//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "ModuleManager.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"

class FSteamworksModule : public IModuleInterface
{
private:


	class FOnlineFactorySteam* SteamFactory;


public:


	virtual void StartupModule() override;
	virtual void ShutdownModule() override;



};

DECLARE_LOG_CATEGORY_EXTERN(SteamworksLog, Log, All);
