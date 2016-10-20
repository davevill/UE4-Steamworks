//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "GameFramework/GameMode.h"
#include "SteamworksGameMode.generated.h"






UCLASS()
class STEAMWORKS_API ASteamworksGameMode : public AGameMode
{

	GENERATED_BODY()


protected:


public:




	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void BeginDestroy() override;


	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;


	virtual TSubclassOf<class AGameSession> GetGameSessionClass() const override;
};
