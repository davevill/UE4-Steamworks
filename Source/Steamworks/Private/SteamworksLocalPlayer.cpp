//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksLocalPlayer.h"
#include "Runtime/Core/Public/Misc/Base64.h"




FString USteamworksLocalPlayer::GetGameLoginOptions() const
{
	FString TicketOption = "ticket=";
	
	uint32 Size;

	TArray<uint8> Buffer;
	Buffer.SetNumZeroed(1024);

	if (SteamUser())
	{
		HAuthTicket Ticket = SteamUser()->GetAuthSessionTicket(Buffer.GetData(), Buffer.Num(), &Size);


		if (Ticket)
		{
			check(Size < (uint32)Buffer.Num());
			check(Size > 0);

			Buffer.RemoveAt(Size, Buffer.Num() - Size);

			check(Buffer.Num() == Size);

			TicketOption += FBase64::Encode(Buffer);
		}
	}
	else
	{
		TicketOption += "invalid";
	}

	return TicketOption;
}
