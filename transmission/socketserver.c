#include <tran_manager.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

static int net_transfer_init(PT_ContextDevice udp)
{
	int iRet;

	udp->socket = socket(AF_INET, SOCK_DGRAM, 0);

	if(-1 == udp->socket)
	{
		printf("socket error!\n");
		return -1;
	}
	udp->servAddr.sin_family      = AF_INET;
	udp->servAddr.sin_port        = htons(udp->dstPort);    /* host to net, short */
    inet_aton(udp->dstIp, &udp->servAddr.sin_addr);

    iRet = (int)sendto(udp->socket, "", 1, 0, (struct sockaddr *)&udp->servAddr, sizeof(udp->servAddr));
    if (iRet != 1){
        printf("udpInit sendto test err. %d", iRet);
        close(udp->socket);
        return -1;
    }		
	return 0;
}



static int net_transfer_exit(PT_ContextDevice udp)
{

	close(udp->socket);
	return 0;
}


static int net_transfer_send(PT_ContextDevice udp,unsigned char* strData, int cnt)
{
    //int iAddrLen;
    int iSendLen;
    //iAddrLen = sizeof(struct sockaddr);
    iSendLen = sendto(udp->socket, strData, cnt, 0, (struct sockaddr *)&udp->servAddr, sizeof(udp->servAddr));
    //iSendLen = sendto(g_iSocketServer, strData, cnt, 0, (const struct sockaddr*) &g_tSocketClientAddr, iAddrLen);
    //printf("set %d msg\n", iSendLen);
    if (iSendLen != cnt)
    {
        printf("%s sendto err. %d %d\n", __FUNCTION__, iSendLen, cnt);
        return -1;
    }
	return iSendLen;
}


static T_TranOpr g_tNetTransferOpr =
{
	.name      = "UDP",
	.isCanUse  = 1,
	.TranInit  = net_transfer_init,
	.TranExit  = net_transfer_exit,
	.TranSend  = net_transfer_send,
	
};

int net_Tran_init(void)
{
	return register_tran_opr(&g_tNetTransferOpr);
}


