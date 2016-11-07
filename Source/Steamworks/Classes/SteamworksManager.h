//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "Object.h"
#include "Tickable.h"
#include "Steamworks.h"
#include "SteamworksManager.generated.h"











UCLASS()
class STEAMWORKS_API USteamworksManager : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

	class FSteamworksCallbacks* Callbacks;
	friend class FSteamworksCallbacks;

protected:

	/* Current lobby id */
	CSteamID LobbyId;

	bool bInitialized;

	UPROPERTY()
	class UGameInstance* GameInstance;

	UPROPERTY()
	TMap<uint64, UTexture2D*> Avatars;


	void CopySteamAvatar(int Handle, UTexture2D* AvatarTexture) const;


public:


	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bInitialized; }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }

	
	virtual class UWorld* GetWorld() const override;


	/** Must be called within the GameInstance::Init method */
	virtual void Init();

	/** Must be called within the GameInstance::Shutdown method */
	virtual void Shutdown();


	/** Used internally to keep track of the lobby status */
	inline CSteamID GetLobbyId() { return LobbyId; }
	inline void SetLobbyId(CSteamID Id) { LobbyId = Id; }


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

};
