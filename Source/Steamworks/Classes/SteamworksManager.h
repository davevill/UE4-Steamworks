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

protected:

	/* Current lobby id */
	CSteamID LobbyId;

	bool bInitialized;

	UPROPERTY()
	class UGameInstance* GameInstance;

public:


	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bInitialized; }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }

	
	virtual class UWorld* GetWorld() const override;


	/** Must be called within the GameInstance::Init method */
	virtual void Init();

	/** Must be called within the GameInstance::Shutdown method */
	virtual void Shutdown();


	/* Used internally to keep track of the lobby status */
	inline CSteamID GetLobbyId() { return LobbyId; }
	inline void SetLobbyId(CSteamID Id) { LobbyId = Id; }


	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Steamworks Manager", WorldContext = "WorldContextObject"), Category = "Steamworks")
	static USteamworksManager* Get(UObject* WorldContextObject);
};
