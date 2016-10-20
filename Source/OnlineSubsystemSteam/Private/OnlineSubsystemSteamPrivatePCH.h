//Copyright 2016 davevillz, https://github.com/davevill

#pragma once

#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSubsystemSteam.h"
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "OnlineSubsystemTypes.h"
#include "IPAddress.h"
#include "OnlineSubsystem.h"
#include "ModuleManager.h"

#define INVALID_INDEX -1

/** URL Prefix when using Null socket connection */
#define NULL_URL_PREFIX TEXT("Steam.")

/** pre-pended to all NULL logging */
#undef ONLINE_LOG_PREFIX
#define ONLINE_LOG_PREFIX TEXT("Steam: ")

