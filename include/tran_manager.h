
#ifndef _TRAN_MANAGER_H
#define _TRAN_MANAGER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PACKBUFCNT 500
#define TABLECNT 2048

typedef struct ContextDevice{
    char *dstIp;
    int dstPort;
    struct sockaddr_in servAddr;
    int socket;
}T_ContextDevice,*PT_ContextDevice;


typedef struct TranOpr
{
	char* name;
	int isCanUse;
	int (*TranInit)(PT_ContextDevice dev);
	int (*TranExit)(PT_ContextDevice dev);
	int (*TranSend)(PT_ContextDevice dev,unsigned char* strData, int cnt);
	struct TranOpr* ptNext;
} T_TranOpr, *PT_TranOpr;


PT_TranOpr get_tran_opr(char* pcName);
int init_tran_chanel(void);
int register_tran_opr(PT_TranOpr ptTranOpr);
void show_tran_opr(void);
int tran_init(void);
int tran_send(PT_ContextDevice dev,unsigned char* Data, int cnt);
int tran_chanel_init(PT_ContextDevice dev);



#endif /* _DEBUG_MANAGER_H */
