//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksManager.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Http.h"
#include "UniqueNetIdSteam.h"
#include "SteamworksGameMode.h"



#define STEAMWORKS_AVATAR_SIZE 184
#define STEAMWORKS_AVATAR_BYTE_SIZE STEAMWORKS_AVATAR_SIZE * STEAMWORKS_AVATAR_SIZE * 4





class FSteamworksCallbacks
{

public:

	STEAM_GAMESERVER_CALLBACK(FSteamworksCallbacks, OnValidateTicket, ValidateAuthTicketResponse_t, OnValidateTicketCallback);

	STEAM_CALLBACK(FSteamworksCallbacks, OnAvatarImageLoaded, AvatarImageLoaded_t, OnAvatarImageLoadedCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnPersonaStateChange, PersonaStateChange_t, OnPersonaStateChangeCallback);

	TWeakObjectPtr<USteamworksManager> Manager;

	FSteamworksCallbacks(USteamworksManager* Owner) :
		Manager(Owner),
		OnValidateTicketCallback(this, &FSteamworksCallbacks::OnValidateTicket),
		OnAvatarImageLoadedCallback(this, &FSteamworksCallbacks::OnAvatarImageLoaded),
		OnPersonaStateChangeCallback(this, &FSteamworksCallbacks::OnPersonaStateChange)
	{

	}


	


};

void FSteamworksCallbacks::OnPersonaStateChange(PersonaStateChange_t* pData)
{
	UTexture2D* AvatarTexture = Manager->GetAvatarBySteamId(pData->m_ulSteamID);
}

void FSteamworksCallbacks::OnAvatarImageLoaded(AvatarImageLoaded_t* pData)
{
	UTexture2D* AvatarTexture = Manager->Avatars.FindRef(pData->m_steamID.ConvertToUint64());
	ensure(AvatarTexture);

	if (AvatarTexture)
	{
		Manager->CopySteamAvatar(pData->m_iImage, AvatarTexture);
	}
}

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

			/*if (SteamGameServer())
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
			}*/	
		}

		bInitialized = true;
	}

	Callbacks = new FSteamworksCallbacks(this);
}

void USteamworksManager::Shutdown()
{
	if (Callbacks)
	{
		delete Callbacks;
		Callbacks = nullptr;
	}

	if (bInitialized)
	{
		SteamGameServer_Shutdown();
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

void USteamworksManager::CopySteamAvatar(int Handle, UTexture2D* AvatarTexture) const
{
	check(Handle > 0);
	check(AvatarTexture != nullptr);

	TArray<uint8> Buffer;
	Buffer.SetNumZeroed(STEAMWORKS_AVATAR_BYTE_SIZE);

	//copy it directly within this call
	if (SteamUtils()->GetImageRGBA(Handle, Buffer.GetData(), Buffer.Num()))
	{
		void* MipData = AvatarTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(MipData, Buffer.GetData(), Buffer.Num());
		AvatarTexture->PlatformData->Mips[0].BulkData.Unlock();

		AvatarTexture->PlatformData->NumSlices = 1;
		AvatarTexture->NeverStream = true;

		AvatarTexture->UpdateResource();
	}
	else
	{
		ensure(false && "this shouldn't happend");
	}
}

UTexture2D* USteamworksManager::GetAvatar(class APlayerState* PlayerState, UTexture2D* FailoverTexture)
{
	if (PlayerState == nullptr || !SteamFriends() || !SteamUtils()) return FailoverTexture;


	if (PlayerState->UniqueId.GetUniqueNetId().IsValid())
	{
		const FUniqueNetIdSteam* SteamNetId = (const FUniqueNetIdSteam*)PlayerState->UniqueId.GetUniqueNetId().Get();

		CSteamID SteamId(SteamNetId->UniqueNetId);

		UTexture2D* AvatarTexture=  GetAvatarBySteamId(SteamId);

		if (AvatarTexture)
		{
			return AvatarTexture;
		}
		else
		{
			return FailoverTexture;
		}
	}

	return FailoverTexture;
}

UTexture2D* USteamworksManager::GetAvatarBySteamId(CSteamID SteamId)
{

	if (!SteamId.IsValid())
	{
		return nullptr;
	}


	//Try to get it from the cache
	UTexture2D* AvatarTexture = Avatars.FindRef(SteamId.ConvertToUint64());

	if (AvatarTexture)
	{
		//we found it no need to do anything else
		return AvatarTexture;
	}

	//Create the texture
	AvatarTexture = UTexture2D::CreateTransient(STEAMWORKS_AVATAR_SIZE, STEAMWORKS_AVATAR_SIZE, PF_R8G8B8A8);

	int Handle = SteamFriends()->GetLargeFriendAvatar(SteamId);

	//check if Avatar is inmediatly available
	if (Handle > 0)
	{
		CopySteamAvatar(Handle, AvatarTexture);
	}
	else
	{
		uint8* MipData = (uint8*)AvatarTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memset(MipData, 0, STEAMWORKS_AVATAR_BYTE_SIZE);
		AvatarTexture->PlatformData->Mips[0].BulkData.Unlock();

		AvatarTexture->PlatformData->NumSlices = 1;
		AvatarTexture->NeverStream = true;

		AvatarTexture->UpdateResource();

		if (Handle == 0)
		{
			SteamFriends()->RequestUserInformation(SteamId, false);
		}

		//steam will call us back once the avatar is ready to be read
	}

	Avatars.Add(SteamId.ConvertToUint64(), AvatarTexture);

	return AvatarTexture;
}

