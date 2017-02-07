//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksManager.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Http.h"
#include "UniqueNetIdSteam.h"
#include "SteamworksGameMode.h"
#include "LatentActions.h"
#include "Runtime/Engine/Classes/Sound/SoundWaveProcedural.h"
#include "Voice.h"
#include "SteamItem.h"
#include "AudioDevice.h"



#define STEAMWORKS_AVATAR_SIZE 184
#define STEAMWORKS_AVATAR_BYTE_SIZE STEAMWORKS_AVATAR_SIZE * STEAMWORKS_AVATAR_SIZE * 4





static ESteamLobbyChatMemberStateChange GetUnrealEnumFromSteamSDK(EChatMemberStateChange StateChange)
{
	switch (StateChange)
	{
	case k_EChatMemberStateChangeEntered:      return ESteamLobbyChatMemberStateChange::Entered;
	case k_EChatMemberStateChangeLeft:         return ESteamLobbyChatMemberStateChange::Left;			
	case k_EChatMemberStateChangeDisconnected: return ESteamLobbyChatMemberStateChange::Disconnected;
	case k_EChatMemberStateChangeKicked:       return ESteamLobbyChatMemberStateChange::Kicked;	
	case k_EChatMemberStateChangeBanned:       return ESteamLobbyChatMemberStateChange::Banned;
	};

	//can't happen
	ensure(false);
	return ESteamLobbyChatMemberStateChange::Disconnected;
}










class FSteamworksCallbacks
{

public:

	STEAM_GAMESERVER_CALLBACK(FSteamworksCallbacks, OnValidateTicket, ValidateAuthTicketResponse_t, OnValidateTicketCallback);

	STEAM_CALLBACK(FSteamworksCallbacks, OnAvatarImageLoaded, AvatarImageLoaded_t, OnAvatarImageLoadedCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnPersonaStateChange, PersonaStateChange_t, OnPersonaStateChangeCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnInventoryUpdate, SteamInventoryFullUpdate_t, OnInventoryUpdateCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnInventoryDefinitionUpdate, SteamInventoryDefinitionUpdate_t, OnInventoryDefinitionUpdateCallback);


	// Matchmaking & lobbies
	STEAM_CALLBACK(FSteamworksCallbacks, OnLobbyDataUpdated, LobbyDataUpdate_t, OnLobbyDataUpdatedCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnLobbyChatUpdated, LobbyChatUpdate_t, OnLobbyChatUpdatedCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnLobbyEnter, LobbyEnter_t, OnLobbyEnterCallback);
	STEAM_CALLBACK(FSteamworksCallbacks, OnLobbyChatMsg, LobbyChatMsg_t, OnLobbyChatMsgCallback);

	//P2P Networking
	STEAM_CALLBACK(FSteamworksCallbacks, OnP2PSessionRequest, P2PSessionRequest_t, OnP2PSessionRequestCallback);

	TWeakObjectPtr<USteamworksManager> Manager;

	FSteamworksCallbacks(USteamworksManager* Owner) :
		Manager(Owner),
		OnValidateTicketCallback(this, &FSteamworksCallbacks::OnValidateTicket),
		OnAvatarImageLoadedCallback(this, &FSteamworksCallbacks::OnAvatarImageLoaded),
		OnPersonaStateChangeCallback(this, &FSteamworksCallbacks::OnPersonaStateChange),
		OnInventoryUpdateCallback(this, &FSteamworksCallbacks::OnInventoryUpdate),
		OnInventoryDefinitionUpdateCallback(this, &FSteamworksCallbacks::OnInventoryDefinitionUpdate),
		OnLobbyDataUpdatedCallback(this, &FSteamworksCallbacks::OnLobbyDataUpdated),
		OnLobbyChatUpdatedCallback(this, &FSteamworksCallbacks::OnLobbyChatUpdated),
		OnLobbyEnterCallback(this, &FSteamworksCallbacks::OnLobbyEnter),
		OnLobbyChatMsgCallback(this, &FSteamworksCallbacks::OnLobbyChatMsg),
		OnP2PSessionRequestCallback(this, &FSteamworksCallbacks::OnP2PSessionRequest)
	{

	}


