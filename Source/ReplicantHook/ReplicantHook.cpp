#include "ReplicantHook.hpp"
#include <iostream>

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
void ReplicantHook::_hook()
{

	DWORD ID = this->_getProcessID();
	if (ID <= 0)
		return;
	this->_pID = ID;
	this->_baseAddress = this->_getModuleBaseAddress(ID, L"NieR Replicant ver.1.22474487139.exe");

	//Get game version


	switch (_version) {
	case 0: {
		_offsets.entity = 0x4372790;
		_offsets.actorPlayable = 0x26F72D0;
		_offsets.model = 0xB88280;
		_offsets.gold = 0xBC;
		_offsets.zone = 0x4;
		_offsets.name = 0x2C;
		_offsets.health = 0x4C;
		_offsets.magic = 0x58;
		_offsets.level = 0x64;
		_offsets.playtime = 0x4A0;
		_offsets.InfiniteHealth = 0x5D106DD;
		_offsets.InfiniteMagic = 0x3BDB5E;
	}
	case 1: {
		_offsets.entity = 0x4374A20;
		_offsets.actorPlayable = 0x26F9560;
		_offsets.model = 0xB892C0;
		_offsets.gold = 0xBC;
		_offsets.zone = 0x4;
		_offsets.name = 0x2C;
		_offsets.health = 0x4C;
		_offsets.magic = 0x58;
		_offsets.level = 0x64;
		_offsets.playtime = 0x4A0;
		_offsets.InfiniteHealth = 0x5F72DED;
		_offsets.InfiniteMagic = 0x3BE2BE;
	}
	}
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


std::string ReplicantHook::readMemoryString(uintptr_t address)
{
	char val[20];
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->_pID);
	ReadProcessMemory(pHandle, (LPCVOID)address, &val, sizeof(val), NULL);
	CloseHandle(pHandle); //Close handle to prevent memory leaks
	return std::string(val);
}

void ReplicantHook::writeMemoryString(uintptr_t address, std::string value)
{
	SIZE_T BytesToWrite = value.length() + 1;
	SIZE_T BytesWritten;
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->_pID);
	WriteProcessMemory(pHandle, (LPVOID)address, (LPCVOID)value.c_str(), BytesToWrite, &BytesWritten);
}

ReplicantHook::ReplicantHook(int version)
{
	this->_version = version;
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
	this->actorPlayable = readMemory <uintptr_t>(this->_baseAddress + _offsets.actorPlayable);
	this->gold = readMemory<int>(this->_baseAddress + _offsets.entity + _offsets.gold);
	this->zone = readMemoryString(this->_baseAddress + _offsets.entity + _offsets.zone);
	this->name = readMemoryString(this->_baseAddress + _offsets.entity + _offsets.name);
	this->health = readMemory<int>(this->_baseAddress + _offsets.entity + _offsets.health);
	this->magic = readMemory<float>(this->_baseAddress + _offsets.entity + _offsets.magic);
	this->level = readMemory<int>(this->_baseAddress + _offsets.entity + _offsets.level);
	this->playtime = readMemory<double>(this->_baseAddress + _offsets.entity + _offsets.playtime);
	this->x = readMemory<float>((uintptr_t)(this->actorPlayable + 0x9C));
	this->y = readMemory<float>((uintptr_t)(this->actorPlayable + 0xAC));
	this->z = readMemory<float>((uintptr_t)(this->actorPlayable + 0xBC));
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
	this->writeMemory(this->_baseAddress + _offsets.entity + _offsets.gold, value);
}

void ReplicantHook::setZone(std::string value)
{
	this->writeMemoryString(this->_baseAddress + _offsets.entity + _offsets.zone, value);
}

void ReplicantHook::setName(std::string value)
{
	this->writeMemoryString(this->_baseAddress + _offsets.entity + _offsets.name, value);
}

void ReplicantHook::setHealth(int value)
{
	this->writeMemory(this->_baseAddress + _offsets.entity + _offsets.health, value);
}

void ReplicantHook::setMagic(float value)
{
	this->writeMemory(this->_baseAddress + _offsets.entity + _offsets.magic, value);
}

void ReplicantHook::setLevel(int value)
{
	this->writeMemory(this->_baseAddress + _offsets.entity + _offsets.level, value);
}

void ReplicantHook::setPlaytime(double value)
{
	this->writeMemory(this->_baseAddress + _offsets.entity + _offsets.playtime, value);
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
		_patch((BYTE*)(this->_baseAddress + _offsets.InfiniteHealth), (BYTE*)"\x90\x90\x90\x90", 4);
	else
		_patch((BYTE*)(this->_baseAddress + _offsets.InfiniteHealth), (BYTE*)"\x89\x44\x81\x4C", 4);
}

void ReplicantHook::InfiniteMagic(bool enabled)
{
	if (enabled)
		_patch((BYTE*)(this->_baseAddress + _offsets.InfiniteMagic), (BYTE*)"\x90\x90\x90\x90\x90\x90", 6);
	else
		_patch((BYTE*)(this->_baseAddress + _offsets.InfiniteMagic), (BYTE*)"\xF3\x0F\x11\x54\x81\x58", 6);
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void ReplicantHook::setActorModel(std::string model)
{
	BYTE* modelBytes;
	switch (str2int(model.c_str())) {
	case str2int("nierB"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x42\x00\x00"; //nierB
		break;
	case str2int("nierT"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x54\x00\x00"; //nierT
		break;
	case str2int("nierF"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x46\x00\x00"; //nierF
		break;
	case str2int("nierY"):
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x59\x00\x00"; //nierY
		break;
		//case str2int("nier010"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x31\x30"; //nier010
		//	break;
		//case str2int("nier011"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x31\x31"; //nier011
		//	break;
		//case str2int("nier020"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x32\x30"; //nier020
		//	break;
		//case str2int("nier030"):
		//	modelBytes = (BYTE*)"\x6E\x69\x65\x72\x30\x33\x30"; //nier030
		//	break;
	case str2int("kaineE"):
		modelBytes = (BYTE*)"\x6B\x61\x69\x6E\x65\x45\x00"; //kaineE
		break;
	default:
		modelBytes = (BYTE*)"\x6E\x69\x65\x72\x42\x00\x00"; //default nierB
		break;
	}
	this->_patch((BYTE*)(this->_baseAddress + _offsets.model), modelBytes, 7);
}

std::string ReplicantHook::getActorModel()
{
	return readMemoryString(this->_baseAddress + _offsets.model);
}

