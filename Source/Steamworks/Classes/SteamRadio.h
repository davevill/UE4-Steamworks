//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "GameFramework/Info.h"
#include "SteamRadio.generated.h"


USTRUCT()
struct FSteamRadioPlaybackChannel
{
	GENERATED_USTRUCT_BODY();
public:

	UPROPERTY()
	UAudioComponent* SoundComponent;

	UPROPERTY()
	class USoundWaveProcedural* SoundStreaming;

};


UCLASS(Blueprintable)
class STEAMWORKS_API ASteamRadio : public AInfo
{
	GENERATED_BODY()
public:


	/* Do not confuse with radio channels, this is a playback channel per player*/
	UPROPERTY(BlueprintReadOnly, Category="Steam Radio")
	TArray<FSteamRadioPlaybackChannel> PlaybackChannels;


	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Steam Radio")



	ASteamRadio();



	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category="Steam Radio")
	void InitializePlaybackChannels(USoundCue* ChannelCue);

	void PushRadioAudio(uint8* Data, int32 DataSize, int32 PlayerIndex);
};
