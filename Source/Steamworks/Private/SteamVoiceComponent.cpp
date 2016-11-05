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

	if (SteamUser() == nullptr) return;

	SoundStreaming = NewObject<USoundWaveProcedural>();
	SoundStreaming->SampleRate = SteamUser()->GetVoiceOptimalSampleRate();
	SoundStreaming->NumChannels = 1;
	SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
	SoundStreaming->SoundGroup = SOUNDGROUP_Voice;
	SoundStreaming->bLooping = true;

	SetWaveParameter("Voice", SoundStreaming);
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


	if (bRecordingVoice)
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

	if (IsPlaying() == false)
	{
		Play();
	}
}

void USteamVoiceComponent::Talk()
{
	bRecordingVoice = true;
	SteamUser()->StartVoiceRecording();
}

void USteamVoiceComponent::ShutUp()
{
	bRecordingVoice = false;
	SteamUser()->StopVoiceRecording();
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
	uint8 Buffer[28672];
	uint32 WrittenSize = 0;

	EVoiceResult Result = SteamUser()->DecompressVoice(VoiceData.GetData(), VoiceData.Num(), Buffer, 28672, &WrittenSize, SoundStreaming->SampleRate);

	if (Result == k_EVoiceResultOK && WrittenSize > 0)
	{
		SoundStreaming->QueueAudio(Buffer, WrittenSize);
	}
}




