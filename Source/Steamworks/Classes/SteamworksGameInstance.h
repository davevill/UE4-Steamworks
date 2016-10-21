//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "Engine/GameInstance.h"
#include "Steamworks.h"
#include "SteamworksGameInstance.generated.h"



UCLASS()
class STEAMWORKS_API USteamworksGameInstance : public UGameInstance
{
	GENERATED_BODY()

	FTimerHandle PollHandle;


	class FSteamworksCallbacks* Callbacks;

protected:

	/** Steamcalls have a latency, we poll every second to check its status */
	UFUNCTION()
	virtual void Poll();

	/* Current lobby id */
	CSteamID LobbyId;

public:

	USteamworksGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	/** Called when the public address is resolved */
	virtual void OnPublicAddressResolved(FString IpString);


	inline CSteamID GetLobbyId() { return LobbyId; }
	inline void SetLobbyId(CSteamID Id) { LobbyId = Id; }


	virtual ULocalPlayer* CreateInitialPlayer(FString& OutError) override;
};
