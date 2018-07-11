#define _WIN32_WINNT 0x0500
#define MAXBUFF 512
#define BUFFMSGS 200000

#include <windows.h>
#include <time.h>
#include "resource.h"
BOOL CALLBACK ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

class Client{
public:
	HWND dialogo_chat;
	HWND dialogo_conectar;
	MSG messages;
	SOCKET s;
	sockaddr_in estructura;
	char buffer[MAXBUFF];
	char bufferMSGS[BUFFMSGS];
	char ip[32];
	char puerto[6];
	char nick[32];
	bool conectado;
	time_t tiempo;

	Client();
	bool Init();
	bool Conexion();
	bool Conectar();
	void Desconectar();
	bool Enviar();
	bool Mensaje();
	void Chequear();
	void CambiarNick();
	void Escribir(char* mensaje);
	bool Recibir();
	void Procesar();
	bool MainLoop();
	void LimpiarBuffer();
	void AgregarUsuario();
	void SacarUsuario();

}cliente;

Client::Client(){
	conectado = false;
	memset(buffer,0,MAXBUFF+BUFFMSGS+32+6+32);
	tiempo = time(NULL);
}

bool Client::Init(){
	dialogo_conectar = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(DIALOGO_CONECTAR), NULL, ToolDlgProc);
	dialogo_chat = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(DIALOGO_CHAT), NULL, ToolDlgProc);
	ShowWindow(dialogo_conectar,SW_SHOW);
	WSADATA wsd;
    WSAStartup(0x0202,&wsd);
	char tmp[64];
	sprintf(tmp,"\t\t\t---Inicializado correctamente%x ---",&buffer);
	Escribir(tmp);
	return true;
}

bool Client::MainLoop(){
	while (PeekMessage(&messages, NULL,0,0,PM_REMOVE))
    {
		if (!IsDialogMessage(dialogo_conectar, &messages)){
			if (!IsDialogMessage(dialogo_chat,&messages)){
				TranslateMessage(&messages);
				DispatchMessage(&messages);
			}
		}
		if(messages.message == WM_QUIT) return false;
    }
	if (conectado){
		Recibir();
		Chequear();
	}
	return true;
}

void Client::LimpiarBuffer(){
	memset(buffer,0,MAXBUFF);
}

bool Client::Conexion(){
	int port;
	sscanf(puerto,"%d",&port);
	sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr=inet_addr(ip);
    s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (s == INVALID_SOCKET)
		return false;
    if (connect(s, (sockaddr*)&serv, sizeof(sockaddr)) == SOCKET_ERROR)
		return false;
	unsigned long  b = 1;
    ioctlsocket(s,FIONBIO,&b);
    return true;
}

void Client::Escribir(char* mensaje){
	GetDlgItemText(dialogo_chat,CCHAT,bufferMSGS,BUFFMSGS-1);
    strcat(bufferMSGS,mensaje);
    strcat(bufferMSGS,"\r\n");
    SetDlgItemText(dialogo_chat,CCHAT,bufferMSGS);
	PostMessage(GetDlgItem(dialogo_chat,CCHAT),WM_VSCROLL,SB_BOTTOM,(LPARAM)NULL);
}

bool Client::Recibir(){
    int i = recv(s,buffer,MAXBUFF-1,0); 
    if (i > 0){
        Procesar();
		LimpiarBuffer();
		Sleep(1);
    }
    else if ( i == 0) {
        Desconectar();
        return false;
    }
    return true;
}

void Client::Procesar(){
	if(!strncmp(buffer,"-m",2)){
		char* mensaje = buffer+2;
		Escribir(mensaje);
	}
	else if(!strncmp(buffer,"-a",2)){
		memcpy(buffer,buffer+2,35);
		AgregarUsuario();
	}
	else if(!strncmp(buffer,"-d",2)){
		memcpy(buffer,buffer+2,35);
		SacarUsuario();
	}
}

