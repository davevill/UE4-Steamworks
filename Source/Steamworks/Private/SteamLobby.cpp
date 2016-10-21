//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksGameMode.h"
#include "SteamworksGameSession.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "UniqueNetIdSteam.h"
#include "SteamLobby.h"







void FSteamLobbyInfo::UpdateData(bool bPassive)
{
	check(SteamMatchmaking() != nullptr);

	if (!Id.IsValid()) return;

	const char* pchLobbyName = SteamMatchmaking()->GetLobbyData(Id, "name");

	if (pchLobbyName && pchLobbyName[0])
	{
		Name = pchLobbyName;
	}
	else
	{
		if (!bPassive)
		{
			SteamMatchmaking()->RequestLobbyData(Id);
		}
	}

	const uint32 Num = SteamMatchmaking()->GetLobbyDataCount(Id);

	char KeyBuffer[256];
	char ValueBuffer[4096];

	Metadata.Empty();

	for (uint32 i = 0; i < Num; i++)
	{
		SteamMatchmaking()->GetLobbyDataByIndex(Id, i, KeyBuffer, 1024, ValueBuffer, 4096);

		FSteamLobbyDataEntry Data;

		Data.Key = KeyBuffer;
		Data.Value = ValueBuffer;

		Metadata.Add(Data);
	}
}





