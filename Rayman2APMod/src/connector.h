#pragma once

#define MESSAGE_TYPE_DEBUG 0
#define MESSAGE_TYPE_CHECK 1
#define MESSAGE_TYPE_ITEM 2
#define MESSAGE_TYPE_DEATH 3
#define MESSAGE_TYPE_UPDATE_DEATHLINK 4
#define MESSAGE_TYPE_TEST 5

int MOD_StartConnector();
void MOD_StopConnector();

void MOD_RunPendingMessages();

void MOD_SendMessage(int, const char*);
void MOD_HandleMessage(int, const char*);