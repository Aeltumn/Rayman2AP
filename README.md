Rayman 2 for Archipelago
---

A mod for Rayman 2 built on the [Twofold](https://github.com/spitfirex86/Twofold) mod loader to add support to Rayman 2 for using it with the [Archipelago](https://github.com/ArchipelagoMW/Archipelago) mutli-world randomizer.

This mod is implemented in two parts:
- Rayman2Connector is a standalone exe placed in the game directory. This is a C++ program which implements the AP client using N00byKing's [APCpp](https://github.com/N00byKing/APCpp).
- Rayman2APMod is a dll mod installed with Twofold which gets injected into the game. It requires the Rayman2Connector.exe to be present so it can boot it as a subprocess and communicate with it. This is done as Rayman 2 is written in C and it is easier to use the existing AP client written in C++.

There is also a DLLDebugger module available which was used to test the connector module as executed from the C code.

Absolutely gigantic credit goes to Spitfire_x86 for their incredible work on making the mod loader and so many useful mods without which this would be absolutely impossible.

This work is based on [R2DbgScr](https://github.com/spitfirex86/R2DbgScr) and [R2Console](https://github.com/spitfirex86/R2Console) by Spitfire_x86. Parts derived from these projects are under the copyright of Spitfire_x86.