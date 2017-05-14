//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamVoiceComponent.h"
#include "Runtime/Engine/Classes/Sound/SoundWaveProcedural.h"
#include "SteamworksManager.h"
#include "SteamRadio.h"
#include "Voice.h"



DECLARE_CYCLE_STAT(TEXT("SteamVoiceTick"), STAT_SteamVoiceTick, STATGROUP_Steamworks);
DECLARE_CYCLE_STAT(TEXT("SteamVoicePlayback"), STAT_SteamVoicePlayback, STATGROUP_Steamworks);





USteamVoiceComponent::USteamVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.bRunOnAnyThread = true;
	PrimaryComponentTick.TickInterval = 0.025f;//125f;
	bWantsInitializeComponent = true;

	bReplicates = true;
	bRecordingVoice = false;
}

void USteamVoiceComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SoundStreaming = NewObject<USoundWaveProcedural>();
	SoundStreaming->SampleRate = VOICE_SAMPLE_RATE;//SteamUser() == nullptr ? 11025 : SteamUser()->GetVoiceOptimalSampleRate();
	SoundStreaming->NumChannels = 1;
	SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
	SoundStreaming->bLooping = true;
	SoundStreaming->bProcedural = true;

	SetWaveParameter("Voice", SoundStreaming);

	//OnAudioFinished.AddDynamic(this, &USteamVoiceComponent::OnVoiceFinished);


	/*UNetConnection* Connection = GetOwner()->GetNetConnection();

	if (Connection)
	{
		UChannel* Channel = Connection->CreateChannel(EChannelType::CHTYPE_Voice, false);

		ensure(Channel != nullptr);

		Channel->IsUnreachable();
	}*/

	Manager = USteamworksManager::Get(this);
}

class ASteamRadio* USteamVoiceComponent::CreateSteamRadioInstance()
{
	ASteamRadio* Instance = GetWorld()->SpawnActor<ASteamRadio>();

	if (Instance)
	{
		Instance->InitializePlaybackChannels(RadioVoiceCue);
	}

	return Instance;
}

class ASteamRadio* USteamVoiceComponent::GetSteamRadioInstance()
{
	ASteamRadio* Instance = nullptr;

	TActorIterator<ASteamRadio> Itr(GetWorld());

	if (Itr)
	{
		Instance = *Itr;
	}
	else
	{
		Instance = CreateSteamRadioInstance();
		ensure(Instance != nullptr);
	}

	ensure(Instance != nullptr);

	return Instance;
}

void USteamVoiceComponent::UninitializeComponent()
{
	SoundStreaming = nullptr;

	Super::UninitializeComponent();
}

void USteamVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_SteamVoiceTick);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	APawn* Pawn = Cast<APawn>(GetOwner());

	//we shouldn't hear ourselves 
	if (Pawn && Pawn->Controller == UGameplayStatics::GetPlayerController(this, 0) && Manager)
	{	
		if (bOpenMic)
		{
			Manager->SetVoiceRecording(true);
			bRecordingVoice = true;
		}

		if (bRecordingVoice && SteamUser())
		{
			FSteamworksVoicePacket& VoicePacket = TempVoicePacket;
			VoicePacket.Data.SetNumUninitialized(STEAMWORKS_VOICE_BUFFER_SIZE, false);

			uint32 WrittenSize = 0;

			if (Manager->GetVoice(VoicePacket.Data.GetData(), WrittenSize))
			{
				VoicePacket.Data.SetNumUninitialized(WrittenSize, false);

				ServerOnVoice(VoicePacket);
			}

			/*FSteamworksVoicePacket& VoicePacket = TempVoicePacket;
			VoicePacket.Data.SetNumUninitialized(STEAMWORKS_VOICE_BUFFER_SIZE, false);

			uint32 WrittenSize = 0;

			EVoiceResult Result = SteamUser()->GetVoice(true, VoicePacket.Data.GetData(), STEAMWORKS_TICK_VOICE_BUFFER_SIZE, &WrittenSize, false, nullptr, 0, nullptr, 0);

			ensure(Result == k_EVoiceResultOK || Result == k_EVoiceResultNoData || Result == k_EVoiceResultNotRecording);

			if (Result == k_EVoiceResultOK)
			{
				VoicePacket.Data.SetNumUninitialized(WrittenSize, false);

				ServerOnVoice(VoicePacket);
			}
			else
			if (k_EVoiceResultNotRecording)
			{
				bRecordingVoice = false;
			}*/
		}
	}
}

void USteamVoiceComponent::OnVoiceFinished()
{
	OnVoiceChanged.Broadcast(false, false);
	bVoiceActive = false;
}

void USteamVoiceComponent::Talk()
{
	bRecordingVoice = true;

	if (SteamUser())
	{
		SteamUser()->StartVoiceRecording();
	}

	OnVoiceChanged.Broadcast(true, true);
}

void USteamVoiceComponent::ShutUp()
{
	//bRecordingVoice = false;

	if (SteamUser())
	{
		SteamUser()->StopVoiceRecording();
	}

	OnVoiceChanged.Broadcast(false, true);
}

bool USteamVoiceComponent::ServerOnVoice_Validate(const FSteamworksVoicePacket& VoiceData)
{
	return true;
}

