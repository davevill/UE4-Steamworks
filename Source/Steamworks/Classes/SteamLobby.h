//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "GameFramework/Info.h"
#include "Steamworks.h"
#include "SteamLobby.generated.h"


USTRUCT(BlueprintType)
struct FSteamLobbyDataEntry
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FString Key;

	UPROPERTY(BlueprintReadWrite)
	FString Value;
};

USTRUCT(BlueprintType)
struct FSteamLobbyInfo
{
	GENERATED_USTRUCT_BODY()

public:

	CSteamID Id;

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	TArray<FSteamLobbyDataEntry> Metadata;

	FSteamLobbyInfo()
	{

	}


	void UpdateData(bool bPassive = false);

};



UCLASS()
class STEAMWORKS_API ASteamLobby : public AInfo
{
	GENERATED_BODY()


protected:

	UPROPERTY(BlueprintReadOnly, Category="Steam Lobby")
	FSteamLobbyInfo Info;



};
