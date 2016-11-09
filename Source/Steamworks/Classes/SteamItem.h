//Copyright 2016 davevillz, https://github.com/davevill



#pragma once

#include "Object.h"
#include "Steamworks.h"
#include "SteamItem.generated.h"



/** The base classs for the inventory service item */
UCLASS()
class STEAMWORKS_API USteamItem : public UObject
{
	GENERATED_BODY()
public:


	/** Details from the Steam API */
	SteamItemDetails_t Details;



	UFUNCTION(BlueprintPure, Category="Steam Item")
	FName GetType() const;

};
