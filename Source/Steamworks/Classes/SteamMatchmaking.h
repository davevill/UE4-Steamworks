//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "GameFramework/Info.h"
#include "SteamLobby.h"
#include "SteamMatchmaking.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamOnLobbyListUpdatedSignature, class ASteamMatchmaking*, Matchmaking);


class ASteamLobby;


UENUM(BlueprintType)
enum class ESteamLobbyComparison : uint8
{
	EqualToOrLessThan,
	LessThan,
	Equal,
	GreaterThan,
	EqualToOrGreaterThan,
	NotEqual
};

UENUM(BlueprintType)
enum class ESteamLobbyDistanceFilter : uint8
{
	Close,		// only lobbies in the same immediate region will be returned
	Default,	// only lobbies in the same region or near by regions
	Far,		// for games that don't have many latency requirements, will return lobbies about half-way around the globe
	Worldwide	// no filtering, will match lobbies as far as India to NY (not recommended, expect multiple seconds of latency between the clients)
};


UCLASS()
class STEAMWORKS_API ASteamMatchmaking : public AInfo
{
	GENERATED_BODY()

	bool bRequestingLobbyList;

	class FSteamMatchmakingImpl* Impl;

	friend FSteamMatchmakingImpl;

protected:

public:


	/** Request lobby list populates this list asyncronously */
	UPROPERTY(BlueprintReadOnly, Category="Steam Matchmaking")
	TArray<FSteamLobbyInfo> LobbyList;



	ASteamMatchmaking();



	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;




	
	/** String and numerical filters simply restrict the result set returned */
	UFUNCTION(BlueprintCallable, Category="Steam Matchmaking")
	void AddRequestLobbyListStringFilter(const FString& KeyToMatch, const FString& ValueToMatch, ESteamLobbyComparison ComparisonType);

	/** String and numerical filters simply restrict the result set returned */
	UFUNCTION(BlueprintCallable, Category="Steam Matchmaking")
	void AddRequestLobbyListNumericalFilter(const FString& KeyToMatch, int32 ValueToMatch, ESteamLobbyComparison ComparisonType);

	/** Near filters don't actually filter out values, they just influence how the results are sorted. You can specify multiple near filters, 
	 *  with the first near filter influencing the most, and the last near filter influencing the least. */
	UFUNCTION(BlueprintCallable, Category="Steam Matchmaking")
	void AddRequestLobbyListNearValueFilter(const FString& KeyToMatch, int32 ValueToBeCloseTo);

	/** Specifying the filter slots sets how many open slots you want in the lobby (usefully if you're trying to match groups of people into a lobby). */
	UFUNCTION(BlueprintCallable, Category="Steam Matchmaking")
	void AddRequestLobbyListFilterSlotsAvailable(int32 SlotsAvailable);

	/** sets the distance for which we should search for lobbies (based on users IP address to location map on the Steam backed) */
	UFUNCTION(BlueprintCallable, Category="Steam Matchmaking")
	void AddRequestLobbyListDistanceFilter(ESteamLobbyDistanceFilter DistanceFilter);



	/** Add filters first before calling this, filters are cleared after calling this*/
	UFUNCTION(BlueprintCallable, Category="Steam Matchmaking")
	void RequestLobbyList();







	/** Broadcast if the list is requested or/and if its data changed
	 *  This is a good place to update any UI */
	UPROPERTY(BlueprintAssignable, Category="Steam Matchmaking")
	FSteamOnLobbyListUpdatedSignature OnLobbyListUpdated;


};
