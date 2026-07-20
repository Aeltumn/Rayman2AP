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
#include <r2console_api.h>

#pragma comment (lib, "crypt32")

#define LEVEL_COUNT 57
#define MAX_LEVEL_NAME_LENGTH 46
#define MAX_LENGTH 32
#define CHAIN_COUNT 22

#define CURRENT_VERSION "1.2.0"