Rayman 2 for Archipelago
---

A mod for Rayman 2 built on the [Twofold](https://github.com/spitfirex86/Twofold) mod loader to add support to Rayman 2 for using it with the [Archipelago](https://github.com/ArchipelagoMW/Archipelago) mutli-world randomizer.

This mod is implemented in two parts:
- Rayman2Connector is a standalone exe placed in the game directory. This is a C++ program which implements the AP client using N00byKing's [APCpp](https://github.com/N00byKing/APCpp).
- Rayman2APMod is a dll mod installed with Twofold which gets injected into the game. It requires the Rayman2Connector.exe to be present so it can boot it as a subprocess and communicate with it. This is done as Rayman 2 is written in C and it is easier to use the existing AP client written in C++.

There is also a DLLDebugger module available which was used to test the connector module as executed from the C code.

Absolutely gigantic credit goes to Spitfire_x86 for their incredible work on making the mod loader and so many useful mods without which this would be absolutely impossible.

This work is based on [R2DbgScr](https://github.com/spitfirex86/R2DbgScr) and [R2Console](https://github.com/spitfirex86/R2Console) by Spitfire_x86. Parts derived from these projects are under the copyright of Spitfire_x86.

How to install
---
This guide only covers how to install and run the Rayman 2 mod, customizing the config, generating the game, and hosting the room are all the same as any other Archipelago game.

1. Install the GOG version of Rayman 2 using the standalone installer ([Part 1](https://www.gog.com/downloads/rayman_2_the_great_escape/en1installer0), [Part 2](https://www.gog.com/downloads/rayman_2_the_great_escape/en1installer1)). 
    - You might be able to use the Uplay/Ubisoft Connect version but this is not supported and may cause issues.
    - Any retail CD/DVD versions are not supported as their built-in DRM is broken on modern hardware.
    - Installing through GOG Galaxy should work but you may experience issues with controller support.
2. Install [Ray2Fix](https://github.com/spitfirex86/Ray2Fix/releases) by downloading the zip file of its latest release and extracting the contents of the zip into the Rayman 2 game folder.
3. Run `R2FixCfg.exe` and choose your preferred display settings in the General tab.
4. Install [Twofold](https://github.com/spitfirex86/Twofold/releases) by downloading the zip file of its latest release and extracting the contents of the zip into the Rayman 2 game folder.
5. Install [R2Console](https://github.com/spitfirex86/R2Console/releases) by downloading the zip file of its latest release and extracting the contents of the zip into the Rayman 2 game folder.
6. Download the [latest release](https://github.com/Aeltumn/Rayman2AP/releases) of this project and locate `Rayman2AP.zip`, extract the contents of the zip into the Rayman 2 game folder.
7. Install [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-supported-redistributable-version) which is required to run the Archipelago integration.
8. Run the game through `Ray2x.exe` to launch the modded version. `Rayman2.exe` will launch the unmodded version of the game.
9. In-game you can press `~` to open up the console. Use the `ap connect <ip> <slot> [password]` command to connect to an Archipelago room.
    - If you use a non-US keyboard the key may not be the same. It will use whatever `VK_OEM_3` is on your keyboard.
10. Starting a new game after connecting to a room will use the configured random settings.