//Copyright 2016 davevillz, https://github.com/davevill




#pragma once
#include "IpConnection.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "OnlineSubsystemTypes.h"
#include "SteamNetConnection.generated.h"




UCLASS(transient, config = Engine)
class USteamNetConnection : public UIpConnection
{
	GENERATED_UCLASS_BODY()


	TSharedPtr<FUniqueNetId> SteamNetId; 


	virtual void InitRemoteConnection(class UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitLocalConnection(class UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;


};