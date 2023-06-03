#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "Source_h.h"
#include <fstream>
#include <windows.h>
#include <stdio.h>

using namespace std;

#pragma comment (lib, "Rpcrt4.lib")

struct Client {
	char login[100];
	char password[100];
	bool access = 0;
	FILE* working = NULL;
	handle_t id = 0;
};

struct Client Clients[100];

int g_amount_of_clinets = 0;

void Output(/* [string][in] */ const unsigned char* szOutput)
{
	cout << szOutput << std::endl;
}


int Remove(const unsigned char* path, int idd) {
	if (ImpersonateLoggedOnUser(Clients[idd].id) == 0) {
		return -1;
	}
	if (remove((const char*)path) == -1)
	{
		return -2;
	}
}

int Download(/* [string][in] */int buf[1024], /* [string][in] */const unsigned char* path, int idd) {
	if (!ImpersonateLoggedOnUser(Clients[idd].id)) {
		cout << "Impersonate Error!";
		return -1;
	}
	if (Clients[idd].working == NULL)Clients[idd].working = fopen((const char*)path, "r");
	if (!Clients[idd].working) {
		Clients[idd].working = NULL;
		cout << "Acces Error!";
		return -2;
	}
	int count = fread((int*)buf, sizeof(int), 1024, Clients[idd].working);
	if (count != 1024) {
		fclose(Clients[idd].working);
		Clients[idd].working = NULL;
	}
	return count;

}

int Upload(/* [string][in] */const unsigned char* buf, /* [string][in] */const unsigned char* filename, /* [in] */int count, /* [in] */int size, /* [in] */int idd) {
	if (ImpersonateLoggedOnUser(Clients[idd].id) == 0) {
		cout << "Impersonate Error!";
		size = -1;
	}
	if (Clients[idd].working == NULL)Clients[idd].working = fopen((const char*)filename, "w");
	if (!Clients[idd].working) {
		Clients[idd].working = NULL;
		cout << "Acces Error!";
		return -2;
	}
	for (int i = 0; i < count; i++) {
		fputc(buf[i], Clients[idd].working);
	}
	if (count != 1024) {
		fclose(Clients[idd].working);
		Clients[idd].working = NULL;
	}
	return 1;
}

int Log_in(/* [string][in] */const unsigned char* login,/* [string][in] */ const unsigned char* password) { // -1 too many clients
	printf("%s %s\n", login, password);
	if (g_amount_of_clinets > 100)return -1;
	int current_index = g_amount_of_clinets;

	handle_t handle = 0;

	if (!LogonUserA((LPCSTR)login, NULL, (LPCSTR)password, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &handle))
	{
		cout << "Descriptor token error.";
		return -2;
	}
	if (!ImpersonateLoggedOnUser(handle))
	{
		cout << "Impersonation error.";
		return -3;
	}

	Clients[current_index].id = handle;
	strcpy(Clients[current_index].login, (char *)login);
	strcpy(Clients[current_index].password, (char *)password);
	g_amount_of_clinets++;
	cout << "Client is on." << endl;
	return g_amount_of_clinets-1;
}

RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE /*hInterface*/, void* /*pBindingHandle*/)
{
	return RPC_S_OK; // Always allow anyone.
}

int main() {
	/*
	setlocale(LC_ALL, "Rus");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	*/

	RPC_STATUS status;
	RpcServerRegisterAuthInfoA(nullptr, RPC_C_AUTHN_WINNT, 0, nullptr);
	//Функция RpcServerUseProtseqEp сообщает библиотеке времени выполнения RPC использовать указанную \
	последовательность протокола в сочетании с указанной конечной точкой для приема вызовов удаленных процедур.
	status = RpcServerUseProtseqEpA(
		(RPC_CSTR)("ncacn_ip_tcp"),			// Use TCP/IP protocol.
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT,		// Backlog queue length for TCP/IP.
		(RPC_CSTR)("9000"),					// TCP/IP port to use.
		NULL);								// No security.

	if (status)
		exit(status);

	//Функция RpcServerRegisterIf2 регистрирует интерфейс с библиотекой времени выполнения RPC.
	status = RpcServerRegisterIf2(
		RPC_v1_0_s_ifspec,              // Interface to register.
		NULL,                                // Use the MIDL generated entry-point vector.
		NULL,                                // Use the MIDL generated entry-point vector.
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, // Forces use of security callback.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Use default number of concurrent calls.
		(unsigned)-1,                        // Infinite max size of incoming data blocks.
		SecurityCallback);                   // Naive security callback.

	if (status)
		exit(status);
	cout << "Server in on" << endl;

	// Функция RpcServerListen сигнализирует библиотеке времени выполнения RPC прослушивать удаленные вызовы процедур
	status = RpcServerListen(
		1,                                   // Recommended minimum number of threads.
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,      // Recommended maximum number of threads.
		FALSE);                              // Start listening now.
	cout << "Server is off" << endl;
	if (status)
		exit(status);
}

// Memory allocation function for RPC.
// The runtime uses these two functions for allocating/deallocating
// enough memory to pass the string to the server.
void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
	free(p);
}
