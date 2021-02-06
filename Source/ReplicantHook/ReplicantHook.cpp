#include "ReplicantHook.hpp"

DWORD ReplicantHook::_getProcessID(void)
{
	//TODO getProcessID
	return 0;
}

uintptr_t ReplicantHook::_getModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	//TODO getModuleBaseAddress
	return uintptr_t();
}

ReplicantHook::ReplicantHook()
{
	//TODO Constructor
}

ReplicantHook::~ReplicantHook()
{
}