	CCallResult<FSteamworksCallbacks, LobbyCreated_t> SteamCallLobbyCreated;

	void OnLobbyCreatedCallback(LobbyCreated_t* pLobbyCreated, bool bIOFailure)
	{
		if (pLobbyCreated->m_eResult == EResult::k_EResultOK)
		{
			ensure(Manager->LobbyInstance == nullptr);

			if (Manager->LobbyInstance)
			{
				//Manager->LeaveLobby();
			}


			//Manager->CreateLobbyInstance(pLobbyCreated->m_ulSteamIDLobby);
		}
	}


	/*CCallResult<FSteamworksCallbacks, LobbyEnter_t> SteamCallResultLobbyEnter;

	void OnLobbyEnterCallback(LobbyEnter_t* pLobbyEnter, bool bIOFailure)
	{
		if (pLobbyEnter && pLobbyEnter->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
		{
			CSteamID LobbyId;
			LobbyId.SetFromUint64(pLobbyEnter->m_ulSteamIDLobby);

			ensure(LobbyId.IsValid() && LobbyId.IsLobby());

			Manager->CreateLobbyInstance(LobbyId);
			Manager->OnEnterLobby.Broadcast(true);
		}
		else
		{
			Manager->OnEnterLobby.Broadcast(false);
		}
	}*/


	CCallResult<FSteamworksCallbacks, LobbyMatchList_t> SteamCallResultLobbyMatchList;

