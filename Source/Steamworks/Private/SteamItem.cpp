//Copyright 2016 davevillz, https://github.com/davevill

#include "SteamworksPrivatePCH.h"
#include "SteamItem.h"







FName USteamItem::GetType() const
{
	char ValueBuffer[1024 * 10];
	uint32 ValueSize = sizeof(ValueBuffer);

	if (SteamInventory() && SteamInventory()->GetItemDefinitionProperty(Details.m_iDefinition, "type", ValueBuffer, &ValueSize))
	{
		if (ValueSize)
		{
			return ValueBuffer;
		}
	}

	return NAME_None;
}


