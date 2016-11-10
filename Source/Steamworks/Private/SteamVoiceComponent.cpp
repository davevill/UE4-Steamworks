//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamVoiceComponent.h"
#include "Runtime/Engine/Classes/Sound/SoundWaveProcedural.h"






USteamVoiceComponent::USteamVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

	SetWaveParameter("Voice", SoundStreaming);

	//OnAudioFinished.AddDynamic(this, &USteamVoiceComponent::OnVoiceFinished);
}

void USteamVoiceComponent::UninitializeComponent()
{
	SoundStreaming = nullptr;

	Super::UninitializeComponent();
}

#define STEAMWORKS_TICK_VOICE_BUFFER_SIZE 28672

void USteamVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (bRecordingVoice && SteamUser())
	{
		VoiceBuffer.Empty(STEAMWORKS_TICK_VOICE_BUFFER_SIZE);
		VoiceBuffer.Reserve(STEAMWORKS_TICK_VOICE_BUFFER_SIZE);

		uint32 WrittenSize = 0;

		EVoiceResult Result = SteamUser()->GetVoice(true, VoiceBuffer.GetData(), STEAMWORKS_TICK_VOICE_BUFFER_SIZE, &WrittenSize, false, nullptr, 0, nullptr, 0);

		if (Result == k_EVoiceResultOK)
		{
			VoiceBuffer.SetNumUninitialized(WrittenSize, false);

			ServerOnVoice(VoiceBuffer);
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
	bRecordingVoice = false;

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

	//we shouldn't hear ourselves 
	if (Pawn && Pawn->Controller == UGameplayStatics::GetPlayerController(this, 0))
	{

		/*
		//testing...
		if (Pawn->PlayerState && Pawn->PlayerState->bIsABot == false)
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