	void OnLobbyMatchListCallback(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
	{
		Manager->LobbyList.Empty();

		for (uint32 i = 0; i < pLobbyMatchList->m_nLobbiesMatching; i++)
		{
			FSteamLobbyInfo Info;

			Info.Id = SteamMatchmaking()->GetLobbyByIndex(i);
			Info.Name = "";
			Info.UpdateData();

			Manager->LobbyList.Add(Info);
		}


		Manager->bRequestingLobbyList = false;

		Manager->OnLobbyListUpdated.Broadcast();
	}
};

void FSteamworksCallbacks::OnInventoryDefinitionUpdate(SteamInventoryDefinitionUpdate_t* pData)
{

}

void FSteamworksCallbacks::OnInventoryUpdate(SteamInventoryFullUpdate_t* pData)
{
	TArray<SteamItemDetails_t> ItemDetails;

	uint32 Count = 0;

	Manager->Inventory.Empty();

	if (SteamInventory()->GetResultItems(pData->m_handle, nullptr, &Count))
	{
ItemDetails.SetNumZeroed(Count);

if (SteamInventory()->GetResultItems(pData->m_handle, ItemDetails.GetData(), &Count))
{
	for (uint32 i = 0; i < Count; i++)
	{
		USteamItem* Item = NewObject<USteamItem>(Manager.Get());
		check(Item);

		Item->Details = ItemDetails[i];

		Manager->Inventory.Add(Item);
	}
}
	}

	Manager->OnInventoryUpdate.Broadcast();
}

void FSteamworksCallbacks::OnPersonaStateChange(PersonaStateChange_t* pData)
{
	UTexture2D* AvatarTexture = Manager->GetAvatarBySteamId(pData->m_ulSteamID);

	CSteamID SteamId;
	SteamId.SetFromUint64(pData->m_ulSteamID);

	Manager->LoadSteamAvatar(SteamId, AvatarTexture);
}

void FSteamworksCallbacks::OnAvatarImageLoaded(AvatarImageLoaded_t* pData)
{
	UTexture2D* AvatarTexture = Manager->GetAvatarBySteamId(pData->m_steamID.ConvertToUint64());
	ensure(AvatarTexture);

	Manager->LoadSteamAvatar(pData->m_steamID, AvatarTexture);
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

void FSteamworksCallbacks::OnLobbyDataUpdated(LobbyDataUpdate_t* pCallback)
{
	if (pCallback->m_bSuccess)
	{
		for (int32 i = 0; i < Manager->LobbyList.Num(); i++)
		{
			FSteamLobbyInfo& Info = Manager->LobbyList[i];

			if (Info.Id == pCallback->m_ulSteamIDLobby)
			{
				Info.UpdateData(true);
				break;
			}
		}

		USteamLobby* Lobby = Manager->LobbyInstance;

		if (Lobby && Lobby->Info.Id == pCallback->m_ulSteamIDLobby)
		{
			Lobby->Info.UpdateData(true);
			Lobby->UpdateMemberList();
			Lobby->OnLobbyDataUpdated();
			Manager->OnLobbyUpdated.Broadcast();
		}

	}
}

void FSteamworksCallbacks::OnLobbyChatUpdated(LobbyChatUpdate_t* pCallback)
{
	if (pCallback)
	{
		USteamLobby* SteamLobby = Manager->GetLobbyInstance();


		CSteamID LobbyId;
		LobbyId.SetFromUint64(pCallback->m_ulSteamIDLobby);

		if (SteamLobby && SteamLobby->Info.Id == LobbyId)
		{
			SteamLobby->UpdateMemberList();	
		}

		static CSteamID LocalUserId = SteamUser()->GetSteamID();

		if (LocalUserId == pCallback->m_ulSteamIDUserChanged)
		{
			if (pCallback->m_rgfChatMemberStateChange != k_EChatMemberStateChangeEntered)
			{
				Manager->OnLeftLobby.Broadcast(GetUnrealEnumFromSteamSDK(EChatMemberStateChange(pCallback->m_rgfChatMemberStateChange)));
			}
			
		}
		else
		{
			//TODO report other user's state change
		}

	}
}

void FSteamworksCallbacks::OnLobbyEnter(LobbyEnter_t* pLobbyEnter)
{
	if (pLobbyEnter && pLobbyEnter->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
	{
		CSteamID LobbyId;
		LobbyId.SetFromUint64(pLobbyEnter->m_ulSteamIDLobby);

		ensure(LobbyId.IsValid() && LobbyId.IsLobby());

		Manager->CreateLobbyInstance(LobbyId);
		Manager->OnEnterLobby.Broadcast(false);
	}
	else
	{
		Manager->OnEnterLobby.Broadcast(true);
	}
}

void FSteamworksCallbacks::OnLobbyChatMsg(LobbyChatMsg_t* pCallback)
{
	if (pCallback)
	{
		USteamLobby* Lobby = Manager->LobbyInstance;

		if (Lobby && Lobby->Info.Id == pCallback->m_ulSteamIDLobby)
		{
			CSteamID SenderId;

			TArray<char, TInlineAllocator<256>> Buffer;
			Buffer.SetNum(256);

			EChatEntryType ChatEntryType;

			int WrittenSize = SteamMatchmaking()->GetLobbyChatEntry(pCallback->m_ulSteamIDLobby, pCallback->m_iChatID, &SenderId, Buffer.GetData(), 256, &ChatEntryType);

			Buffer.SetNum(WrittenSize, false);

			Lobby->OnLobbyChatMsg(SenderId, Buffer);

		}
	}
}

void FSteamworksCallbacks::OnP2PSessionRequest(P2PSessionRequest_t* pCallback)
{
	USteamLobby* Lobby = Manager->LobbyInstance;

	if (Lobby)
	{
		for (auto Member : Lobby->Members)
		{
			if (Member.UserId == pCallback->m_steamIDRemote)
			{
				SteamNetworking()->AcceptP2PSessionWithUser(pCallback->m_steamIDRemote);
				break;
			}
		}
	}
}

















USteamworksManager::USteamworksManager(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	Callbacks = nullptr;
	bInitialized = false;

	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture();
	VoiceEncoder = FVoiceModule::Get().CreateVoiceEncoder();
	VoiceDecoder = FVoiceModule::Get().CreateVoiceDecoder();

	MaxRawCaptureDataSize = STEAMWORKS_RAW_VOICE_BUFFER_SIZE;
	MaxCompressedDataSize = STEAMWORKS_VOICE_BUFFER_SIZE;
	MaxUncompressedDataSize = MaxRawCaptureDataSize;
	MaxRemainderSize = 1 * 1024;
	LastRemainderSize = 0;

	RawCaptureData.AddUninitialized(MaxRawCaptureDataSize);
	CompressedData.AddUninitialized(MaxCompressedDataSize);
	UncompressedData.AddUninitialized(MaxUncompressedDataSize);
	Remainder.AddUninitialized(MaxRemainderSize);


	bRequestingLobbyList = false;

	LobbyClass = USteamLobby::StaticClass();
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

		if (SteamUser() == nullptr)
		{
			if (SteamAPI_Init())
			{
				UE_LOG_ONLINE(Log, TEXT("SteamAPI_Init() succeeded"));
			}
			else
			{
				UE_LOG_ONLINE(Warning, TEXT("SteamAPI_Init() failed, make sure to run this with steam or if in development add the steam_appid.txt in the binary folder"));
			}
		}

		bInitialized = true;
	}

	Callbacks = new FSteamworksCallbacks(this);

	if (SteamInventory())
	{
		//load the definitions for usage in client and server usage
SteamInventory()->LoadItemDefinitions();
	}

	bRecordingVoice = false;
}

void USteamworksManager::GetAllInventoryItems()
{
	SteamInventoryResult_t Result;

	if (SteamInventory() && SteamInventory()->GetAllItems(&Result))
	{
	}
}

bool USteamworksManager::HasInstanceOf(int32 DefinitionId) const
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i]->Details.m_iDefinition == DefinitionId)
		{
			return true;
		}
	}

	return false;
}

