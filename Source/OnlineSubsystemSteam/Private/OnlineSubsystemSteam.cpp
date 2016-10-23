//Copyright 2016 davevillz, https://github.com/davevill


#include "OnlineSubsystemSteamPrivatePCH.h"
#include "OnlineSubsystemNull.h"
#include "OnlineIdentityInterface.h"
#include "OnlineIdentitySteam.h"





#define LOCTEXT_NAMESPACE "FOnlineSubsystemSteamModule"


typedef TSharedPtr<class FOnlineIdentitySteam, ESPMode::ThreadSafe> FOnlineIdentitySteamPtr;

class FOnlineSubsystemSteam : public FOnlineSubsystemNull
{
public:

	FOnlineIdentitySteamPtr IdentityInterfaceSteam;

	virtual IOnlineIdentityPtr GetIdentityInterface() const override
	{
		return IdentityInterfaceSteam;
	}

	virtual bool Init() override
	{
		if (FOnlineSubsystemNull::Init())
		{
			IdentityInterfaceSteam = MakeShareable(new FOnlineIdentitySteam(this));
			return true;
		}

		return false;
	}

	virtual bool Shutdown() override
	{
		if (IdentityInterfaceSteam.IsValid())
		{
			ensure(IdentityInterfaceSteam.IsUnique());
			IdentityInterfaceSteam = nullptr;
		}

		return FOnlineSubsystemNull::Shutdown();
	}
	
	FOnlineSubsystemSteam(FName InInstanceName) : 
		FOnlineSubsystemNull(InInstanceName)
	{

	}

};


typedef TSharedPtr<FOnlineSubsystemSteam, ESPMode::ThreadSafe> FOnlineSubsystemSteamPtr;



class FOnlineFactorySteam : public IOnlineFactory
{
public:

	FOnlineFactorySteam() {}
	virtual ~FOnlineFactorySteam() {}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName)
	{
		FOnlineSubsystemSteamPtr OnlineSub = MakeShareable(new FOnlineSubsystemSteam(InstanceName));

		if (!OnlineSub->Init())
		{
			UE_LOG_ONLINE(Warning, TEXT("Steam API failed to initialize!"));
			OnlineSub->Shutdown();
			OnlineSub = NULL;
		}

		return OnlineSub;
	}
};


void FOnlineSubsystemSteamModule::StartupModule()
{
	SteamFactory = new FOnlineFactorySteam();

	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService("Steam", SteamFactory);
}

void FOnlineSubsystemSteamModule::ShutdownModule()
{
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService("Steam");

	delete SteamFactory;
	SteamFactory = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOnlineSubsystemSteamModule, OnlineSubsystemSteam)
