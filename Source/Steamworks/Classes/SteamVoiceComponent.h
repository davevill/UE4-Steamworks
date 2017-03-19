//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "Components/AudioComponent.h"
#include "SteamVoiceComponent.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSteamOnVoiceChangedSignature, bool, bTalking, bool, bLocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSteamOnRadioToggleSignature, bool, bToggled, bool, bLocalPlayer);






USTRUCT()
struct FSteamworksVoicePacket
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<uint8> Data;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FSteamworksVoicePacket> : public TStructOpsTypeTraitsBase
{
	enum
	{
		WithNetSerializer = true
	};
};





/** Represents a voice in the game, if part of a local-controlled pawn it can send voice to the server, 
  * otherwise will playback other player's voice */
UCLASS(ClassGroup=(Steamworks), meta=(BlueprintSpawnableComponent), Config=Game)
class STEAMWORKS_API USteamVoiceComponent : public UAudioComponent
{
	GENERATED_BODY()

	FSteamworksVoicePacket TempVoicePacket;	

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


	virtual void UpdateRadioTalkingState();

	virtual class ASteamRadio* CreateSteamRadioInstance();
	virtual class ASteamRadio* GetSteamRadioInstance();

	UFUNCTION()
	void OnRep_TalkingInRadio();

	UPROPERTY()
	class USteamworksManager* Manager;

public:

	UPROPERTY(BlueprintReadOnly, Category="Steam Voice Component")
	class ASteamRadio* Radio;

	UPROPERTY(BlueprintReadOnly, Category="Steam Voice Component", ReplicatedUsing=OnRep_TalkingInRadio)
	bool bTalkingInRadio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Steam Voice Component")
	bool bOpenMic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Steam Voice Component")
	USoundCue* RadioVoiceCue;



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

	/* Whether to join the radio channel or not */
	UFUNCTION(BlueprintCallable, Category="Steam Voice Component")
	void SetRadio(bool bEnabled);

	/* Whether to send radio data */
	UFUNCTION(BlueprintCallable, Category="Steam Voice Component")
	void ToggleRadio(bool bEnabled);

	/** Send compressed voice data to the server*/
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerOnVoice(const FSteamworksVoicePacket& VoicePacket);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerToggleRadio(bool bToggled);

	/** Receive compressed voice data from the server */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastOnVoice(const FSteamworksVoicePacket& VoicePacket);


	UFUNCTION(BlueprintPure, Category="Steam Voice Component")
	bool IsTalking() const { return true; }



	/** Called whenever the voice state has changed (taling/not-talking) */
	UPROPERTY(BlueprintAssignable, Category="Steam Voice Component")
	FSteamOnVoiceChangedSignature OnVoiceChanged;

	UPROPERTY(BlueprintAssignable, Category="Steam Voice Component")
	FSteamOnRadioToggleSignature OnRadioToggle;

};