void USteamVoiceComponent::ServerOnVoice_Implementation(const FSteamworksVoicePacket& VoicePacket)
{
	//Send to relevant receipments

	/*for (TObjectIterator<USteamVoiceComponent> Itr; Itr; ++Itr)
	{
		if (Itr->GetWorld() == GetWorld() && *Itr != this && Itr->IsPendingKillOrUnreachable() == false)
		{
			Itr->MulticastOnVoice(VoicePacket);
		}
	}*/

	MulticastOnVoice(VoicePacket);
}

void USteamVoiceComponent::MulticastOnVoice_Implementation(const FSteamworksVoicePacket& VoicePacket)
{
	SCOPE_CYCLE_COUNTER(STAT_SteamVoicePlayback);
	if (SteamUser() == nullptr) return;

	APawn* Pawn = Cast<APawn>(GetOwner());

	int32 PlayerIndex = -1;

	if (Pawn && Pawn->PlayerState && Radio)
	{
		AGameStateBase* GameState = UGameplayStatics::GetGameState(this);

		if (GameState)
		{
			for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
			{
				if (GameState->PlayerArray[i] == Pawn->PlayerState)
				{
					PlayerIndex = i;
					break;
				}
			}
		}
	}

	//we shouldn't hear ourselves 
	if (Pawn && Pawn->Controller == UGameplayStatics::GetPlayerController(this, 0))
	{
		//testing...
		/*if (Pawn->PlayerState && Pawn->PlayerState->bIsABot == false)
		{
			for (TObjectIterator<USteamVoiceComponent> Itr; Itr; ++Itr)
			{
				if (Itr->GetWorld() == GetWorld() && *Itr != this && Itr->IsPendingKillOrUnreachable() == false)
				{
					Itr->MulticastOnVoice_Implementation(VoicePacket);
				}
			}
		}*/
		return;
	}


	if (ShouldPlayback() == false) return;


	static uint8 Buffer[STEAMWORKS_RAW_VOICE_BUFFER_SIZE];
	uint32 WrittenSize = 0;


	if (Manager && Manager->DecompressVoice(VoicePacket.Data.GetData(), VoicePacket.Data.Num(), Buffer, WrittenSize))
	{
		SoundStreaming->QueueAudio(Buffer, WrittenSize);

		if (Radio && bTalkingInRadio)
		{
			Radio->PushRadioAudio(Buffer, WrittenSize, PlayerIndex);
		}
	}

	/*EVoiceResult Result = SteamUser()->DecompressVoice(VoicePacket.Data.GetData(), VoicePacket.Data.Num(), Buffer, 28672, &WrittenSize, SoundStreaming->SampleRate);

	if (Result == k_EVoiceResultOK && WrittenSize > 0)
	{
		SoundStreaming->QueueAudio(Buffer, WrittenSize);

		if (Radio && bTalkingInRadio)
		{
			Radio->PushRadioAudio(Buffer, WrittenSize, PlayerIndex);
		}
	}*/

	if (IsPlaying() == false)
	{
		Play();
	}

	if (bVoiceActive == false)
	{

		OnVoiceChanged.Broadcast(true, false);

		if (VoiceFinishTimer.IsValid())
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(VoiceFinishTimer);
		}

		GetOwner()->GetWorldTimerManager().SetTimer(VoiceFinishTimer, this, &USteamVoiceComponent::OnVoiceFinished, 1.f, false);

		bVoiceActive = true;
	}
}


bool USteamVoiceComponent::ServerToggleRadio_Validate(bool bToggled)
{
	return true;
}

void USteamVoiceComponent::ServerToggleRadio_Implementation(bool bToggled)
{
	ToggleRadio(bToggled);
}

void USteamVoiceComponent::ToggleRadio(bool bEnabled)
{
	if (GetOwner()->HasAuthority() == false) return ServerToggleRadio(bEnabled);

	if (bTalkingInRadio == bEnabled) return;

	bTalkingInRadio = bEnabled;
	UpdateRadioTalkingState();
}

void USteamVoiceComponent::SetRadio(bool bEnabled)
{
	if ((Radio != nullptr) == bEnabled) return;

	if (bEnabled)
	{
		Radio = GetSteamRadioInstance();
	}
	else
	{
		Radio = nullptr;
	}
}

void USteamVoiceComponent::UpdateRadioTalkingState()
{
	if (Radio)
	{				
		bool bLocalPlayer = false;

		APawn* Pawn = Cast<APawn>(GetOwner());
		
		if (Pawn && Pawn->Controller && Pawn->Controller == UGameplayStatics::GetPlayerController(this, 0))
		{
			bLocalPlayer = true;	
		}

		OnRadioToggle.Broadcast(bTalkingInRadio, bLocalPlayer);
	}
}

void USteamVoiceComponent::OnRep_TalkingInRadio()
{
	UpdateRadioTalkingState();
}

void USteamVoiceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USteamVoiceComponent, bTalkingInRadio);
}




//FSteamworksVoicePacket Implementation

bool FSteamworksVoicePacket::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	if (Ar.IsSaving())
	{
		Ar << Data;
		return true;
	}
	else
	if (Ar.IsLoading())
	{
		Ar << Data;
		return true;
	}

	return false;
}


