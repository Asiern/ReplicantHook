#include "ReplicantHook.hpp"

DWORD ReplicantHook::_getProcessID(void)
{
	//Search game window
	HWND hwnd = FindWindowA(NULL, "NieR Replicant ver.1.22474487139...");
	if (hwnd == NULL)
	{
		//return if game window not found
		return 0;
	}
	DWORD pID;													  //Process ID
	GetWindowThreadProcessId(hwnd, &pID);						  //Get Process ID
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); //Open process
	if (pHandle == INVALID_HANDLE_VALUE)
	{
		//return if couldn't open the process
		return 0;
	}
	return pID;
}

uintptr_t ReplicantHook::_getModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap); //Close handle to prevent memory leaks
	return modBaseAddr;
}

//Hook to NieR:Automata process
void ReplicantHook::_hook(void)
{
	DWORD ID = this->_pID;
	while (ID <= 0)
	{
		ID = this->_getProcessID();
	}
	this->_pID = ID;
	this->_baseAddress = this->_getModuleBaseAddress(ID, L"NieR Replicant ver.1.22474487139.exe");
	this->_hooked = true;
}
//unHook NieR:Automata
void ReplicantHook::_unHook(void)
{
	this->_hooked = false;
	this->_pID = 0;
	this->_baseAddress = 0;
	this->actorPlayable = 0;
	this->gold = 0;
	this->zone = "";
	this->name = "";
	this->health = 0;
	this->magic = 0.0f;
	this->level = 0;
	this->playtime = 0.0;
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->InfiniteHealth(false);
	this->InfiniteMagic(false);
}

void ReplicantHook::_patch(BYTE* destination, BYTE* src, unsigned int size)
{
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->_pID);
	DWORD oldprotection;
	VirtualProtectEx(pHandle, destination, size, PAGE_EXECUTE_READWRITE, &oldprotection);
	WriteProcessMemory(pHandle, destination, src, size, nullptr);
	VirtualProtectEx(pHandle, destination, size, oldprotection, &oldprotection);
	CloseHandle(pHandle);
}


ReplicantHook::ReplicantHook()
{
	this->_hooked = false;
	this->_baseAddress = 0;
	this->actorPlayable = 0;
	this->_pID = 0;
	this->gold = 0;
	this->health = 0;
	this->level = 0;
	this->magic = 0.0f;
	this->playtime = 0.0;
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->zone = "";
	this->name = "";
}

ReplicantHook::~ReplicantHook()
{
}

DWORD ReplicantHook::getProcessID(void)
{
	return this->_pID;
}

uintptr_t ReplicantHook::getBaseAddress(void)
{
	return this->_baseAddress;
}

void ReplicantHook::start(void)
{
	this->_hook();
}

void ReplicantHook::stop(void)
{
	this->_unHook();
}

void ReplicantHook::hookStatus(void)
{
	if (this->_pID != this->_getProcessID())
	{
		this->_unHook();
	}
}

void ReplicantHook::update()
{
	this->actorPlayable = readMemory <DWORD>(0x26F72D0);
	this->gold = readMemory<int>(0x437284C);
	//this->zone = readMemory<std::string>(0x4372794);
	//this->name = readMemory<std::string>(0x43727BC);
	this->health = readMemory<int>(0x43727DC);
	this->magic = readMemory<float>(0x43727E8);
	this->level = readMemory<int>(0x43727F4);
	this->playtime = readMemory<double>(0x4372C30);
	this->x = readMemory<float>((uintptr_t)this->actorPlayable + 0x9C);
	this->y = readMemory<float>((uintptr_t)this->actorPlayable + 0xAC);
	this->z = readMemory<float>((uintptr_t)this->actorPlayable + 0xBC);
}

bool ReplicantHook::isHooked(void)
{
	return this->_hooked;
}

int ReplicantHook::getGold()
{
	return this->gold;
}

std::string ReplicantHook::getZone()
{
	return this->zone;
}

std::string ReplicantHook::getName()
{
	return this->name;
}

int ReplicantHook::getHealth()
{
	return this->health;
}

float ReplicantHook::getMagic()
{
	return this->magic;
}

int ReplicantHook::getLevel()
{
	return this->level;
}

double ReplicantHook::getPlaytime()
{
	return this->playtime;
}

float ReplicantHook::getX()
{
	return this->x;
}

float ReplicantHook::getY()
{
	return this->y;
}

float ReplicantHook::getZ()
{
	return this->z;
}

void ReplicantHook::setGold(int value)
{
	this->writeMemory(0x437284C, value);
}

void ReplicantHook::setZone(std::string value)
{
	this->writeMemory(0x4372794, value);
}

void ReplicantHook::setName(std::string value)
{
	this->writeMemory(0x43727BC, value);
}

void ReplicantHook::setHealth(int value)
{
	this->writeMemory(0x43727DC, value);
}

void ReplicantHook::setMagic(float value)
{
	this->writeMemory(0x43727E8, value);
}

void ReplicantHook::setLevel(int value)
{
	this->writeMemory(0x43727F4, value);
}

void ReplicantHook::setPlaytime(double value)
{
	this->writeMemory(0x4372C30, value);
}

void ReplicantHook::setX(float value)
{
	this->writeMemory(this->actorPlayable + 0x9C, value);
}

void ReplicantHook::setY(float value)
{
	this->writeMemory(this->actorPlayable + 0xAC, value);
}

void ReplicantHook::setZ(float value)
{
	this->writeMemory(this->actorPlayable + 0xBC, value);
}

void ReplicantHook::setPosition(float x, float y, float z)
{
	this->setX(x);
	this->setY(y);
	this->setZ(z);
}

void ReplicantHook::InfiniteHealth(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(this->_baseAddress + 0x5D106E2), (BYTE*)"\x90\x90\x90\x90", 4);
	else
		_patch((BYTE*)(this->_baseAddress + 0x5D106E2), (BYTE*)"\x89\x44\x81\x4C", 4);
}

void ReplicantHook::InfiniteMagic(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(this->_baseAddress + 0x3BDB5E), (BYTE*)"\x90\x90\x90\x90\x90\x90", 6);
	else
		_patch((BYTE*)(this->_baseAddress + 0x3BDB5E), (BYTE*)"\xF3\x0F\x11\x54\x81\x58", 6);
}
