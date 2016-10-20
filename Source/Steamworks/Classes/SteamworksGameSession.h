//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "GameFramework/GameSession.h"
#include "SteamworksGameSession.generated.h"






UCLASS()
class STEAMWORKS_API ASteamworksGameSession : public AGameSession
{
	GENERATED_BODY()


	TArray<uint64> Whitelist;

public:



	virtual FString ApproveLogin(const FString& Options) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;


};
