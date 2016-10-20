//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksGameMode.h"
#include "SteamworksGameSession.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "UniqueNetIdSteam.h"









void ASteamworksGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

}

void ASteamworksGameMode::BeginDestroy()
{
	Super::BeginDestroy();

}

void ASteamworksGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	FString Ticket = ParseOption(Options, "ticket");


	TArray<uint8> Buffer;

	if (FBase64::Decode(Ticket, Buffer) && UniqueId.IsValid())
	{
		FUniqueNetIdSteam SteamId(*UniqueId.GetUniqueNetId());

		if (SteamGameServer() && SteamId.IsValid())
		{

			//Disabled for now, this is an antipiracy feature that is not required now

			/*//We let the user login, however when the callback returns we will kick it if nesesary
			EBeginAuthSessionResult Result = SteamGameServer()->BeginAuthSession(Buffer.GetData(), Buffer.Num(), *SteamId);

			if (Result != k_EBeginAuthSessionResultOK)
			{
				ErrorMessage = "Invalid auth ticket";

				return;
			}*/

			Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

		}
	}

	ErrorMessage = "Forbiden";
}

TSubclassOf<class AGameSession> ASteamworksGameMode::GetGameSessionClass() const
{
	return ASteamworksGameSession::StaticClass();
}