void USteamworksManager::Shutdown()
{
	SetVoiceRecording(false);

	if (Callbacks)
	{
		delete Callbacks;
		Callbacks = nullptr;
	}

	if (bInitialized)
	{
		SteamAPI_Shutdown();
		//SteamGameServer_Shutdown();
	}

	if (VoiceCapture.IsValid())
	{
		VoiceCapture->Stop();
		VoiceCapture->Shutdown();
	}
}

void USteamworksManager::Tick(float DeltaTime)
{
	if (bInitialized)
	{
		SteamAPI_RunCallbacks();

		if (LobbyInstance)
		{
			LobbyInstance->Tick(DeltaTime);
		}
	}
}

void USteamworksManager::SetVoiceRecording(bool bEnabled)
{
	if (bRecordingVoice == bEnabled || SteamUser() == nullptr) return;

	bRecordingVoice = bEnabled;

	if (bRecordingVoice)
	{

		if (VoiceCapture.IsValid()) VoiceCapture->Start();
		//SteamUser()->StartVoiceRecording();
		//SteamFriends()->SetInGameVoiceSpeaking(SteamUser()->GetSteamID(), true);
	}
	else
	{
		if (VoiceCapture.IsValid()) VoiceCapture->Stop();

		//SteamUser()->StopVoiceRecording();
		//SteamFriends()->SetInGameVoiceSpeaking(SteamUser()->GetSteamID(), false);
	}
}

bool USteamworksManager::GetVoice(uint8* DestBuffer, uint32& WrittenSize)
{
	//if we have a lobby instance and has voice enabled, ignore all calls, we will foward it to the lobby instead
	if (LobbyInstance && LobbyInstance->IsVoiceChatEnabled())
	{
		WrittenSize = 0;
		return false;	
	}

	return GetVoice_Implementation(DestBuffer, WrittenSize);
}

bool USteamworksManager::GetVoice_Implementation(uint8* DestBuffer, uint32& WrittenSize)
{
	if (!VoiceCapture.IsValid())
	{
		WrittenSize = 0;
		return false;
	}


	bool bDoWork = false;
	uint32 TotalVoiceBytes = 0;

	uint32 NewVoiceDataBytes = 0;
	EVoiceCaptureState::Type MicState = VoiceCapture->GetCaptureState(NewVoiceDataBytes);

	if (MicState == EVoiceCaptureState::Ok && NewVoiceDataBytes > 0)
	{
		TotalVoiceBytes = NewVoiceDataBytes + LastRemainderSize;
		RawCaptureData.Empty(MaxRawCaptureDataSize);
		RawCaptureData.AddUninitialized(TotalVoiceBytes);

		if (LastRemainderSize > 0)
		{
			FMemory::Memcpy(RawCaptureData.GetData(), Remainder.GetData(), LastRemainderSize);
		}

		MicState = VoiceCapture->GetVoiceData(RawCaptureData.GetData() + LastRemainderSize, NewVoiceDataBytes, NewVoiceDataBytes);
		TotalVoiceBytes = NewVoiceDataBytes + LastRemainderSize;
		bDoWork = MicState == EVoiceCaptureState::Ok;
	}

	if (bDoWork && TotalVoiceBytes > 0)
	{
		WrittenSize = MaxCompressedDataSize;
		LastRemainderSize = VoiceEncoder->Encode(RawCaptureData.GetData(), TotalVoiceBytes, DestBuffer, WrittenSize);

		if (LastRemainderSize > 0)
		{
			FMemory::Memcpy(Remainder.GetData(), RawCaptureData.GetData() + (TotalVoiceBytes - LastRemainderSize), LastRemainderSize);
		}

		return WrittenSize > 0;
	}

	return false;
}

