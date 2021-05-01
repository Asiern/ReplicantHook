# NieR Replicant Hook

A C++ library to attach to the NieR Replicant process and read/write memory.

[Using ReplicantHook](#using-replicanthook) <br>
[Memory Reference](#memory-reference)<br>
[ReplicantHook Reference](#replicanthook-reference)

## Using ReplicantHook

```
#include <iostream>
#include "ReplicantHook.hpp"
#include <thread>
using namespace std;

//Function used to exit the program
void ENDPressed(ReplicantHook* hook) {
	while (true) {
		if (GetKeyState(VK_END) & 0x8000) //END button pressed
		{
			//Disable cheats before exiting
			hook->InfiniteHealth(false);
			hook->InfiniteMagic(false);
			//Stop hook
			hook->stop();
			return; //exit function
		}
	}
}


/*This is a showcase program of the hook
* As NieR Replicant ver.1.22474487139 is a x64 program, you must compile this solution in x64.
*/
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //Look for memory leaks

	ReplicantHook hook = ReplicantHook();
	cout << "Replicant Hook\n";
	cout << "Hooking..." << endl;
	//Hook to process
	while (!hook.isHooked()) {
		hook.start();
		Sleep(500);
	}
	cout << "Hooked" << endl;

	//Enable some cheats
	hook.InfiniteHealth(true);
	hook.InfiniteMagic(true);

	//Create a thread to exit when the 'END' button is pressed
	//thread exitThread(ENDPressed, &hook);

	//Print some values
	while (hook.isHooked()) {
		hook.update();
		cout << "Magic " << hook.getMagic() << endl;
		cout << "Health " << hook.getHealth() << endl;
		cout << "Gold " << hook.getGold() << endl;
		cout << "Zone " << hook.getZone() << endl;
		Sleep(500);
		system("cls");
	}

	//Join thread and exit
	//exitThread.join();

	return 0;
}
```

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
- `getZone` - returns world's Zone
- `getName` - returns player's Name
- `getX` - returns player's X position
- `getY` - returns player's Y position
- `getZ` - returns player's Z position
- `getActorModel` - returns current actor model name
- `setX` - sets player's X position
- `setY` - sets player's Y position
- `setZ` - sets player's Z position
- `setPosition` - sets the position of the player
- `setHealth` - sets player Health
- `setMagic` - sets player's Magic
- `setGold` - sets player's Gold
- `setZone` - sets world's Zone
- `setName` - sets player's Name
- `setLevel` - sets player's Level
- `setActorModel` - sets actor model (you need to trigger a loading zone to see the results)
- `InfiniteHealth` - enables or disables infinite Health
- `InfiniteMagic` - enables or disables infinite Magic

## Contributors

- [`DeepGameResearch`](https://twitter.com/DeepGameRes) - model addresses finding
