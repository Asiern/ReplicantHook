#pragma once
#include <Windows.h>

struct offsets {
	uintptr_t entity;
	uintptr_t actorPlayable;
	uintptr_t model;
	uintptr_t gold;
	uintptr_t zone;
	uintptr_t name;
	uintptr_t health;
	uintptr_t magic;
	uintptr_t level;
	uintptr_t playtime;
	uintptr_t x;
	uintptr_t y;
	uintptr_t z;

	//Cheats
	uintptr_t InfiniteHealth;
	uintptr_t InfiniteMagic;
};

