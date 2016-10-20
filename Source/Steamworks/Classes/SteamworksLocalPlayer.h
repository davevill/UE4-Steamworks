//Copyright 2016 davevillz, https://github.com/davevill

#pragma once
#include "Engine/LocalPlayer.h"
#include "SteamworksLocalPlayer.generated.h"






UCLASS(config = Engine, transient)
class STEAMWORKS_API USteamworksLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:


	virtual FString GetGameLoginOptions() const override;
};