void Client::Desconectar(){
	sprintf(buffer,"El servidor %s:%s ha caido.",ip,puerto);
	MessageBox(dialogo_chat,buffer,"Desconectado",MB_OK | MB_ICONEXCLAMATION);
	LimpiarBuffer();
	conectado = false;
	closesocket(s);
	ShowWindow(dialogo_chat,SW_HIDE);
	ShowWindow(dialogo_conectar,SW_SHOW);
}

bool Client::Mensaje(){
	if(conectado){
		sprintf(buffer,"%s","-m");
		GetDlgItemText(dialogo_chat,CMENSAJE,buffer+2,MAXBUFF-3);
		Enviar();
	}
	SetDlgItemText(dialogo_chat,CMENSAJE,"");
	return true;
}

bool Client::Enviar(){
	int i = send(s,buffer,strlen(buffer)+1,0);
	LimpiarBuffer();
    if (i == SOCKET_ERROR || i == 0){
        Desconectar();
        return false;
    }
	return true;
}

bool Client::Conectar(){
	GetDlgItemText(dialogo_conectar,CIP,ip,31);
	GetDlgItemText(dialogo_conectar,CPUERTO,puerto,5);
	GetDlgItemText(dialogo_conectar,CNICK,nick,31);
	if(!strlen(ip))
		sprintf(ip,"127.0.0.1");
	if(!strlen(puerto))
		sprintf(puerto,"1339");
	if(!strlen(nick)){
		MessageBox(dialogo_conectar,"Debes introducir un nick.","Error",MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	if(Conexion()){
		conectado = true;
		ShowWindow(dialogo_chat,SW_SHOW);
		ShowWindow(dialogo_conectar,SW_HIDE);
		sprintf(buffer,"Chat: Conectado a %s en el puerto %s",ip,puerto);
		SetWindowText(dialogo_chat,buffer);
		LimpiarBuffer();
		CambiarNick();
		int time = clock()+1000;
		return true;
	}
	MessageBox(dialogo_conectar,"Conexion fallida","Conexion",MB_OK | MB_ICONEXCLAMATION);
	return false;
}

void Client::CambiarNick(){
		sprintf(buffer,"%s%s","-n",nick);
		Enviar();
}

void Client::Chequear(){
	if (time(NULL) >= tiempo+5){
		tiempo +=5;
		buffer[0] = 'c';
		Enviar();
	}
}

void Client::AgregarUsuario(){
	SendDlgItemMessage(dialogo_chat,LUSUARIOS,LB_ADDSTRING,NULL,(LPARAM)buffer);
	LimpiarBuffer();
}

void Client::SacarUsuario(){
	int index = SendDlgItemMessage(dialogo_chat,LUSUARIOS,LB_FINDSTRING,-1,(LPARAM)buffer);
	if (index != LB_ERR)
		SendDlgItemMessage(dialogo_chat,LUSUARIOS,LB_DELETESTRING,(WPARAM)index,NULL);
	LimpiarBuffer();
}


BOOL CALLBACK ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
    switch(Message)
    {
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case BCONECTAR:
                    cliente.Conectar();
                break;
				case BENVIAR:
					cliente.Mensaje();

                break;
				case MSALIR:
					PostQuitMessage(0);
				break;
				case MACERCADE:
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(ACERCA_DE),cliente.dialogo_chat,ToolDlgProc);
				break;
				case BCNICK:
					GetDlgItemText(cliente.dialogo_chat,CCNICK,cliente.nick,31);
					cliente.CambiarNick();
				break;
				case IDOK:
					EndDialog( hwnd, TRUE );
					return TRUE;
				break;
            }
        break;

		case WM_CLOSE:
			if(hwnd == cliente.dialogo_chat || hwnd ==cliente.dialogo_conectar)
				PostQuitMessage(0);
		break;

        default:
            return FALSE;
    }
    return TRUE;
}
