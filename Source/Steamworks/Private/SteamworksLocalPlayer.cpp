//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamworksLocalPlayer.h"
#include "Runtime/Core/Public/Misc/Base64.h"




FString USteamworksLocalPlayer::GetGameLoginOptions() const
{
	return FString();

	FString TicketOption = "ticket=";
	
	uint32 ByteSize;

	TArray<uint8> Buffer;
	Buffer.SetNumZeroed(1024);

	if (SteamUser())
	{
		HAuthTicket Ticket = SteamUser()->GetAuthSessionTicket(Buffer.GetData(), Buffer.Num(), &ByteSize);


		if (Ticket)
		{
			check(ByteSize < (uint32)Buffer.Num());
			check(ByteSize > 0);

			Buffer.RemoveAt(ByteSize, Buffer.Num() - ByteSize);

			check(Buffer.Num() == ByteSize);

			TicketOption += FBase64::Encode(Buffer);
		}
	}
	else
	{
		TicketOption += "invalid";
	}

	return TicketOption;
}
