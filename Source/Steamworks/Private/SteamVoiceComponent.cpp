//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamVoiceComponent.h"
#include "Runtime/Engine/Classes/Sound/SoundWaveProcedural.h"
#include "SteamRadio.h"






USteamVoiceComponent::USteamVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.bRunOnAnyThread = true;
	PrimaryComponentTick.TickInterval = 1.f / (90 / 2.f);
	bWantsInitializeComponent = true;

	bReplicates = true;
	bRecordingVoice = false;
}

void USteamVoiceComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SoundStreaming = NewObject<USoundWaveProcedural>();
	SoundStreaming->SampleRate = SteamUser() == nullptr ? 11025 : SteamUser()->GetVoiceOptimalSampleRate();
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


	//ensure(Channel);

	if (bOpenMic)
	{
		//Talk();
	}
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

	APawn* Pawn = Cast<APawn>(GetOwner());

	if (Pawn && Pawn->Controller == UGameplayStatics::GetPlayerController(this, 0))
	{
		if (bOpenMic && SteamUser())
		{
			SteamUser()->StopVoiceRecording();
		}
	}

	Super::UninitializeComponent();
}

#define STEAMWORKS_TICK_VOICE_BUFFER_SIZE 1024

void USteamVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* Pawn = Cast<APawn>(GetOwner());

	//we shouldn't hear ourselves 
	if (Pawn && Pawn->Controller == UGameplayStatics::GetPlayerController(this, 0))
	{
		if (bOpenMic && SteamUser() && bRecordingVoice == false)
		{
			SteamUser()->StartVoiceRecording();
			bRecordingVoice = true;
		}

		if (bRecordingVoice && SteamUser())
		{
			uint32 AvailableSize = 0;

			EVoiceResult PeekResult = SteamUser()->GetAvailableVoice(&AvailableSize, nullptr, 0);

			if (AvailableSize > 0)
			{
				VoiceBuffer.SetNumUninitialized(STEAMWORKS_TICK_VOICE_BUFFER_SIZE, false);

				//VoiceBuffer.Empty(STEAMWORKS_TICK_VOICE_BUFFER_SIZE);
				//VoiceBuffer.Reserve(STEAMWORKS_TICK_VOICE_BUFFER_SIZE);

				//uint8 Buffer[STEAMWORKS_TICK_VOICE_BUFFER_SIZE];

				uint32 WrittenSize = 0;

				EVoiceResult Result = SteamUser()->GetVoice(true, VoiceBuffer.GetData(), STEAMWORKS_TICK_VOICE_BUFFER_SIZE, &WrittenSize, false, nullptr, 0, nullptr, 0);

				ensure(Result == k_EVoiceResultOK || Result == k_EVoiceResultNoData || Result == k_EVoiceResultNotRecording);

				if (Result == k_EVoiceResultOK)
				{
					VoiceBuffer.SetNumUninitialized(WrittenSize, false);

					ServerOnVoice(VoiceBuffer);
				}
				else
				if (k_EVoiceResultNotRecording)
				{
					bRecordingVoice = false;
				}
			}
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

bool USteamVoiceComponent::ServerOnVoice_Validate(const TArray<uint8>& VoiceData)
{
	return true;
}

void USteamVoiceComponent::ServerOnVoice_Implementation(const TArray<uint8>& VoiceData)
{
	//Send to relevant receipments
	MulticastOnVoice(VoiceData);
}

void USteamVoiceComponent::MulticastOnVoice_Implementation(const TArray<uint8>& VoiceData)
{
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
					Itr->MulticastOnVoice_Implementation(VoiceData);
				}
			}
		}*/
		return;
	}

	uint8 Buffer[28672];
	uint32 WrittenSize = 0;

	EVoiceResult Result = SteamUser()->DecompressVoice(VoiceData.GetData(), VoiceData.Num(), Buffer, 28672, &WrittenSize, SoundStreaming->SampleRate);

	if (Result == k_EVoiceResultOK && WrittenSize > 0)
	{
		SoundStreaming->QueueAudio(Buffer, WrittenSize);

		if (Radio && bTalkingInRadio)
		{
			Radio->PushRadioAudio(Buffer, WrittenSize, PlayerIndex);
		}
	}

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





