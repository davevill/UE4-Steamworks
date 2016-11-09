//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamworksStatics.generated.h"



UCLASS()
class STEAMWORKS_API USteamworksStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:



	UFUNCTION(Category = "Steamworks", BlueprintCallable, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void CreateLobby(UObject* WorldContextObject);

	UFUNCTION(Category = "Steamworks", BlueprintCallable, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void FindLobbies(UObject* WorldContextObject);




	UFUNCTION(Category = "Steamworks", BlueprintPure, meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static FString GetLocalSteamId(UObject* WorldContextObject);



};
