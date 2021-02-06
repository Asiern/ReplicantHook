#pragma once
#include <Windows.h>
#include <TlHelp32.h>
class ReplicantHook
{
private:
	DWORD _pID;
	uintptr_t _baseAddress;
	bool hooked;

	DWORD _getProcessID(void);
	uintptr_t _getModuleBaseAddress(DWORD procId, const wchar_t* modName);

public:
	ReplicantHook();
	~ReplicantHook();
};

