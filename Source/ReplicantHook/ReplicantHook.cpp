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
	//Recovery Items
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Medicinal Herb", 0x4374AE0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Health Salve", 0x4374AE1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Recovery Potion", 0x4374AE2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Strenght Drop", 0x4374AF5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Strenght Capsule", 0x4374AF6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Magic Drop", 0x4374AF7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Magic Capsule", 0x4374AF8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Defense Drop", 0x4374AF9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Defense Capsule", 0x4374AFA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Spirit Drop", 0x4374AFB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Spirit Capsule", 0x4374AFC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Antidotal Weed", 0x4374AF5F));

	//Cultivation
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

	//Fishing
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lugworm", 0x4373B45));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Earthworm", 0x4373B46));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lure", 0x4373B47));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sardine", 0x4373B4F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Carp", 0x4373B50));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blowfish", 0x4373B51));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bream", 0x4373B52));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Shark", 0x4373B53));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Blue Marlin", 0x4373B54));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dunkleosteus", 0x4373B55));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rainbow Trout", 0x4373B56));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Black Bass", 0x4373B57));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Giant Catfish", 0x4373B58));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Royal Fish", 0x4373B59));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Hyneria", 0x4373B5A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sandfish", 0x4373B5B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rhizodont", 0x4373B5C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Shaman FIsh", 0x4373B5D));

	//Raw Materials
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Aquatic Plant", 0x4373B63));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Deadwood", 0x4373B64));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rusty Bucket", 0x4373B65));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Empty Can", 0x4373B66));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gold Ore", 0x4373B6A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Silvert Ore", 0x4373B6B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Copper Ore", 0x4373B6C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Iron Ore", 0x4373B6D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Crystal", 0x4373B6E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pyrite", 0x4373B6F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Moldavite", 0x4373B70));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Meteorite", 0x4373B71));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Amber", 0x4373B72));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Fluorite", 0x4373B73));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Clay", 0x4373B74));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Berries", 0x4373B79));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Royal Fern", 0x4373B7A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Tree Branch", 0x4373B7B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Log", 0x4373B7C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Natural Rubber", 0x4373B7D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Ivy", 0x4373B7E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lichen", 0x4373B7F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mushroom", 0x4373B80));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sap", 0x4373B81));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mutton", 0x4373B87));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Meat", 0x4373B88));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Woll", 0x4373B89));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Hide", 0x4373B8A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Wolf Hide", 0x4373B8B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Wolf Fang", 0x4373B8C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Giant Spider SIlk", 0x4373B8D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bat Fang", 0x4373B8E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bat Wing", 0x4373B8F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Goat Meat", 0x4373B90));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Goat Hide", 0x4373B91));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Venison", 0x4373B92));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rainbow Spider Silk", 0x4373B93));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Boar Liver", 0x4373B94));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Scorpion Claw", 0x4373B95));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Scorpion Tail", 0x4373B96));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dented Metal Board", 0x4373B97));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Stripped Bolt", 0x4373B98));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Lens", 0x4373B99));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Severed Cable", 0x4373B9A));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Arm", 0x4373B9B));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Antena", 0x4373B9C));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Motor", 0x4373B9D));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Battery", 0x4373B9E));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mysterious Switch", 0x4373B9F));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Large Gear", 0x4373BA0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Titanium Alloy", 0x4373BA1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Memory Alloy", 0x4373BA2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rusted Clump", 0x4373BA3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Machine Oil", 0x4373BA4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Forlorn Necklace", 0x4373BA9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Twisted Ring", 0x4373BAA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Earring", 0x4373BAB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pretty Choker", 0x4373BAC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Metal Piercing", 0x4373BAD));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Subdued Bracelet", 0x4373BAE));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Technical Guide", 0x4373BAF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Grubby Book", 0x4373BB0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Thick Dictionary", 0x4373BB1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Closed Book", 0x4373BB2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Used Coloring Book", 0x4373BB3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Old Schoolbook", 0x4373BB4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dirty Bag", 0x4373BB5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Flashyh Hat", 0x4373BB6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Leather Gloves", 0x4373BB7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Silk Handkerchief", 0x4373BB8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Leather Boots", 0x4373BB9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Complex Machine", 0x4373BBA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Elaborate Machine", 0x4373BBB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Simple Machine", 0x4373BBC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Stopped Clock", 0x4373BBD));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Wristwatch", 0x4373BBE));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Rusty Kitchen Knife", 0x4373BBF));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Saw", 0x4373BC0));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Dented Metal Bat", 0x4373BC1));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Shell", 0x4373BC3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Gastropod", 0x4373BC4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Bivalve", 0x4373BC5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Seaweed", 0x4373BC6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Empty Bottle", 0x4373BC7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Driftwood", 0x4373BC8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Pearl", 0x4373BC9));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Black Pearl", 0x4373BCA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Crab", 0x4373BCB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Starfish", 0x4373BCC));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Sea Turtle Egg", 0x4373BD2));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Broken Pottery", 0x4373BD3));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Desert Rose", 0x4373BD4));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Giant Egg", 0x4373BD5));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Damascus Steel", 0x4373BD6));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Eagle Egg", 0x4373BD7));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Chicken Egg", 0x4373BD8));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Mouse Tail", 0x4373BDA));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Lizard Tail", 0x4373BDB));
	this->_inventory.insert(std::pair<std::string, uintptr_t>("Deer Antler", 0x4373BDF));
}

uintptr_t ReplicantHook::getItemAddress(std::string name)
{
	std::map<std::string, uintptr_t>::iterator it = _inventory.begin();
	std::map<std::string, uintptr_t >::iterator end = _inventory.end();
	for (;it != end;it++) {
		if (it->first == name) {
			return it->second;
		}
	}
	return -1;
}
