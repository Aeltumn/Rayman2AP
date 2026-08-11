#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <detours.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <direct.h>
#include <stdbool.h>

#include <ACP_Ray2.h>
#include <AI/AI_Array.h>
#include <Ray2x/SPTXT/SPTXT.h>
#include <r2console_api.h>

#pragma comment (lib, "crypt32")

#define LEVEL_COUNT 57
#define MAX_LEVEL_NAME_LENGTH 46
#define MAX_LENGTH 32
#define CHAIN_COUNT 22

#define CHAIN_BAYOU 0
#define CHAIN_BENEATH 1
#define CHAIN_CANOPY 2
#define CHAIN_COBD 3
#define CHAIN_ECHOING 4
#define CHAIN_FAIRY_GLADE 5
#define CHAIN_FAIRY_REVISIT 6
#define CHAIN_IRON_MOUNT 7
#define CHAIN_MARSHES 8
#define CHAIN_MENHIR 9
#define CHAIN_PRECIPICE 10
#define CHAIN_PRISON 11
#define CHAIN_SANC_ROCK 12
#define CHAIN_SANC_STONE 13
#define CHAIN_SANC_WATER 14
#define CHAIN_SIDE_TEMPLE 15
#define CHAIN_TOMB 16
#define CHAIN_TOP 17
#define CHAIN_WALK_LIFE 18
#define CHAIN_WALK_POWER 19
#define CHAIN_WHALE 20
#define CHAIN_WOODS 21

#define CURRENT_VERSION "1.2.2"