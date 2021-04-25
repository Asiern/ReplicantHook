#include <iostream>
#include "ReplicantHook.hpp"

int main()
{
	ReplicantHook hook = ReplicantHook();
	std::cout << "Replicant Hook\n";

	hook.start();
	hook.update();
	std::cout << std::dec << hook.getX() << " " << hook.getY() << " " << hook.getZ() << std::endl;
	std::cout << hook.getMagic() << std::endl;
	std::cout << hook.getHealth() << std::endl;
	std::cout << hook.getGold() << std::endl;

	return 0;
}