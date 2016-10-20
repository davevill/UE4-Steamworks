//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "Engine/GameInstance.h"
#include "SteamworksGameInstance.generated.h"



UCLASS()
class STEAMWORKS_API USteamworksGameInstance : public UGameInstance
{
	GENERATED_BODY()

	FTimerHandle PollHandle;


protected:

	/** Steamcalls have a latency, we poll every second to check its status */
	UFUNCTION()
	virtual void Poll();

public:

	USteamworksGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	/** Called when the public address is resolved */
	virtual void OnPublicAddressResolved(FString IpString);


	virtual ULocalPlayer* CreateInitialPlayer(FString& OutError) override;
};
