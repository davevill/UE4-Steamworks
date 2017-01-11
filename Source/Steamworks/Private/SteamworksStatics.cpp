//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksStatics.h"
#include "SteamworksGameMode.h"
#include "SteamworksGameSession.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "UniqueNetIdSteam.h"


void USteamworksStatics::CreateLobby(UObject* WorldContextObject)
{


	
}


class FTestCallback
{
public:

	CCallResult<FTestCallback, LobbyMatchList_t> SteamCallResultLobbyMatchList;
	void OnLobbyMatchListCallback(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
	{

		for (uint32 i = 0; i < pLobbyMatchList->m_nLobbiesMatching; i++)
		{
			CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(i);

			const char* pchLobbyName = SteamMatchmaking()->GetLobbyData(steamIDLobby, "name");

			if (pchLobbyName && pchLobbyName[0])
			{
			}
			else
			{
				SteamMatchmaking()->RequestLobbyData( steamIDLobby );
				steamIDLobby.GetAccountID();
			}
		}

	}
};

void USteamworksStatics::FindLobbies(UObject* WorldContextObject)
{
	if (SteamMatchmaking() == nullptr) return;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();

	FTestCallback* Test = new FTestCallback();

	Test->SteamCallResultLobbyMatchList.Set(hSteamAPICall, Test, &FTestCallback::OnLobbyMatchListCallback);
}

FString USteamworksStatics::GetLocalSteamId(UObject* WorldContextObject)
{
	if (SteamUser())
	{
		return FString::Printf(TEXT("%llu"), SteamUser()->GetSteamID().ConvertToUint64());
	}

	return FString("null");
}


