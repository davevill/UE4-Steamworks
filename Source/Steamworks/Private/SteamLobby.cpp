//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksGameMode.h"
#include "SteamworksGameSession.h"
#include "SteamworksManager.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "UniqueNetIdSteam.h"
#include "SteamLobby.h"
#include "Voice.h"
#include "AudioDevice.h"
#include "SoundDefinitions.h"
#include "Runtime/Engine/Classes/Sound/AudioSettings.h"
#include "Runtime/Engine/Classes/Sound/SoundWaveProcedural.h"










#define STEAMLOBBY_UPDATE_INTERVAL 10.f



USteamLobby::USteamLobby()
{
	bVoiceChatEnabled = false;

	VoiceVolume = 4.f;

}

void USteamLobby::SetVoiceChat(bool bEnabled)
{
	if (bVoiceChatEnabled == bEnabled) return;

	ensure(Manager.IsValid());

	if (Manager.IsValid() && bEnabled)
	{
		Manager->SetVoiceRecording(bEnabled);
	}

	bVoiceChatEnabled = bEnabled;

	if (bEnabled == false)
	{
		VoiceBuffers.Empty();
	}
}

void USteamLobby::Initialize()
{
	LocalUserId = SteamUser()->GetSteamID();

	check(Manager.IsValid());
	UpdateTimer = 0.f;
	UpdateMemberList();
}

void USteamLobby::Shutdown()
{

}

void USteamLobby::UpdateMemberList()
{
	check(SteamMatchmaking());

	const int32 N = GetNumLobbyMembers();

	Members.Empty();


	for (int32 i = 0; i < N; i++)
	{	
		FSteamLobbyMember Member;
		Member.UserId = SteamMatchmaking()->GetLobbyMemberByIndex(Info.Id, i);
		Member.Name = SteamFriends()->GetFriendPersonaName(Member.UserId);
		Member.Index = i;

		if (Manager.IsValid())
		{
			Member.AvatarTexture = Manager->GetAvatarBySteamId(Member.UserId);
		}


		Members.Push(Member);
	}

	Manager->OnLobbyMembersUpdate.Broadcast();
}

bool USteamLobby::IsLocalUserOwner() const
{
	check(SteamMatchmaking());

	CSteamID OwnerId = SteamMatchmaking()->GetLobbyOwner(Info.Id);

	ensure(OwnerId.IsValid());

	if (OwnerId == LocalUserId)
	{
		return true;
	}

	return false;
}

void USteamLobby::SetLobbyData(const FString& Key, const FString& Value)
{
	check(SteamMatchmaking());
	SteamMatchmaking()->SetLobbyData(Info.Id, TCHAR_TO_ANSI(*Key), TCHAR_TO_ANSI(*Value));
}

FString USteamLobby::GetLobbyData(const FString& Key) const
{
	check(SteamMatchmaking());
	return SteamMatchmaking()->GetLobbyData(Info.Id, TCHAR_TO_ANSI(*Key));
}

int32 USteamLobby::GetNumLobbyMembers() const
{
	check(SteamMatchmaking());
	return SteamMatchmaking()->GetNumLobbyMembers(Info.Id);
}

int32 USteamLobby::GetLobbyMemberLimit() const
{
	check(SteamMatchmaking());
	return SteamMatchmaking()->GetLobbyMemberLimit(Info.Id);
}

void USteamLobby::SetLobbyMemberLimit(int32 MemberLimit)
{
	check(SteamMatchmaking());
	SteamMatchmaking()->SetLobbyMemberLimit(Info.Id, MemberLimit);

	Manager->OnLobbyMembersUpdate.Broadcast();
}

FString USteamLobby::GetMemberData(int32 Index, const FString& Key) const
{
	check(SteamMatchmaking());

	const FSteamLobbyMember* Member = GetMemberByIndex(Index);

	if (Member)
	{
		return SteamMatchmaking()->GetLobbyMemberData(Info.Id, Member->UserId, TCHAR_TO_ANSI(*Key));
	}

	return "";
}

void USteamLobby::SetMemberData(const FString& Key, const FString& Value)
{
	check(SteamMatchmaking());

	SteamMatchmaking()->SetLobbyMemberData(Info.Id, TCHAR_TO_ANSI(*Key), TCHAR_TO_ANSI(*Value));
}

const FSteamLobbyMember* USteamLobby::GetMemberByIndex(int32 Index) const
{
	if (Index >= 0 && Index < Members.Num())
	{
		return &Members[Index];
	}

	return nullptr;
}

FSteamLobbyMember* USteamLobby::GetMutableMemberById(CSteamID Id)
{
	for (int32 i = 0; i < Members.Num(); i++)
	{
		if (Members[i].UserId == Id) return &Members[i];
	}

	return nullptr;
}

UAudioComponent* USteamLobby::CreateVoiceAudioComponent() const
{
	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice())
	{
		USoundWaveProcedural* SoundStreaming = nullptr;

		SoundStreaming = NewObject<USoundWaveProcedural>();
		SoundStreaming->SampleRate = VOICE_SAMPLE_RATE; 
		SoundStreaming->NumChannels = 1;
		SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
		SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
		SoundStreaming->bLooping = false;

		UAudioComponent* AudioComponent = AudioDevice->CreateComponent(SoundStreaming, GetWorld(), nullptr, false);

		if (AudioComponent)
		{
			AudioComponent->bIsUISound = true;
			AudioComponent->bAllowSpatialization = true;
			AudioComponent->bAlwaysPlay = true;
			AudioComponent->bAutoDestroy = false;
			AudioComponent->SetVolumeMultiplier(VoiceVolume);

			//AudioComponent->AddToRoot();

			return AudioComponent;
		}
	}


	return nullptr;
}

