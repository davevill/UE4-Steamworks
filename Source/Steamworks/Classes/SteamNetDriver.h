//Copyright 2016 davevillz, https://github.com/davevill


#pragma once
#include "IpNetDriver.h"
#include "SteamNetDriver.generated.h"




UCLASS(transient, config = Engine)
class USteamNetDriver : public UIpNetDriver
{
	GENERATED_UCLASS_BODY()

	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) override;

};