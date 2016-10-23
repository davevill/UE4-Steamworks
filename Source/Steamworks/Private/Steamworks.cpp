//Copyright 2016 davevillz, https://github.com/davevill


#include "SteamworksPrivatePCH.h"


#define LOCTEXT_NAMESPACE "FSteamworksModule"




void FSteamworksModule::StartupModule()
{
}

void FSteamworksModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSteamworksModule, Steamworks)
DEFINE_LOG_CATEGORY(SteamworksLog);
