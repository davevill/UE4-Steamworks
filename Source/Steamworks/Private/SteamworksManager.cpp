//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksManager.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Http.h"
#include "UniqueNetIdSteam.h"
#include "SteamworksGameMode.h"





class FSteamworksCallbacks
{

public:

	STEAM_GAMESERVER_CALLBACK(FSteamworksCallbacks, OnValidateTicket, ValidateAuthTicketResponse_t, OnValidateTicketCallback);



	TWeakObjectPtr<USteamworksManager> Manager;

	FSteamworksCallbacks(USteamworksManager* Owner) :
		Manager(Owner),
		OnValidateTicketCallback(this, &FSteamworksCallbacks::OnValidateTicket)
	{

	}


	


};

void FSteamworksCallbacks::OnValidateTicket(ValidateAuthTicketResponse_t* pResponse)
{
	if (pResponse->m_eAuthSessionResponse == k_EAuthSessionResponseOK)
	{

		FUniqueNetIdSteam SteamId(pResponse->m_SteamID);


		ASteamworksGameMode* GameMode = Manager->GetWorld()->GetAuthGameMode<ASteamworksGameMode>();

		if (GameMode)
		{
			GameMode->GetGameSessionClass();
		}

		int32 i = 0;
		i++;
		//Set a pending kick request, so when user fully logins will get kicked, if already in should happend instantly

		//TODO ban the user if this happends

		return;
	}
}


USteamworksManager::USteamworksManager(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	Callbacks = nullptr;
	bInitialized = false;
}

void USteamworksManager::Init()
{
	Callbacks = new FSteamworksCallbacks(this);

	GameInstance = Cast<UGameInstance>(GetOuter());

	/*FHttpModule* HTTP = &FHttpModule::Get();

	if (HTTP && HTTP->IsHttpEnabled())
	{

		FString TargetHost = "http://api.ipify.org";

		TSharedRef<IHttpRequest> Request = HTTP->CreateRequest();
		Request->SetVerb("GET");
		Request->SetURL(TargetHost);
		Request->SetHeader("User-Agent", "SteamworksUnreal/1.0");
		Request->SetHeader("Content-Type", "text/html");

		TWeakObjectPtr<USteamworksGameInstance> Instance = this;

		Request->OnProcessRequestComplete().BindLambda(
			[=](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
			{
				if (bSucceeded && Instance.IsValid())
				{
					Instance->OnPublicAddressResolved(Response->GetContentAsString());
				}
			}
		);

		Request->ProcessRequest();
	}*/

	if (GameInstance)
	{
		if (GameInstance->IsDedicatedServerInstance())
		{
			if (!SteamGameServer_Init(0, 7776, 7777, 7778, EServerMode::eServerModeAuthentication, "0.0.1"))
			{
				UE_LOG(SteamworksLog, Log, TEXT("SteamGameServer_Init() failed"));
			}

			if (SteamGameServer())
			{
				// Set the "game dir".
				// This is currently required for all games.  However, soon we will be
				// using the AppID for most purposes, and this string will only be needed
				// for mods.  it may not be changed after the server has logged on
				//SteamGameServer()->SetModDir( pchGameDir );

				// These fields are currently required, but will go away soon.
				// See their documentation for more info
				SteamGameServer()->SetProduct( "Pavlov" );
				SteamGameServer()->SetGameDescription( "VR Shooter" );

				// We don't support specators in our game.
				// .... but if we did:
				//SteamGameServer()->SetSpectatorPort( ... );
				//SteamGameServer()->SetSpectatorServerName( ... );

				// Initiate Anonymous logon.
				// Coming soon: Logging into authenticated, persistent game server account
				SteamGameServer()->LogOnAnonymous();


				// We want to actively update the master server with our presence so players can
				// find us via the steam matchmaking/server browser interfaces
				SteamGameServer()->EnableHeartbeats(true);


				UE_LOG(SteamworksLog, Log, TEXT("SteamGameServer LogOnAnonymous"));
				bInitialized = true;
			}	
		}
		else
		{
			if (SteamAPI_Init())
			{
				UE_LOG(SteamworksLog, Log, TEXT("SteamAPI_Init() succeeded"));
				bInitialized = true;
			}
			else
			{
				UE_LOG(SteamworksLog, Warning, TEXT("SteamAPI_Init() failed, make sure to run this with steam or if in development add the steam_appid.txt in the binary folder"));
			}	
		}

	}
}

void USteamworksManager::Shutdown()
{
	if (bInitialized)
	{

		SteamAPI_Shutdown();
		SteamGameServer_Shutdown();
	}

	if (Callbacks)
	{
		delete Callbacks;
		Callbacks = nullptr;
	}
}

void USteamworksManager::Tick(float DeltaTime)
{
	if (bInitialized)
	{
		SteamAPI_RunCallbacks();
	}
}

class UWorld* USteamworksManager::GetWorld() const
{
	if (GameInstance)
	{
		return GameInstance->GetWorld();
	}

	return nullptr;
}


USteamworksManager* USteamworksManager::Get(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		for (TObjectIterator<USteamworksManager> Itr; Itr; ++Itr)
		{
			if (Itr->GetWorld() == WorldContextObject->GetWorld() && Itr->bInitialized)
			{
				return *Itr;
			}
		}
	}

	return nullptr;
}