bool USteamworksManager::DecompressVoice(const uint8* CompressedBuffer, uint32 CompressedSize, uint8* DestBuffer, uint32& WrittenSize)
{
	WrittenSize = STEAMWORKS_RAW_VOICE_BUFFER_SIZE;
	VoiceDecoder->Decode(CompressedBuffer, CompressedSize, DestBuffer, WrittenSize);

	return WrittenSize > 0;
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

bool USteamworksManager::LoadSteamAvatar(CSteamID SteamId, UTexture2D* AvatarTexture) const
{
	if (AvatarTexture != nullptr && SteamFriends() != nullptr && SteamId.IsValid())
	{
		if (AvatarTexture)
		{
			int Handle = SteamFriends()->GetLargeFriendAvatar(SteamId);

			if (Handle > 0)
			{
				CopySteamAvatar(Handle, AvatarTexture);

				return true;
			}
		}
	}

	return false;
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


	if (!SteamFriends()->RequestUserInformation(SteamId, false))
	{
		//the avatar is imediatly available
		bool bResult = LoadSteamAvatar(SteamId, AvatarTexture);

		ensure(bResult == true);
	}
	else
	{
		uint8* MipData = (uint8*)AvatarTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memset(MipData, 0, STEAMWORKS_AVATAR_BYTE_SIZE);
		AvatarTexture->PlatformData->Mips[0].BulkData.Unlock();

		AvatarTexture->PlatformData->NumSlices = 1;
		AvatarTexture->NeverStream = true;

		AvatarTexture->UpdateResource();
		//steam will call us back once the avatar is ready to be read
	}

	Avatars.Add(SteamId.ConvertToUint64(), AvatarTexture);

	return AvatarTexture;
}




#define STEAMWORKS_CHECK_MATCHMAKING() if (!SteamMatchmaking()) { UE_LOG(SteamworksLog, Error, TEXT("SteamMatchmaking is null")); ensure(false); return; }

void USteamworksManager::RequestLobbyList()
{
	if (bRequestingLobbyList) return;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();

	Callbacks->SteamCallResultLobbyMatchList.Set(hSteamAPICall, Callbacks, &FSteamworksCallbacks::OnLobbyMatchListCallback);

	bRequestingLobbyList = true;
}

static ELobbyComparison GetSteamSDKEnum(ESteamLobbyComparison ComparisonType)
{
	switch (ComparisonType)
	{
	case ESteamLobbyComparison::EqualToOrLessThan:    return k_ELobbyComparisonEqualToOrLessThan;
	case ESteamLobbyComparison::LessThan:             return k_ELobbyComparisonLessThan;
	case ESteamLobbyComparison::Equal:                return k_ELobbyComparisonEqual;
	case ESteamLobbyComparison::GreaterThan:          return k_ELobbyComparisonGreaterThan;
	case ESteamLobbyComparison::EqualToOrGreaterThan: return k_ELobbyComparisonEqualToOrGreaterThan;
	case ESteamLobbyComparison::NotEqual:             return k_ELobbyComparisonNotEqual;
	};

	ensure(false);
	return k_ELobbyComparisonEqual;
};


static ELobbyDistanceFilter GetSteamSDKEnum(ESteamLobbyDistanceFilter Filter)
{
	switch (Filter)
	{
	case ESteamLobbyDistanceFilter::Close: return k_ELobbyDistanceFilterClose;
	case ESteamLobbyDistanceFilter::Default: return k_ELobbyDistanceFilterDefault;
	case ESteamLobbyDistanceFilter::Far: return k_ELobbyDistanceFilterFar;
	case ESteamLobbyDistanceFilter::Worldwide: return k_ELobbyDistanceFilterWorldwide;
	};

	return k_ELobbyDistanceFilterDefault;
}

void USteamworksManager::AddRequestLobbyListStringFilter(const FString& KeyToMatch, const FString& ValueToMatch, ESteamLobbyComparison ComparisonType)
{
	STEAMWORKS_CHECK_MATCHMAKING();
	SteamMatchmaking()->AddRequestLobbyListStringFilter(TCHAR_TO_ANSI(*KeyToMatch), TCHAR_TO_ANSI(*ValueToMatch), GetSteamSDKEnum(ComparisonType));
}

void USteamworksManager::AddRequestLobbyListNumericalFilter(const FString& KeyToMatch, int32 ValueToMatch, ESteamLobbyComparison ComparisonType)
{
	STEAMWORKS_CHECK_MATCHMAKING();
	SteamMatchmaking()->AddRequestLobbyListNumericalFilter(TCHAR_TO_ANSI(*KeyToMatch), ValueToMatch, GetSteamSDKEnum(ComparisonType));
}

void USteamworksManager::AddRequestLobbyListNearValueFilter(const FString& KeyToMatch, int32 ValueToBeCloseTo)
{
	STEAMWORKS_CHECK_MATCHMAKING();
	SteamMatchmaking()->AddRequestLobbyListNearValueFilter(TCHAR_TO_ANSI(*KeyToMatch), ValueToBeCloseTo);
}

void USteamworksManager::AddRequestLobbyListFilterSlotsAvailable(int32 SlotsAvailable)
{
	STEAMWORKS_CHECK_MATCHMAKING();
	SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(SlotsAvailable);
}

void USteamworksManager::AddRequestLobbyListDistanceFilter(ESteamLobbyDistanceFilter DistanceFilter)
{
	STEAMWORKS_CHECK_MATCHMAKING();
	SteamMatchmaking()->AddRequestLobbyListDistanceFilter(GetSteamSDKEnum(DistanceFilter));
}

void USteamworksManager::CreateLobbyInstance(CSteamID LobbyId)
{
	ensure(LobbyInstance == nullptr);

	UClass* Class = LobbyClass;

	if (Class == nullptr)
	{
		Class = USteamLobby::StaticClass();
	}

	LobbyInstance = NewObject<USteamLobby>(this, Class, "LobbyInstance");
	check(LobbyInstance);

	LobbyInstance->Info.Id = LobbyId;
	LobbyInstance->Info.UpdateData(true);

	LobbyInstance->Manager = this;
	LobbyInstance->Initialize();
}

void USteamworksManager::JoinLobby(FSteamLobbyInfo LobbyInfo)
{
	STEAMWORKS_CHECK_MATCHMAKING();

	if (LobbyInstance)
	{
		LeaveLobby();
	}

	ensure(LobbyInstance == nullptr);

	SteamMatchmaking()->JoinLobby(LobbyInfo.Id);
}

void USteamworksManager::LeaveLobby()
{
	STEAMWORKS_CHECK_MATCHMAKING();

	ensure(LobbyInstance);

	if (LobbyInstance)
	{
		SteamMatchmaking()->LeaveLobby(LobbyInstance->Info.Id);

		LobbyInstance->Shutdown();
		LobbyInstance = nullptr;

		//Non standard on steam, but we got no feedback whether we left or not
		OnLeftLobby.Broadcast(ESteamLobbyChatMemberStateChange::Left);
	}
}

void USteamworksManager::CreateLobby(int32 LobbyMemberLimit, bool bPublic)
{
	STEAMWORKS_CHECK_MATCHMAKING();

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(bPublic ? ELobbyType::k_ELobbyTypePublic : ELobbyType::k_ELobbyTypeFriendsOnly, LobbyMemberLimit);

	Callbacks->SteamCallLobbyCreated.Set(hSteamAPICall, Callbacks, &FSteamworksCallbacks::OnLobbyCreatedCallback);
}