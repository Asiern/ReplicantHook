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

	//Change actor model
	hook.setActorModel("nierF");

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