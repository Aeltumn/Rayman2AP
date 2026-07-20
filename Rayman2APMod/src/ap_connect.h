#pragma once

#ifdef __cplusplus
class APListener {
public:
	void wait();
};

extern "C" {
#endif

bool AP_StartArchipelagoConnector();
void AP_StopArchipelagoConnector();

bool AP_IsConnected();
void AP_Connect(char* args);
void AP_Disconnect();
void AP_PrintConnectionStatus();

void AP_SendChat(char* message);
void AP_SendDeathLink(char* message);

void AP_MarkCollected(int id);
void AP_UpdateLums(int collected);
void AP_Complete();

#ifdef __cplusplus
}
#endif