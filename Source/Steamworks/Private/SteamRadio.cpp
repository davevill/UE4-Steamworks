//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamRadio.h"
#include "Runtime/Engine/Classes/Sound/SoundWaveProcedural.h"








ASteamRadio::ASteamRadio()
{
}

void ASteamRadio::BeginPlay()
{
	Super::BeginPlay();
}

void ASteamRadio::InitializePlaybackChannels(USoundCue* ChannelCue)
{
	for (int32 i = 0; i < 8; i++)
	{
		FSteamRadioPlaybackChannel Channel;

		Channel.SoundComponent = NewObject<UAudioComponent>(this);

		Channel.SoundStreaming = NewObject<USoundWaveProcedural>();
		Channel.SoundStreaming->SampleRate = SteamUser() == nullptr ? 11025 : SteamUser()->GetVoiceOptimalSampleRate();
		Channel.SoundStreaming->NumChannels = 1;
		Channel.SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
		Channel.SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
		Channel.SoundStreaming->bLooping = true;
		Channel.SoundStreaming->bProcedural = true;

		if (Channel.SoundComponent)
		{
			Channel.SoundComponent->SetSound(ChannelCue);
			Channel.SoundComponent->RegisterComponent();
			Channel.SoundComponent->SetWaveParameter("Voice", Channel.SoundStreaming);
		}

		PlaybackChannels.Push(Channel);
	}
}

void ASteamRadio::BeginDestroy()
{
	Super::BeginDestroy();
}

void ASteamRadio::PushRadioAudio(uint8* Data, int32 DataSize, int32 PlayerIndex)
{
	FSteamRadioPlaybackChannel* Channel = nullptr;

	if (PlayerIndex >= 0 && PlayerIndex < 8)
	{
		Channel = &PlaybackChannels[PlayerIndex];
	}

	if (Channel)
	{
		if (Channel->SoundStreaming)
		{
			Channel->SoundStreaming->QueueAudio(Data, DataSize);
		}

		if (Channel->SoundComponent)
		{
			if (Channel->SoundComponent->IsPlaying() == false)
			{
				Channel->SoundComponent->Play();
			}
		}
	}

}


	


