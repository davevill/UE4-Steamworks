//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksGameSession.h"



FString ASteamworksGameSession::ApproveLogin(const FString& Options)
{
	return Super::ApproveLogin(Options);
}

void ASteamworksGameSession::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

