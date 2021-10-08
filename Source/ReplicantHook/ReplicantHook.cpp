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

uintptr_t ReplicantHook::_getModuleBaseAddress(DWORD procId, const wchar_t *modName)
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

	switch (_version)
	{
	case 0:
	{
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
	case 1:
	{
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

void ReplicantHook::_patch(BYTE *destination, BYTE *src, unsigned int size)
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
	this->loadInventory();
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
	this->actorPlayable = readMemory<uintptr_t>(this->_baseAddress + _offsets.actorPlayable);
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
		_patch((BYTE *)(this->_baseAddress + _offsets.InfiniteHealth), (BYTE *)"\x90\x90\x90\x90", 4);
	else
		_patch((BYTE *)(this->_baseAddress + _offsets.InfiniteHealth), (BYTE *)"\x89\x44\x81\x4C", 4);
}

void ReplicantHook::InfiniteMagic(bool enabled)
{
	if (enabled)
		_patch((BYTE *)(this->_baseAddress + _offsets.InfiniteMagic), (BYTE *)"\x90\x90\x90\x90\x90\x90", 6);
	else
		_patch((BYTE *)(this->_baseAddress + _offsets.InfiniteMagic), (BYTE *)"\xF3\x0F\x11\x54\x81\x58", 6);
}

constexpr unsigned int str2int(const char *str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void ReplicantHook::setActorModel(std::string model)
{
	BYTE *modelBytes;
	switch (str2int(model.c_str()))
	{
	case str2int("nierB"):
		modelBytes = (BYTE *)"\x6E\x69\x65\x72\x42\x00\x00"; //nierB
		break;
	case str2int("nierT"):
		modelBytes = (BYTE *)"\x6E\x69\x65\x72\x54\x00\x00"; //nierT
		break;
	case str2int("nierF"):
		modelBytes = (BYTE *)"\x6E\x69\x65\x72\x46\x00\x00"; //nierF
		break;
	case str2int("nierY"):
		modelBytes = (BYTE *)"\x6E\x69\x65\x72\x59\x00\x00"; //nierY
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
		modelBytes = (BYTE *)"\x6B\x61\x69\x6E\x65\x45\x00"; //kaineE
		break;
	default:
		modelBytes = (BYTE *)"\x6E\x69\x65\x72\x42\x00\x00"; //nierB
		break;
	}
	this->_patch((BYTE *)(this->_baseAddress + _offsets.model), modelBytes, 7);
}

std::string ReplicantHook::getActorModel()
{
	return readMemoryString(this->_baseAddress + _offsets.model);
}

std::map<std::string, uintptr_t> ReplicantHook::getInventory(void)
{
	return this->_inventory;
}

int ReplicantHook::addItem(std::string name, int quantity)
{
	uintptr_t offset = getItemAddress(name);
	if (offset == -1) //Item not found
		return -1;
	writeMemory(this->_baseAddress + offset, quantity);
	return 0; //Success
}

int ReplicantHook::removeItem(std::string name)
{
	return addItem(name, 0);
}

void ReplicantHook::loadInventory()
{
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Medicinal Herb", 0x4374AE0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Health Salve", 0x4374AE1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Recovery Potion", 0x4374AE2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Strength Drop", 0x4374AF5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Strength Capsule", 0x4374AF6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Magic Drop", 0x4374AF7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Magic Capsule", 0x4374AF8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Defense Drop", 0x4374AF9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Defense Capsule", 0x4374AFA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Spirit Drop", 0x4374AFB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Spirit Capsule", 0x4374AFC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Antidotal Weed", 0x4374AFF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Speed Fertilizer", 0x4374B09));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Flowering Fertilizer", 0x4374B0A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bounty Fertilizer", 0x4374B0B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pumpkin Seed", 0x4374B0E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Watermelon Seed", 0x4374B0F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Melon Seed", 0x4374B10));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gourd Seed", 0x4374B11));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tomato Seed", 0x4374B12));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Eggplant Seed", 0x4374B13));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bell Pepper Seed", 0x4374B14));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Been Seed", 0x4374B15));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Wheat Seed", 0x4374B16));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rice Plant Seed", 0x4374B17));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dahlia Seed", 0x4374B18));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tulip Seed", 0x4374B19));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Freesia Bulb", 0x4374B1A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Red Moonflower Seed", 0x4374B1B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gold Moonlfower Seed", 0x4374B1C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Peach Moonflower Seed", 0x4374B1D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pink Moonflower Seed", 0x4374B1E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blue Moonflower Seed", 0x4374B1F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Indigo Moonflower Seed", 0x4374B20));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("White Moonflower Seed", 0x4374B21));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pumpkin", 0x4374B27));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Watermelon", 0x4374B28));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Melon", 0x4374B29));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gourd", 0x4374B2A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tomato", 0x4374B2B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Eggplant", 0x4374B2C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bell Pepper", 0x4374B2D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Beans", 0x4374B2E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Wheat", 0x4374B2F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rice", 0x4374B30));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dahlia", 0x4374B31));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tulip", 0x4374B32));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Freesia", 0x4374B33));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Red Moonflower", 0x4374B34));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gold Moonlfower", 0x4374B35));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Peach Moonflower", 0x4374B36));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pink Moonflower", 0x4374B37));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blue Moonflower", 0x4374B38));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Indigo Moonflower", 0x4374B39));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("White Moonflower", 0x4374B3A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lugworm", 0x4374B45));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Earthworm", 0x4374B46));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lure", 0x4374B47));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sardine", 0x4374B4F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Carp", 0x4374B50));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blowfish", 0x4374B51));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bream", 0x4374B52));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Shark", 0x4374B53));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blue Marlin", 0x4374B54));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dunkleosteus", 0x4374B55));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rainbow Trout", 0x4374B56));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Black Bass", 0x4374B57));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Giant Catfish", 0x4374B58));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Royal Fish", 0x4374B59));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Hyneria", 0x4374B5A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sandfish", 0x4374B5B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rhizodont", 0x4374B5C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Shaman FIsh", 0x4374B5D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Aquatic Plant", 0x4374B63));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Deadwood", 0x4374B64));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rusty Bucket", 0x4374B65));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Empty Can", 0x4374B66));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gold Ore", 0x4374B6A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Silvert Ore", 0x4374B6B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Copper Ore", 0x4374B6C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Iron Ore", 0x4374B6D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Crystal", 0x4374B6E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pyrite", 0x4374B6F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Moldavite", 0x4374B70));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Meteorite", 0x4374B71));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Amber", 0x4374B72));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Fluorite", 0x4374B73));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Clay", 0x4374B74));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Berries", 0x4374B79));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Royal Fern", 0x4374B7A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tree Branch", 0x4374B7B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Log", 0x4374B7C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Natural Rubber", 0x4374B7D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Ivy", 0x4374B7E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lichen", 0x4374B7F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mushroom", 0x4374B80));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sap", 0x4374B81));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mutton", 0x4374B87));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Meat", 0x4374B88));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Woll", 0x4374B89));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Hide", 0x4374B8A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Wolf Hide", 0x4374B8B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Wolf Fang", 0x4374B8C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Giant Spider Silk", 0x4374B8D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bat Fang", 0x4374B8E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bat Wing", 0x4374B8F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Goat Meat", 0x4374B90));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Goat Hide", 0x4374B91));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Venison", 0x4374B92));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rainbow Spider Silk", 0x4374B93));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Liver", 0x4374B94));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Scorpion Claw", 0x4374B95));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Scorpion Tail", 0x4374B96));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dented Metal Board", 0x4374B97));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Stripped Bolt ", 0x4374B98));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Lens", 0x4374B99));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Severed Cable", 0x4374B9A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Arm", 0x4374B9B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Antena", 0x4374B9C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Motor", 0x4374B9D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Battery", 0x4374B9E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mysterious Switch", 0x4374B9F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Large Gear", 0x4374BA0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Titanium Alloy", 0x4374BA1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Memory Alloy", 0x4374BA2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rusted Clump", 0x4374BA3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Machine Oil", 0x4374BA4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Forlorn Necklace", 0x4374BA9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Twisted Ring", 0x4374BAA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Earring", 0x4374BAB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pretty Choker", 0x4374BAC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Metal Piercing", 0x4374BAD));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Subdued Bracelet", 0x4374BAE));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Technical Guide", 0x4374BAF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Grubby Book", 0x4374BB0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Thick Dictionary", 0x4374BB1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Closed Book", 0x4374BB2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Used Coloring Book", 0x4374BB3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Old Schoolbook", 0x4374BB4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dirty Bag", 0x4374BB5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Flashyh Hat", 0x4374BB6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Leather Gloves", 0x4374BB7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Silk Handkerchief", 0x4374BB8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Leather Boots", 0x4374BB9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Complex Machine", 0x4374BBA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Elaborate Machine", 0x4374BBB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Simple Machine", 0x4374BBC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Stopped Clock ", 0x4374BBD));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Wristwatch", 0x4374BBE));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rusty Kitchen Knife", 0x4374BBF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Saw", 0x4374BC0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dented Metal Bat", 0x4374BC1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Shell", 0x4374BC3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gastropod", 0x4374BC4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bivalve", 0x4374BC5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Seaweed", 0x4374BC6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Empty Bottle", 0x4374BC7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Driftwood", 0x4374BC8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pearl", 0x4374BC9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Black Pearl", 0x4374BCA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Crab", 0x4374BCB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Starfish", 0x4374BCC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sea Turtle Egg", 0x4374BD2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Pottery", 0x4374BD3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Desert Rose", 0x4374BD4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Giant Egg", 0x4374BD5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Damascus Steel", 0x4374BD6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Eagle Egg", 0x4374BD7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Chicken Egg", 0x4374BD8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mouse Tail", 0x4374BDA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lizard Tail", 0x4374BDB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Deer Antler", 0x4374BDF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Moon Key", 0x4374BE0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Star Key", 0x4374BE1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Light Key", 0x4374BE2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Darkness Key", 0x4374BE3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Fine Flour", 0x4374BE4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Coarse FLour", 0x4374BE5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Perfume Bottle", 0x4374BE6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Postman's Parcel", 0x4374BE7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lover's Letter", 0x4374BE8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Water Filter", 0x4374BE9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Royal Compass", 0x4374BEA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Vapor Moss", 0x4374BEB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Valley Spoder Silk", 0x4374BEC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Animal Guidebook", 0x4374BED));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Ore Guidebook", 0x4374BEE));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Plant Guidebook", 0x4374BEF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Red Book", 0x4374BF0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blue Book", 0x4374BF1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Old Lady's Elixer", 0x4374BF2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Old Lady's Elixer+", 0x4374BF3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Parcel for The Aerie", 0x4374BF4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Parcel for Seafront", 0x4374BF5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Cookbook", 0x4374BF6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Parcel for Facade", 0x4374BF7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Max's Herbs", 0x4374BF8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Drifting Cargo", 0x4374BF9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Drifting Cargo 2", 0x4374BFA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Drifting Cargo 3", 0x4374BFB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Drifting Cargo 4", 0x4374BFC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Old Package", 0x4374BFD));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mermaid Tear", 0x4374BFE));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mandrake Leaf", 0x4374BFF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Energizer", 0x4374C00));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tad Oil", 0x4374C01));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sleep-B-Gone", 0x4374C02));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Antidote", 0x4374C03));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gold Bracelet", 0x4374C04));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Elite Kitchen Knife", 0x4374C05));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Elevator Parts", 0x4374C06));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dirty Treasure Map", 0x4374C07));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Restored Treasure Map", 0x4374C08));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Jade Hair Ornament", 0x4374C09));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Employee List", 0x4374C0A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Small Safe", 0x4374C0B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Safe Key", 0x4374C0C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Greaty Tree Root", 0x4374C0D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Eye of Power", 0x4374C0E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Ribbon", 0x4374C0F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Yonah's Ribbon", 0x4374C10));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bronze Key", 0x4374C11));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Brass Key", 0x4374C12));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Tusk", 0x4374C13));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pressed Freesia", 0x4374C14));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Potted Freesia", 0x4374C15));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Fressia (Delivery)", 0x4374C16));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pile of Junk", 0x4374C17));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Old Gold Coin", 0x4374C18));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Market Map", 0x4374C19));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("AA Keycard", 0x4374C1A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("KA Keycard", 0x4374C1B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("SA Keycard", 0x4374C1C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("TA Keycard", 0x4374C1D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("NA Keycard", 0x4374C1E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("HA Keycard", 0x4374C1F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("MA Keycard", 0x4374C20));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("YA Keycard", 0x4374C21));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("RA Keycard", 0x4374C22));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("WA Keycard", 0x4374C23));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Cultivator's Handbook", 0x4374C24));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Red Bag", 0x4374C25));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lantern", 0x4374C26));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Empty Lantern", 0x4374C27));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Hold Key ", 0x4374C28));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Passageway Key", 0x4374C29));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Goat Key", 0x4374C2A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lizard Key", 0x4374C2B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Unlocking Procedure Memo ", 0x4374C2C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Red Jewel", 0x4374C2D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Red Flowers ", 0x4374C2E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Apples", 0x4374C2F));
}

uintptr_t ReplicantHook::getItemAddress(std::string name)
{
	std::map<std::string, uintptr_t>::iterator it = _inventory.begin();
	std::map<std::string, uintptr_t>::iterator end = _inventory.end();
	for (; it != end; it++)
	{
		if (it->first == name)
		{
			return it->second;
		}
	}
	return -1;
}
