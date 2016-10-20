//Copyright 2016 davevillz, https://github.com/davevill




#include "SteamworksPrivatePCH.h"
#include "Classes/SteamNetConnection.h"
#include "SteamNetDriver.h"



USteamNetConnection::USteamNetConnection(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void USteamNetConnection::InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	if (SteamUser())
	{

		ensure(SteamUser());

		SteamNetId = MakeShareable(new FUniqueNetIdSteam(SteamUser()->GetSteamID()));
		
		//PlayerId.SetUniqueNetId(SteamNetId);
	}

	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);
}

void USteamNetConnection::InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
}