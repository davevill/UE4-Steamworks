//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamNetDriver.h"



USteamNetDriver::USteamNetDriver(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

bool USteamNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
	return Super::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error);
}

bool USteamNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
	return Super::InitConnect(InNotify, ConnectURL, Error);
}

bool USteamNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
	return Super::InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);
}