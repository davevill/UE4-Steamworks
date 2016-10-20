//Copyright 2016 davevillz, https://github.com/davevill

using System.IO;
using UnrealBuildTool;
using System;


public class OnlineSubsystemSteam : ModuleRules
{
	public OnlineSubsystemSteam(TargetInfo Target)
	{	
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "CoreUObject", "Engine", "InputCore", "Steamworks", "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemNull", "Sockets"
            }
		);		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
