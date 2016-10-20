//Copyright 2016 davevillz, https://github.com/davevill

using System.IO;
using UnrealBuildTool;
using System;


public class Steamworks : ModuleRules
{
	private string ModulePath
    {
        get { return ModuleDirectory; }
    }
 
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath( Path.Combine( ModulePath, "../../ThirdParty/" ) ); }
    }


	public Steamworks(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[]
			{
				"Steamworks/Public"
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[]
			{
				"Steamworks/Private"
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils"
            }
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "Networking",
                "HTTP"
            }
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
       


		LoadSteamworks(Target);
	}

	public bool LoadSteamworks(TargetInfo Target)
    {
		bool is64bit = (Target.Platform == UnrealTargetPlatform.Win64);
        bool isLibrarySupported = false;
 
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

 
            string PlatformString = is64bit ? "win64" : "win32";
            string LibrariesPath = Path.Combine(ThirdPartyPath, "sdk", "public", "steam", "lib", PlatformString);
 
			string EncryptedAppTicketLib = is64bit ? "sdkencryptedappticket64.lib" : "sdkencryptedappticket.lib";

            //Console.WriteLine("Thirdparty path " + LibrariesPath);

			string SteamApiLib = Path.Combine(ThirdPartyPath, "sdk", "redistributable_bin");

			if (is64bit)
			{
				SteamApiLib = Path.Combine(SteamApiLib, "win64", "steam_api64.lib"); 
			}
			else
			{
				SteamApiLib = Path.Combine(SteamApiLib, "steam_api.lib"); 
			}
 
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, EncryptedAppTicketLib)); 
            PublicAdditionalLibraries.Add(SteamApiLib); 
        }
 
        if (isLibrarySupported)
        {
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "sdk", "public"));
        }
 
        Definitions.Add(string.Format("WITH_CUSTOM_STEAMWORKS={0}", isLibrarySupported ? 1 : 0 ) );
 
        return isLibrarySupported;
    }
}
