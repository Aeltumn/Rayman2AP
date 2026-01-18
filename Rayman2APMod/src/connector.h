#pragma once

#define MESSAGE_TYPE_DEBUG 0
#define MESSAGE_TYPE_CHECK 1
#define MESSAGE_TYPE_COLLECTED 2
#define MESSAGE_TYPE_DEATH 3
#define MESSAGE_TYPE_SHUTDOWN 4
#define MESSAGE_TYPE_CONNECT 5
#define MESSAGE_TYPE_DISCONNECT 6
#define MESSAGE_TYPE_MESSAGE 7
#define MESSAGE_TYPE_COMPLETE 8
#define MESSAGE_TYPE_STATE 9
#define MESSAGE_TYPE_SETTINGS 10

#define LEVEL_COUNT 56

int MOD_StartConnector();
void MOD_StopConnector();

void MOD_RunPendingMessages();

void MOD_SendMessageE(int);
void MOD_SendMessage(int, const char*);
void MOD_HandleMessage(int, const char*);