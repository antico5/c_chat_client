#define _WIN32_WINNT 0x0500
#define MAXBUFF 512
#define BUFFMSGS 200000
#include <stdio.h>
#include <windows.h>
#include "resource.h"
#include "module.h"


int WINAPI WinMain (HINSTANCE hThisInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,int nFunsterStil){
	cliente.Init();
	while(1)
    {
		if (!cliente.MainLoop()) break;
		Sleep(1);
    }
}
