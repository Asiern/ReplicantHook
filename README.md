# NieR Replicant Hook

A C++ library to attach to the NieR Replicant process and read/write memory.

[Using ReplicantHook](#using-replicanthook) <br>
[Memory Reference](#memory-reference)<br>
[ReplicantHook Reference](#replicanthook-reference)

## Using ReplicantHook

---

## Memory Reference

You can find all the used IDs and offsets [here](https://docs.google.com/spreadsheets/d/14InwW9gADoyNCYglMC1XKAQ8ynC-UG6q9iGekA2w56Y/edit?usp=sharing)

---

## ReplicantHook Reference

#### Methods

- `start` - attach the hook to `NieR Replicant ver.1.22474487139.exe` process
- `stop` - detach the hook from process
- `isHooked` - return true if hooked
- `getProcessID` - returns process ID
- `update` - refresh hook & attributes
- `getLevel` - return player's Level
- `getHealth` - returns player's Health
- `getMagic` - returns player's Magic
- `getGold` - returns player's Gold
- `getX` - returns player's X position
- `getY` - returns player's Y position
- `getZ` - returns player's Z position
- `setX` - sets player's X position
- `setY` - sets player's Y position
- `setZ` - sets player's Z position
- `setPosition` - sets the position of the player
- `setHealth` - sets player Health
- `setMagic` - sets player's Magic
- `setLevel` - sets player's Level
- `InfiniteHealth` - enables or disables infinite Health
- `InfiniteMagic` - enables or disables infinite Magic
