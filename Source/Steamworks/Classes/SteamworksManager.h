//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "Object.h"
#include "Tickable.h"
#include "Steamworks.h"
#include "SteamLobby.h"
#include "SteamworksManager.generated.h"



UENUM()
enum class ESteamLobbyChatMemberStateChange : uint8
{
	Entered,
	Left,
	Disconnected,
	Kicked,
	Banned		
};




DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSteamOnInventoryUpdateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSteamOnLobbyListUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSteamOnLobbyUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamOnEnterLobbySignature, bool, bFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamOnLeftLobbySignature, ESteamLobbyChatMemberStateChange, StateChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSteamOnLobbyCreatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSteamOnLobbyMembersUpdateSignature);


#define STEAMWORKS_VOICE_BUFFER_SIZE 51200
#define STEAMWORKS_RAW_VOICE_BUFFER_SIZE 102400
#define STEAMWORKS_TICK_VOICE_BUFFER_SIZE 8192

#define STEAMWORKS_LOBBY_VOICE_CHANNEL 10



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
class STEAMWORKS_API USteamworksManager : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

	class FSteamworksCallbacks* Callbacks;
	friend class FSteamworksCallbacks;


	TSharedPtr<class IVoiceCapture> VoiceCapture;
	TSharedPtr<class IVoiceEncoder> VoiceEncoder;
	TSharedPtr<class IVoiceDecoder> VoiceDecoder;

	TArray<uint8> RawCaptureData;
	int32 MaxRawCaptureDataSize;
	TArray<uint8> CompressedData;
	int32 MaxCompressedDataSize;
	TArray<uint8> UncompressedData;
	int32 MaxUncompressedDataSize;
	TArray<uint8> Remainder;
	int32 MaxRemainderSize;
	int32 LastRemainderSize;


protected:

	bool bInitialized;

	UPROPERTY()
	class UGameInstance* GameInstance;

	UPROPERTY()
	TMap<uint64, UTexture2D*> Avatars;


	bool LoadSteamAvatar(CSteamID SteamId, UTexture2D* AvatarTexture) const;

	void CopySteamAvatar(int Handle, UTexture2D* AvatarTexture) const;



	bool bRecordingVoice;


	bool GetVoice_Implementation(uint8* DestBuffer, uint32& WrittenSize);


	friend class USteamLobby;

public:


	/** This is always the latest inventory after calling GetAllItems */
	UPROPERTY(BlueprintReadOnly, Category="Steamworks")
	TArray<class USteamItem*> Inventory;



	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bInitialized; }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }

	
	virtual class UWorld* GetWorld() const override;


	/** Must be called within the GameInstance::Init method */
	virtual void Init();

	/** Must be called within the GameInstance::Shutdown method */
	virtual void Shutdown();


	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Steamworks Manager", WorldContext = "WorldContextObject"), Category = "Steamworks")
	static USteamworksManager* Get(UObject* WorldContextObject);


	/** Gets the avatar for the provided player state, asyncronous and safe to call every frame 
	 *  might return a non-visible (opacity = 0.0) image if the texture is not imediatly available,
	 *  however it's guaranteed to get updated once it arrives from steam api.
	 *  Fail over it's returned if the player state or steam id is invalid */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	UTexture2D* GetAvatar(class APlayerState* PlayerState, UTexture2D* FailoverTexture = nullptr);


	/** Gets Avatar directly from a SteamID, return nullptr if SteamId is invalid */
	UTexture2D* GetAvatarBySteamId(CSteamID SteamId);




	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void GetAllInventoryItems();


	UFUNCTION(BlueprintPure, Category="Steamworks")
	bool HasInstanceOf(int32 DefinitionId) const;


	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnInventoryUpdateSignature OnInventoryUpdate;




	/** Sets the voice recording enabled, Must call GetVoice everyframe afterwards */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void SetVoiceRecording(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="Steamworks")
	inline bool IsRecordingVoice() const { return bRecordingVoice; }




	/** Fills the buffer with voice data, the size of the buffer is set in the WrittenSize param */
	bool GetVoice(uint8* DestBuffer, uint32& WrittenSize);

	/** Decompress voice data into raw PCM with VOICE_SAMPLE_RATE as sample rate */
	bool DecompressVoice(const uint8* CompressedBuffer, uint32 CompressedSize, uint8* DestBuffer, uint32& WrittenSize);




	// Matchmaking & Lobbies

protected:


	UPROPERTY()
	class USteamLobby* LobbyInstance;


	bool bRequestingLobbyList;
	bool bRequestLobbyData;


	void CreateLobbyInstance(CSteamID LobbyId);


public:


	/** The lobby list, gets populated by calling RequestLobbyList */
	UPROPERTY(BlueprintReadOnly, Category="Steamworks")
	TArray<FSteamLobbyInfo> LobbyList;

	/** The lobby class to use while creating the instance */
	UPROPERTY(BlueprintReadWrite, Category="Steamworks")
	TSubclassOf<USteamLobby> LobbyClass;



	/** Called when the lobby list has been updated from a call to RequestLobbyList */
	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnLobbyListUpdatedSignature OnLobbyListUpdated;

	/** Called if the lobby instance has been changed, ie. created, joined or left */
	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnLobbyUpdatedSignature OnLobbyUpdated;

	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnEnterLobbySignature OnEnterLobby;

	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnLeftLobbySignature OnLeftLobby;

	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnLobbyCreatedSignature OnLobbyCreated;

	/** Calls whenever the lobby member list is updated */
	UPROPERTY(BlueprintAssignable, Category="Steamworks")
	FSteamOnLobbyMembersUpdateSignature OnLobbyMembersUpdate;








	/** Request lobby list, filters must be re-set/set in advance */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void RequestLobbyList(bool bWithData = false);



	/** String and numerical filters simply restrict the result set returned */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void AddRequestLobbyListStringFilter(const FString& KeyToMatch, const FString& ValueToMatch, ESteamLobbyComparison ComparisonType);

	/** String and numerical filters simply restrict the result set returned */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void AddRequestLobbyListNumericalFilter(const FString& KeyToMatch, int32 ValueToMatch, ESteamLobbyComparison ComparisonType);

	/** Near filters don't actually filter out values, they just influence how the results are sorted. You can specify multiple near filters, 
	 *  with the first near filter influencing the most, and the last near filter influencing the least. */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void AddRequestLobbyListNearValueFilter(const FString& KeyToMatch, int32 ValueToBeCloseTo);

	/** Specifying the filter slots sets how many open slots you want in the lobby (usefully if you're trying to match groups of people into a lobby). */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void AddRequestLobbyListFilterSlotsAvailable(int32 SlotsAvailable);

	/** sets the distance for which we should search for lobbies (based on users IP address to location map on the Steam backed) */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void AddRequestLobbyListDistanceFilter(ESteamLobbyDistanceFilter DistanceFilter);


	/** Creates a new lobby */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void CreateLobby(int32 LobbyMemberLimit, bool bPublic = true);


	/** Joins a lobby, OnLobbyUpdated will be called, you should update your UI based on this. See also GetLobbyInstance and the Lobby Class itself */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void JoinLobby(FSteamLobbyInfo LobbyInfo);

	/** Leaves imediatly the current lobby thus making GetLobbyInstance return null */
	UFUNCTION(BlueprintCallable, Category="Steamworks")
	void LeaveLobby();




	/** Returns the lobby instance, either created or joined */
	UFUNCTION(BlueprintPure, Category="Steamworks")
	USteamLobby* GetLobbyInstance() const { return LobbyInstance; }



};
