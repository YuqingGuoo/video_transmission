#include <tran_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_TranOpr g_ptTranOprHead;

int register_tran_opr(PT_TranOpr ptTranOpr)
{
	PT_TranOpr ptTmp;

	if(!g_ptTranOprHead)
	{
		g_ptTranOprHead   = ptTranOpr;
		ptTranOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptTranOprHead;
		while(ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptTranOpr;
		ptTranOpr->ptNext = NULL;
	}
	return 0;
}


void show_tran_opr(void)
{
	int i = 0;
	PT_TranOpr ptTmp = g_ptTranOprHead;

	while(ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_TranOpr get_tran_opr(char* pcName)
{
	PT_TranOpr ptTmp = g_ptTranOprHead;

	while(ptTmp)
	{
		if(strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

int tran_init(void)
{
	int iError;
	iError = net_Tran_init();
	return iError;
}

int tran_chanel_init(PT_ContextDevice dev)
{
	PT_TranOpr ptTmp = g_ptTranOprHead;
	while(ptTmp)
	{
		if(ptTmp->isCanUse && ptTmp->TranInit)
		{
			ptTmp->TranInit(dev);
		}
		ptTmp = ptTmp->ptNext;
	}
	return -1;
}


int tran_send(PT_ContextDevice dev,unsigned char* Data, int cnt)
{
    int i = 0;
	PT_TranOpr ptTmp = g_ptTranOprHead;
	while(ptTmp)
	{
		i = ptTmp->TranSend(dev, Data, cnt);
		ptTmp = ptTmp->ptNext;
	}
	return i;
}





