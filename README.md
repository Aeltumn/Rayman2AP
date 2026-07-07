Rayman 2 for Archipelago
---

A mod for Rayman 2 PC built on the [Twofold](https://github.com/spitfirex86/Twofold) mod loader to add support to Rayman 2 for using it with the [Archipelago](https://github.com/ArchipelagoMW/Archipelago) mutli-world randomizer.

This mod is implemented in two parts:
- Rayman2Connector is a standalone exe placed in the game directory. This is a C++ program which implements the AP client using N00byKing's [APCpp](https://github.com/N00byKing/APCpp).
- Rayman2APMod is a dll mod installed with Twofold which gets injected into the game. It requires the Rayman2Connector.exe to be present so it can boot it as a subprocess and communicate with it. This is done as Rayman 2 is written in C and it is easier to use the existing AP client written in C++.

There is also a DLLDebugger module available which was used to test the connector module as executed from the C code.

Absolutely gigantic credit goes to Spitfire_x86 for their incredible work on making the mod loader and so many useful mods without which this would be absolutely impossible.

This work is based on [R2DbgScr](https://github.com/spitfirex86/R2DbgScr) and [R2Console](https://github.com/spitfirex86/R2Console) by Spitfire_x86. Parts derived from these projects are under the copyright of Spitfire_x86.

How to install
---
This guide only covers how to install and run the Rayman 2 mod, customizing the config, generating the game, and hosting the room are all the same as any other Archipelago game.

1. Install the [GOG version of Rayman 2](https://www.gog.com/en/game/rayman_2_the_great_escape), preferably with the **standalone installer**.
    - You can download the standalone installer only if you are logged in to GOG, from these direct download links: [Part 1](https://www.gog.com/downloads/rayman_2_the_great_escape/en1installer0), [Part 2](https://www.gog.com/downloads/rayman_2_the_great_escape/en1installer1). These links only work if you are logged in and own Rayman 2.
    - You can use GOG Galaxy but you have to be careful to not let it override/revert the game files after installing the mods below.
    - You might be able to use the Uplay/Ubisoft Connect version but this is not supported and may cause issues.
    - Any retail CD/DVD versions are not supported as their built-in DRM is broken on modern hardware.
2. Install [Ray2Fix](https://github.com/spitfirex86/Ray2Fix/releases) by downloading the zip file of its latest release and extracting the contents of the zip into the Rayman 2 game folder.
3. Run `R2FixCfg.exe`, choose your preferred display settings in the General tab and if you want to use a controller, configure the gamepad controls in the Gamepad tab.
4. Install [Twofold](https://github.com/spitfirex86/Twofold/releases) by downloading the zip file of its latest release and extracting the contents of the zip into the Rayman 2 game folder.
5. Install [R2Console](https://github.com/spitfirex86/R2Console/releases) by downloading the zip file of its latest release and extracting the contents of the zip into the Rayman 2 game folder.
6. Download the [latest release](https://github.com/Aeltumn/Rayman2AP/releases) of this project and locate `Rayman2AP.zip`, extract the contents of the zip into the Rayman 2 game folder.
7. Install [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-supported-redistributable-version) which is required to run the Archipelago integration.
8. Run the game through `Ray2x.exe` to launch the modded version. `Rayman2.exe` will launch the unmodded version of the game.
9. In-game you can press `~` to open up the console. Use the `ap connect <ip> <slot> [password]` command to connect to an Archipelago room. You have to put the slot name in quotes ("") if it contains spaces. The console will show an error message if it could not connect to the room.
    - If you use a non-US keyboard the key may not be the same. It will use whatever `VK_OEM_3` is on your keyboard.
10. Starting a new game after connecting to a room will use the configured random settings.

How to use Archipelago
---
For users unfamiliar with [Archipelago](https://archipelago.gg/), you can follow the following steps to create an Archipelago room to connect to:

1. Install [Archipelago](https://archipelago.gg/) using [this guide](https://archipelago.gg/tutorial/Archipelago/setup_en).
2. Run the Archipelago Launcher.
3. Download the [latest release](https://github.com/Aeltumn/Rayman2AP/releases) of this project and copy the `Rayman.2.yaml` and `rayman2.apworld` files to a folder of your choice.
4. In the Archipelago Launcher, click `Install APWorld` and select the `rayman2.apworld` file downloaded from this project's GitHub.
5. You now have to prepare a .yaml file which configures the game you want to play. There are two methods to do this:
   - **Option A:** Modify the `Rayman.2.yaml` file using a text editor to configure the game to your liking.
      - Set the `name` at the top of the file to something recognizable.
      - Configure all the options, the value followed by 50 will be selected.
   - **Option B:** In the Archipelago Launcher, use the `Options Creator` tool to create your options with a custom GUI. Hover over settings to read their descriptions.
6. In the Archipelago Launcher, click `Browse Files` and place your .yaml file in the `Players` directory.
   - You can combine as many games together as you want from the [hundreds of games supported by Archipelago](https://mk-404.github.io/Archipelago-Games-Library/). The vast majority of these games are not listed on Archipelago's own website, just like Rayman 2, as these are community developed and not merged into the main project. You can see all available games [on this community-made list](https://mk-404.github.io/Archipelago-Games-Library/). All .yaml files placed into the `Players` directory are combined into a single randomized multi-world.
7. In the Archipelago Launcher, click `Generate` to generate a game, it will be placed in the `output` directory in the Archipelago files.
8. Upload your generated game to the [Archipelago website](https://archipelago.gg/uploads) and create a room. The room will be hosted on Archipelago's servers and will automatically boot up whenever you visit the room page. You can host your own room on your own device as well if you prefer.
9. You can now connect to your created room from Rayman 2, make sure to set the slot name to the name you picked in the .yaml!