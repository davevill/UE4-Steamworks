//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "Components/AudioComponent.h"
#include "SteamVoiceComponent.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSteamOnVoiceChangedSignature, bool, bTalking, bool, bLocalPlayer);




/** Represents a voice in the game, if part of a local-controlled pawn it can send voice to the server, 
  * otherwise will playback other player's voice */
UCLASS(ClassGroup=(Steamworks), meta=(BlueprintSpawnableComponent), Config=Game)
class STEAMWORKS_API USteamVoiceComponent : public UAudioComponent
{
	GENERATED_BODY()
protected:


	UPROPERTY()
	class USoundWaveProcedural* SoundStreaming;


	TArray<uint8> VoiceBuffer;

	bool bRecordingVoice;

	UFUNCTION()
	virtual void OnVoiceFinished();

	UPROPERTY()
	bool bVoiceActive;

	FTimerHandle VoiceFinishTimer;

public:





	USteamVoiceComponent();


	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	/** If owned will start voice playback */
	UFUNCTION(BlueprintCallable, Category="Steam Voice Component")
	void Talk();

	/** If owned will stop voice playback */
	UFUNCTION(BlueprintCallable, Category="Steam Voice Component")
	void ShutUp();


	/** Send compressed voice data to the server*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnVoice(const TArray<uint8>& VoiceData);

	/** Receive compressed voice data from the server */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastOnVoice(const TArray<uint8>& VoiceData);


	UFUNCTION(BlueprintPure, Category="Steam Voice Component")
	bool IsTalking() const { return true; }



	/** Called whenever the voice state has changed (taling/not-talking) */
	UPROPERTY(BlueprintAssignable, Category="Steam Voice Component")
	FSteamOnVoiceChangedSignature OnVoiceChanged;

};
