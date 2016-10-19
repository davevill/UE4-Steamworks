# Steamworks for Unreal Engine
Steamworks plugin for Unreal Engine for those who wants a full steam integration and has no plans to deploy anywhere else. PC games are different and is worth to lift the contrainsts that consoles and mobile platform has put into the OnlineSubsystem abstraction layer.

This plugin does some basic plumbing and exposes several classes for C++ and blueprint usage

## This plugin will ONLY be usefull if you need:
- Dedicated servers without engine modifications
- Ability to integrate server provision services like AWS Gamelift easily
- Inventory
- Microtransactions
- Skill based matchmaking
- L4D, CSGO, DOTA style lobbies
- Advanced Stats (or atleast not bugged)

if you only need a simple peer to peer multiplayer, the default OnlineSystemSteam is enough

**This is not an OnlineSubsystem implementation**