int32 USteamLobby::GetMemberIndexById(CSteamID Id) const
{
	for (int32 i = 0; i < Members.Num(); i++)
	{
		if (Members[i].UserId == Id) return i;
	}

	return -1;
}

FSteamLobbyVoiceBuffer& USteamLobby::GetVoiceBuffer(CSteamID Id)
{
	for (int32 i = 0; i < VoiceBuffers.Num(); i++)
	{
		if (VoiceBuffers[i].UserId == Id) return VoiceBuffers[i];
	}

	FSteamLobbyVoiceBuffer NewBuffer; 
	NewBuffer.UserId = Id;
	NewBuffer.AudioComponent = CreateVoiceAudioComponent();

	VoiceBuffers.Add(NewBuffer);

	return VoiceBuffers[VoiceBuffers.Num() - 1];
}

bool USteamLobby::IsMemberTalking(int32 Index) const
{
	const FSteamLobbyMember* Member = GetMemberByIndex(Index);

	if (Member)
	{
		if (Member->UserId == LocalUserId)
		{
			return LocalUserTalkTimer > 0.f;	
		}

		for (int32 i = 0; i < VoiceBuffers.Num(); i++)
		{
			if (VoiceBuffers[i].UserId == Member->UserId)
			{	
				return VoiceBuffers[i].TalkTimer > 0.f;
			}
		}
	}

	return false;
}

void USteamLobby::Tick(float DeltaTime)
{
	UpdateTimer += DeltaTime;

	if (UpdateTimer > STEAMLOBBY_UPDATE_INTERVAL)
	{
		UpdateTimer = 0.f;	
		OnLightTick();
	}

	if (IsVoiceChatEnabled())
	{
		Manager->SetVoiceRecording(true);
		//get voice data and send it to everyone in the lobby as p2p

		CompressedVoiceBuffer.SetNumUninitialized(STEAMWORKS_TICK_VOICE_BUFFER_SIZE, false);

		uint32 Size = STEAMWORKS_TICK_VOICE_BUFFER_SIZE;

		if (Manager->GetVoice_Implementation(CompressedVoiceBuffer.GetData(), Size))
		{
			LocalUserTalkTimer = 1.f;
			CompressedVoiceBuffer.SetNum(Size, false);

			for (auto Member : Members)
			{
				//Dont send to myself, duh
				if (Member.UserId == LocalUserId) continue;

				SteamNetworking()->SendP2PPacket(Member.UserId, CompressedVoiceBuffer.GetData(), Size, k_EP2PSendReliableWithBuffering, STEAMWORKS_LOBBY_VOICE_CHANNEL);
			}
		}	
	}

	uint32 PacketSize = 0;

	if (SteamNetworking()->IsP2PPacketAvailable(&PacketSize, STEAMWORKS_LOBBY_VOICE_CHANNEL))
	{
		CompressedVoiceBuffer.SetNumUninitialized(PacketSize, false);

		CSteamID RemoteUser;

		if (SteamNetworking()->ReadP2PPacket(CompressedVoiceBuffer.GetData(), PacketSize, &PacketSize, &RemoteUser, STEAMWORKS_LOBBY_VOICE_CHANNEL))
		{
			RawVoiceBuffer.SetNumUninitialized(STEAMWORKS_RAW_VOICE_BUFFER_SIZE, false);

			uint32 BufferSize = STEAMWORKS_RAW_VOICE_BUFFER_SIZE;

			if (PacketSize > 0 && Manager->DecompressVoice(CompressedVoiceBuffer.GetData(), PacketSize, RawVoiceBuffer.GetData(), BufferSize))
			{
				RawVoiceBuffer.SetNum(BufferSize, false);

				FSteamLobbyVoiceBuffer& VoiceBuffer = GetVoiceBuffer(RemoteUser);
				VoiceBuffer.TalkTimer = 1.f;

				USoundWaveProcedural* SoundStreaming = CastChecked<USoundWaveProcedural>(VoiceBuffer.AudioComponent->Sound);

				SoundStreaming->QueueAudio(RawVoiceBuffer.GetData(), BufferSize);

				if (VoiceBuffer.AudioComponent->IsPlaying() == false) VoiceBuffer.AudioComponent->Play();			
			}
		}
	}

	for (int32 i = 0; i < VoiceBuffers.Num(); i++)
	{
		VoiceBuffers[i].TalkTimer -= DeltaTime;

		if (VoiceBuffers[i].TalkTimer < 0.f) VoiceBuffers[i].TalkTimer = 0.f;
	}

	LocalUserTalkTimer -= DeltaTime;
	if (LocalUserTalkTimer < 0.f) LocalUserTalkTimer = 0.f;
}






void FSteamLobbyInfo::UpdateData(bool bPassive)
{
	//TODO find a better way to get all properties since this causes a hitch, a more efficient way is to get them on demand
	/*check(SteamMatchmaking() != nullptr);

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
	}*/
}

UWorld* USteamLobby::GetWorld() const
{
	ensure(Manager.IsValid());

	if (!Manager.IsValid()) return nullptr;

	return Manager->GetWorld();
}
