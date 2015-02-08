
/*
  main function
  lktOS network config moudle for all platform 
  
  v1.0 support mt7620  by luot  20121211
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>


#include <pthread.h>



#include "nvram.h"
#include "lktInitConfig.h"


#define FPRINT_NUM(x) fprintf(fp, #x"=%d\n", atoi(nvram_bufget(ralinkMode, #x)));
#define FPRINT_STR(x) fprintf(fp, #x"=%s\n", nvram_bufget(ralinkMode, #x));
#define FPRINT_CONFIG_STR(x,y) fprintf(fp, #y" %s\n", nvram_bufget(ralinkMode, #x));

char * replaceAll(char * src,char oldChar,char newChar)
{ 
	char * head=src;
	while(*src!='\0')
	{
		if(*src==oldChar) 
			*src=newChar;
		src++;
	}
	return head;
}

static int mtd_open(const char *name, int flags)
{
	FILE *fp;
	char dev[80];
	int i, ret;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, name)) {
				snprintf(dev, sizeof(dev), "/dev/mtd/%d", i);
				if ((ret = open(dev, flags)) < 0) {
					snprintf(dev, sizeof(dev), "/dev/mtd%d", i);
					ret = open(dev, flags);
				}
				fclose(fp);
				return ret;
			}
		}
		fclose(fp);
	}
	return -1;
}

static int flash_read_mac(char *buf)
{
	int fd, ret;

	if (!buf)
		return -1;
	fd = mtd_open("Factory", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device\n");
		return -1;
	}

	lseek(fd, 0x2E, SEEK_SET);

	ret = read(fd, buf, 6);
	close(fd);
	return ret;
}


//控制SWITCH每个端口电源断开或者开启函数，第一个参数为平台类型，第二个参数为端口号，第三个参数为模式开启或者关闭
static int switchPowerControl(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned int portNum, unsigned char mode)
{
	char cmdbuf[64]={0};

	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		//好像5350也是他们用的内置SWITCH应该是一样的
		//在7620包的SWITCH中PHY 的0寄存器的第11位来控制电源来控制
		if(mode)
		{
			sprintf(cmdbuf,"mii_mgr -s -p %d -r 0 -v 0x3100",portNum);
		}
		else
		{
			sprintf(cmdbuf,"mii_mgr -s -p %d -r 0 -v 0x3900",portNum);
		}
		system(cmdbuf);
	}
	else
	{
		return 0;
	}

	return 1;
}

/*
 * substitution of getNthValue which dosen't destroy the original value
 */
int getNthValueSafe(int index, char *value, char delimit, char *result, int len)
{
    int i=0, result_len=0;
    char *begin, *end;

    if(!value || !result || !len)
        return -1;

    begin = value;
    end = strchr(begin, delimit);

    while(i<index && end){
        begin = end+1;
        end = strchr(begin, delimit);
        i++;
    }

    //no delimit
    if(!end){
		if(i == index){
			end = begin + strlen(begin);
			result_len = (len-1) < (end-begin) ? (len-1) : (end-begin);
		}else
			return -1;
	}else
		result_len = (len-1) < (end-begin)? (len-1) : (end-begin);

	memcpy(result, begin, result_len );
	*(result+ result_len ) = '\0';

	return 0;
}



//生成ralink系列的DHCP服务器配置文件
static int genRalinkLanDhcpdConfigFile(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform, char* lanif) 
{
	FILE *fp;
	int intVal=0;
	int ralinkMode=RT2860_NVRAM;
	int staticDhcpEnable=1;
	char *tmpStr=NULL;
	//char *headStr=NULL;
	

	nvram_init(ralinkMode);

	intVal = atoi(nvram_bufget(ralinkMode, "dhcpEnabled"));

	if(0 == intVal)
	{
		nvram_close(ralinkMode);
		return 0;
	}
	
	if(platform == MTK7620_STD || platform == RALINK5350_STD)
	{
		fp = fopen("/etc/udhcpd.conf", "w+");
		if(fp == NULL)
		{
			printf("\r\nopen udhcpd  config file error");
			nvram_close(ralinkMode);
			return 0;
		}
		
		fprintf(fp, "start %s\n", nvram_bufget(ralinkMode, "dhcpStart"));
		fprintf(fp, "end %s\n", nvram_bufget(ralinkMode, "dhcpEnd"));
		fprintf(fp, "interface %s\n", lanif);
		fprintf(fp, "option subnet %s\n", nvram_bufget(ralinkMode, "dhcpMask"));
		fprintf(fp, "option dns %s %s\n", nvram_bufget(ralinkMode, "dhcpPriDns"),nvram_bufget(ralinkMode, "dhcpSecDns"));
		fprintf(fp, "option router %s\n", nvram_bufget(ralinkMode, "dhcpGateway"));
		fprintf(fp, "option lease %s\n", nvram_bufget(ralinkMode, "dhcpLease"));
		fprintf(fp, "lease_file %s\n", "/var/udhcpd.leases");
		
		//staticDhcpEnable= atoi(nvram_get(ralinkMode, "staticDhcpEnabled"));

		tmpStr=nvram_bufget(ralinkMode, "staticDhcpRules");
#if 1
		char rec[128];
		int i = 0;
		char tmp1[50], tmp2[50];

		while(getNthValueSafe(i++, tmpStr, ';', rec, sizeof(rec)) != -1 ){
			if((getNthValueSafe(0, rec, ',', tmp1, sizeof(tmp1)) == -1)){
				continue;
			}
			
			if((getNthValueSafe(1, rec, ',', tmp2, sizeof(tmp2)) == -1)){
				continue;
			}

			fprintf(fp, "static_lease %s %s\n", tmp1, tmp2);
		}
#else
		headStr=tmpStr;
		tmpStr=strchr(tmpStr,'@');
		if(staticDhcpEnable)
		{
			intVal=1;
			while(1)
			{
				char ipmacBuf[32];
				if(tmpStr==NULL)
					break;
				*tmpStr='\0';
				strcpy(ipmacBuf,headStr);
				fprintf(fp, "static_lease %s\n", replaceAll(ipmacBuf,',', ' '));
				headStr=tmpStr+1;
				tmpStr=strchr(tmpStr+1,'@');
				intVal++;
			}
		}
#endif
	}
	else
	{
		nvram_close(ralinkMode);
		fclose(fp);
		return 0;
	}
	// 暂时不加入对SUPER DMZ的DHCP支持

	nvram_close(ralinkMode);
	fclose(fp);

	system("rm -f /var/udhcpd.leases");
	return 1;
}


static int ralinkLanInit(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned wanPort)
{
	int i=0;
	int ralinkMode=RT2860_NVRAM;
	char cmdBuf[128]={0};
	int flag=0;
	FILE *fp1;
	int opMode=0;
	int intVal=0;
/*	
	if(0 == wanPort)
	{
		for(i=1;i<=4;i++)
		{
			switchPowerControl(platform,i,0);
		}
	}
	else if(4 == wanPort)
	{
		for(i=0;i<=3;i++)
		{
			switchPowerControl(platform,i,0);
		}
	}
	else
	{
		return 0;
	}
*/
	nvram_init(ralinkMode);

	system("ifconfig br0 down");
	sprintf(cmdBuf,"ifconfig br0 %s %s up",nvram_bufget(ralinkMode, "lan_ipaddr"),nvram_bufget(ralinkMode, "lan_netmask"));
	system(cmdBuf);
	if(nvram_bufget(ralinkMode, "HostName") != NULL)
	{
		sprintf(cmdBuf,"hostname %s",nvram_bufget(ralinkMode, "HostName"));
	}
	else
	{
		sprintf(cmdBuf,"hostname %s","ralink");
	}
	system(cmdBuf);
	fp1 = fopen("/etc/hosts", "w");
	if(fp1 != NULL)
	{
		fprintf(fp1, "127.0.0.1 localhost.localdomain localhost\n");
		fprintf(fp1, "%s %s.ralinktech.com %s\n", nvram_bufget(ralinkMode, "lan_ipaddr"),nvram_bufget(ralinkMode, "HostName"),nvram_bufget(ralinkMode, "HostName"));
		fclose(fp1);
	}
		
	nvram_close(ralinkMode);

	system("killall udhcpd");
	flag=genRalinkLanDhcpdConfigFile(platform,"br0");
	if(flag != 0)
	{
		system("udhcpd /etc/udhcpd.conf");
	}
/*
	if(0 == wanPort)
	{
		for(i=1;i<=4;i++)
		{
			switchPowerControl(platform,i,1);
		}
	}
	else if(4 == wanPort)
	{
		for(i=0;i<=3;i++)
		{
			switchPowerControl(platform,i,1);
		}
	}
	else
	{
		return 0;
	}
*/
	//upnp
	opMode = atoi(nvram_bufget(ralinkMode, "OperationMode"));
	if (0 != opMode)
	{
		intVal = atoi(nvram_bufget(ralinkMode, "upnpEnabled"));	
		if (1 == intVal)
		{
			system("/sbin/miniupnpd.sh init");
		}
		else
		{
			system("/sbin/miniupnpd.sh remove");
		}
	}
	
	return 1;
}

static int pingFlag=253;

unsigned short cal_chksum(unsigned short *addr,int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short check_sum = 0;
 
    while(nleft>1)       //ICMP包头以字（2字节）为单位累加
    {
        sum += *w++;
        nleft -= 2;
    }
 
    if(nleft == 1)      //ICMP为奇数字节时，转换最后一个字节，继续累加
    {
        *(unsigned char *)(&check_sum) = *(unsigned char *)w;
        sum += check_sum;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    check_sum = ~sum;   //取反得到校验和
    return check_sum;
}


int unpack(char *buf,int len)
{
    int i;
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    double rtt;
 
 
    ip = (struct ip *)buf;
    iphdrlen = ip->ip_hl << 2;
    icmp = (struct icmp *)(buf + iphdrlen);
    len -= iphdrlen;
    if(len < 8)
    {
        printf("ICMP packet\'s length is less than 8\n");
        return -1;
    }
    //if((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
	if((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
    {
        
        printf("%d bytes from %s: icmp_seq=%u ttl=%d time= ms\n",
                len,inet_ntoa(from.sin_addr),
                icmp->icmp_seq,ip->ip_ttl);
    }
    else return -1;
}


int recv_packet(const int fd)
{
	char recpacketBuf[4096]={0};
    int n,fromlen;
	int iRecvLen = 0;
    struct sockaddr_in stFromAddr = {0};
	
    fromlen = sizeof(stFromAddr);
	memset(recpacketBuf, 0, 4 * 1024);
	/*
    if(nreceived < nsend)
    {   
        if((n = recvfrom(sockfd,recvpacket,sizeof(recvpacket),0,
            (struct sockaddr *)&from,&fromlen)) < 0)
        {
            perror("recvfrom error");
        }
        gettimeofday(&tvrecv,NULL);    
        unpack(recvpacket,n);      
        nreceived++;
    }
    */
	iRecvLen = recvfrom(fd, (void *)recpacketBuf, sizeof(recpacketBuf), 0, (struct sockaddr *)&stFromAddr,&fromlen);
	if(iRecvLen >= 0)
	{
		printf("\r\nrec a packet");
		return 0;
	}
	else
	{
		perror("recvfrom error");
		printf("\r\nnot rrrr");
		return -1;
	}
	return;
}
 

static void sendIcmp(const int fd, const struct sockaddr_in *pstDestAddr)

{

	char sendpacketBuf[4096]={0};
	unsigned char ucEchoNum = 0;
	int iPktLen = 0;
	int iRet = 0;
	struct icmp *pstIcmp = NULL;


	while(3 > ucEchoNum)
	{
		memset(sendpacketBuf, 0, sizeof(sendpacketBuf));
		pstIcmp = (struct icmp *)sendpacketBuf;

		pstIcmp->icmp_type = ICMP_ECHO;
		pstIcmp->icmp_code = 0;
		pstIcmp->icmp_cksum = 0;
		pstIcmp->icmp_seq = htons((unsigned short)ucEchoNum);
		pstIcmp->icmp_id = htons((unsigned short)getpid());
		pstIcmp->icmp_cksum = cal_chksum((unsigned short *)pstIcmp, 56 + 8);


		iRet = sendto(fd, pstIcmp, 56+8, 0, (struct sockaddr *)pstDestAddr, sizeof(struct sockaddr_in));

		ucEchoNum++;

	}

}


static int myPing(char *targetIp)
{
	int pingfd=0;
	int iRet=0;
	int recFlag=-1;
	int iRcvBufSize= 20 *1024;
	struct sockaddr_in stDestAddr = {0};
	struct timeval stRcvTimeOut = {0};
	
	struct protoent *pProtoIcmp = NULL;

	/*
	if((pProtoIcmp = getprotobyname("icmp")) == NULL)
	{
		printf("\nget icmp protocol error");
		return -1;
	}
	*/
	//pingfd = socket(PF_INET,SOCK_RAW,pProtoIcmp->p_proto);
	pingfd = socket(PF_INET,SOCK_RAW,IPPROTO_ICMP);
	
	if(0 > pingfd)
	{
		printf("\ncreate socket error");
		return -2;
	}
	iRet = setuid(getuid());
	if(0 > iRet)
	{
		printf("\nsetuid error");
		close(pingfd);
		return -3;
	}

	stRcvTimeOut.tv_usec = 1000 * 1000;
	setsockopt(pingfd, SOL_SOCKET, SO_RCVBUF, &iRcvBufSize, sizeof(iRcvBufSize));
	setsockopt(pingfd, SOL_SOCKET, SO_RCVTIMEO, &stRcvTimeOut, sizeof(struct timeval));

	if(inet_aton(targetIp,&stDestAddr.sin_addr) == 0)
    {
        printf("ping target IP Address Error!\n");
		close(pingfd);
        return -4;
    }
	sendIcmp(pingfd, &stDestAddr);
	recFlag=recv_packet(pingfd);
	if(recFlag == 0)
	{
		printf("\r\npacket from--%s\r\n",targetIp);
	}
	close(pingfd);
	return 0;
}

static void *ipcamtest_thead_create(void *arg)
{
	char *targetIp=NULL;
	char theadBuf[256]={0};
	int pthead_flag=253;
	targetIp=(char*)arg;
	myPing(targetIp);
	//sprintf(theadBuf,"ping %s",targetIp);
	//pthead_flag=system(theadBuf);
	//if(pthead_flag == 0)
	//{
		//pingFlag = 0;
	//}
	//return (void)0;
}

static int ralinkTestmodeInit(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned wanPort)
{
	int i=0,j=1;
	int ralinkMode=RT2860_NVRAM;
	char cmdBuf[128]={0};
	char cmdBuf1[128]={0};
	char ipcambuf1[32]={0};
	char ipcambuf2[32]={0};
	char ipcambuf3[32]={0};
	char ipcambuf4[32]={0};
	char *tmpptr1=NULL;
	char ourtestip[64]={0};
	int flag=0;
	int testMode=0;
	FILE *fp;
	int opMode=0;
	int intVal=0;
	int testFlag=0;
	pthread_t tidps[254];

	nvram_init(ralinkMode);
	
	testMode = atoi(nvram_bufget(ralinkMode, "testMode"));
	if(1 != testMode)
	{
		printf("\r\nnot test mode so exit\r\n");
		nvram_close(ralinkMode);
		return 0;
	}
	for(i=1;i<5;i++)
	{
		printf("\r\nfor test \r\n");
		sprintf(cmdBuf1,"testIpcam%d",i);
		strcpy(ipcambuf1,nvram_bufget(ralinkMode, cmdBuf1));
		printf("\r\nbuf=%s\r\n",ipcambuf1);
		tmpptr1=strchr(ipcambuf1,'.');
		if(tmpptr1 == NULL)
		{
			continue;
		}
		tmpptr1=strchr(tmpptr1+1,'.');
		if(tmpptr1 == NULL)
			continue;
		tmpptr1=strchr(tmpptr1+1,'.');
		if(tmpptr1 == NULL)
			continue;
		strcpy(ipcambuf2,ipcambuf1);
		*tmpptr1 = '\0';
		sprintf(ourtestip,"%s.155",ipcambuf1);
		printf("\r\n%d our ip111 is=\r\n",i,ourtestip);
		system("ifconfig br0 down");
		sprintf(cmdBuf,"ifconfig br0 %s netmask %s up",ourtestip,nvram_bufget(ralinkMode, "lan_netmask"));		
		system(cmdBuf);
		/*
		for(j=1;j<101;j++)
		{
			if(j==155)
			{
				continue;
			}
			sprintf(ourtestip,"%s.%d",ipcambuf1,j);
			printf("\r\npingip=%s\r\n",ourtestip);
			pthread_create(&tidps[j], NULL, ipcamtest_thead_create, (void *)ourtestip);
		}
		for(j=1;j<101;j++)
		{
			if(j==155)
			{
				continue;
			}
			pthread_join(tidps[j], NULL);
		}
		*/
		for(j=1;j<101;j++)
		{
			if(j==155)
			{
				continue;
			}
			sprintf(ourtestip,"%s.%d",ipcambuf1,j);
			printf("\r\npingip=%s\r\n",ourtestip);
			myPing(ourtestip);
			
		}
		//sprintf(cmdBuf,"ping %s",ipcambuf2);
		//pingFlag=system(cmdBuf);
		/*
   		printf("\r\nping flag=%d\r\n",pingFlag);
		sprintf(ourtestip,"%s.%d",ipcambuf1,5);
		myPing(ourtestip);
		sprintf(ourtestip,"%s.%d",ipcambuf1,11);
		myPing(ourtestip);
		*/
		pingFlag=0;
   
  		if( 0 == pingFlag)
  		{
  			printf("\r\nfind ipcam %s\r\n",ipcambuf2);
  			testFlag = 1;
			break;
  		}
	}
	if(testFlag)
	{
		printf("\r\n find it start ipcom config");
		system("killall udhcpd");
		fp = fopen("/etc/udhcpd.conf", "w+");
		if(fp == NULL)
		{
			printf("\r\nopen udhcpd  config file error");
			nvram_close(ralinkMode);
			return 0;
		}
		sprintf(ipcambuf3,"%s.156",ipcambuf1);
		fprintf(fp, "start %s\n", ipcambuf3);
		sprintf(ipcambuf3,"%s.186",ipcambuf1);
		fprintf(fp, "end %s\n", ipcambuf3);
		fprintf(fp, "interface %s\n", "br0");
		fprintf(fp, "option subnet %s\n", nvram_bufget(ralinkMode, "lan_netmask"));
		fprintf(fp, "option dns %s %s\n", ourtestip,ourtestip);
		fprintf(fp, "option router %s\n", ourtestip);
		fprintf(fp, "option lease %s\n", nvram_bufget(ralinkMode, "dhcpLease"));
		fprintf(fp, "lease_file %s\n", "/var/udhcpd.leases");
		fclose(fp);
		system("udhcpd /etc/udhcpd.conf");
		nvram_close(ralinkMode);
	}
	else
	{
		printf("\r\n no ipcam so use the old");
		system("ifconfig br0 down");
		sprintf(cmdBuf,"ifconfig br0 %s netmask %s up",nvram_bufget(ralinkMode, "lan_ipaddr"),nvram_bufget(ralinkMode, "lan_netmask"));
		system(cmdBuf);
		nvram_close(ralinkMode);
		system("killall udhcpd");
		flag=genRalinkLanDhcpdConfigFile(platform,"br0");
		if(flag != 0)
		{
			system("udhcpd /etc/udhcpd.conf");
		}
	}
	
	return 1;
}


static int ralinkWanInit(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned wanPort)
{
	int i=0;
	int ralinkMode=RT2860_NVRAM;
	char cmdBuf[128]={0};
	char tmBuf[64]={0};
	int opMode=0;
	char  wanModeStr[20]={0};
	char  wanIfName[8]={"eth2.2"};
	int intVal=0;
	FILE* fp;
	unsigned char buf[1024];
	unsigned char wan_mac[32];

	if(0 == wanPort)
	{
		switchPowerControl(platform,0,0);
	}
	else if(4 == wanPort)
	{
		
		switchPowerControl(platform,4,0);
	}
	else
	{
		return 0;
	}
	system("killall -q syslogd");
	system("killall -q udhcpc");
	system("killall -q l2tpd");
	system("killall -q openl2tpd");
	system("killall -q ppp_daemon");
	system("killall -q pppd");
	
	system("killall -q 3gdial");
	system("killall -q hotplugd");
	system("killall -q check_usb");
	system("killall -q checkat");
	system("killall -q dnsmasq");
	
	nvram_init(ralinkMode);

	opMode = atoi(nvram_bufget(ralinkMode, "OperationMode"));

	if((1 != opMode) && (3 != opMode))
	{
		printf("\r\nerror opMode so exit");
		nvram_close(ralinkMode);
		switchPowerControl(platform,wanPort,1);
		return 0;
	}
	if(3 == opMode)
	{
		strcpy(wanIfName,"apcli0");
	}
	// mac clone
	intVal = atoi(nvram_bufget(ralinkMode, "macCloneEnabled"));	

	if(1 == intVal)
	{
		sprintf(cmdBuf,"ifconfig %s down",wanIfName);
		system(cmdBuf);
		sprintf(cmdBuf,"ifconfig %s hw ether %s",wanIfName,nvram_bufget(ralinkMode, "macCloneMac"));
		system(cmdBuf);
		sprintf(cmdBuf,"ifconfig %s up",wanIfName);
		system(cmdBuf);
	}
	else
	{
		flash_read_mac(buf);
		sprintf(wan_mac,"%02X:%02X:%02X:%02X:%02X:%02X\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);

		sprintf(cmdBuf,"ifconfig %s down",wanIfName);
		system(cmdBuf);
		sprintf(cmdBuf,"ifconfig %s hw ether %s",wanIfName,wan_mac);
		system(cmdBuf);
		sprintf(cmdBuf,"ifconfig %s up",wanIfName);
		system(cmdBuf);
	}

	strcpy(wanModeStr,nvram_bufget(ralinkMode, "wanConnectionMode"));

	if(!strncmp(wanModeStr,"STATIC",6))
	{
		sprintf(cmdBuf,"ifconfig %s %s netmask %s",wanIfName,nvram_bufget(ralinkMode, "wan_ipaddr"),nvram_bufget(ralinkMode, "wan_netmask"));
		system(cmdBuf);
		system("route del default");
		sprintf(cmdBuf,"route add default gw %s",nvram_bufget(ralinkMode, "wan_gateway"));
		system(cmdBuf);
		fp = fopen("/etc/resolv.conf", "w+");
		if(fp == NULL)
		{
			printf("\r\nopen dns  config file error");
			nvram_close(ralinkMode);
			switchPowerControl(platform,wanPort,1);
			return 0;
		}
		fprintf(fp, "nameserver %s\n", nvram_bufget(ralinkMode, "wan_primary_dns"));
		if(nvram_bufget(ralinkMode, "wan_secondary_dns") != NULL)
		{
			fprintf(fp, "nameserver %s\n", nvram_bufget(ralinkMode, "wan_secondary_dns"));
		}
		fclose(fp);
		system("dnsmasq /etc/resolv.conf &");
		switchPowerControl(platform,wanPort,1);
		system("nat.sh");
	}
	else if(!strncmp(wanModeStr,"DHCP",4))
	{
		sprintf(cmdBuf,"ifconfig %s 0.0.0.0",wanIfName);
		system(cmdBuf);
		if(*(nvram_bufget(ralinkMode, "wan_dhcp_hn")) != '\0')
		{
			sprintf(cmdBuf,"udhcpc -i %s -h %s -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &",wanIfName,nvram_bufget(ralinkMode, "wan_dhcp_hn"));
			system(cmdBuf);
		}
		else
		{
			sprintf(cmdBuf,"udhcpc -i %s  -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &",wanIfName);
			system(cmdBuf);
		}
		switchPowerControl(platform,wanPort,1);
	}
	else if(!strncmp(wanModeStr,"PPPOE",5))
	{
		system("ifconfig ppp0 down");
		sprintf(cmdBuf,"ifconfig %s 0.0.0.0","eth2.2");
		system(cmdBuf);
		switchPowerControl(platform,wanPort,1);
		if(nvram_bufget(ralinkMode, "wan_pppoe_opmode") == NULL)
		{
			sprintf(cmdBuf,"pppoecd %s -u \"%s\" -p \"%s\"",wanIfName,nvram_bufget(ralinkMode, "wan_pppoe_user_mm"),nvram_bufget(ralinkMode, "wan_pppoe_pass"));
			system(cmdBuf);
		}
		else
		{
			strcpy(tmBuf,nvram_bufget(ralinkMode, "wan_pppoe_opmode"));
			system("route del default");
			system("syslogd -m 0");
			fp = fopen("/etc/options.pppoe", "w+");
			if(fp == NULL)
			{
				printf("\r\nopen pppoe  config file error");
				nvram_close(ralinkMode);
				switchPowerControl(platform,wanPort,1);
				return 0;
			}
			fprintf(fp, "noauth\n");
			fprintf(fp, "user %s\n", nvram_bufget(ralinkMode, "wan_pppoe_user_mm"));
			fprintf(fp, "password %s\n", nvram_bufget(ralinkMode, "wan_pppoe_pass"));
			fprintf(fp, "hide-password\n");
			fprintf(fp, "noipdefault\n");
			fprintf(fp, "defaultroute\n");
			fprintf(fp, "nodetach\n");
			fprintf(fp, "usepeerdns\n");
			if(!strncmp(tmBuf,"KeepAlive",9))
			{
				fprintf(fp, "persist\n");
			}
			else if(!strncmp(tmBuf,"OnDemand",8))
			{
				fprintf(fp, "demand\n");
				fprintf(fp, "idle %d\n", atoi(nvram_bufget(ralinkMode, "wan_pppoe_optime"))*60);
			}
			else
			{
			}
			fprintf(fp, "ipcp-accept-remote\n");
			fprintf(fp, "ipcp-accept-local\n");
			fprintf(fp, "lcp-echo-failure 3\n");
			fprintf(fp, "lcp-echo-interval 15\n");
			fprintf(fp, "ktune\n");
			fprintf(fp, "default-asyncmap nopcomp noaccomp\n");
			fprintf(fp, "novj nobsdcomp nodeflate\n");
			fprintf(fp, "plugin /etc_ro/ppp/plugins/rp-pppoe.so %s\n", wanIfName);
			fclose(fp);
			system("pppd file /etc/options.pppoe &");	
		}
	}	
	else if(!strncmp(wanModeStr,"L2TP",4))
	{
		system("ifconfig ppp0 down");
		switchPowerControl(platform,wanPort,1);
		if(atoi(nvram_bufget(ralinkMode, "wan_l2tp_mode")) == 0)
		{
			sprintf(cmdBuf,"ifconfig %s %s netmask %s",wanIfName,nvram_bufget(ralinkMode, "wan_l2tp_ip"),nvram_bufget(ralinkMode, "wan_l2tp_netmask"));
			system(cmdBuf);
			system("route del default");
			sprintf(cmdBuf,"route add default gw %s",nvram_bufget(ralinkMode, "wan_l2tp_gateway"));
			system(cmdBuf);
			
		}
		else
		{
			sprintf(cmdBuf,"udhcpc -i %s  -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &",wanIfName);
			system(cmdBuf);
		}
		strcpy(tmBuf,nvram_bufget(ralinkMode, "wan_l2tp_opmode"));
		system("rm -f /var/run/openl2tpd.pid");
		fp = fopen("/etc/openl2tpd.conf", "w+");
		if(fp == NULL)
		{
			printf("\r\nopen openl2tp  config file error");
			nvram_close(ralinkMode);	
			return 0;
		}
			fprintf(fp, "ppp profile modify profile_name=default \\\n");
			fprintf(fp, "auth_eap=no auth_mschapv1=no auth_mschapv2=no \\\n");
			fprintf(fp, "default_route=yes \n");
			fprintf(fp, "tunnel create tunnel_name=ralink dest_ipaddr=%s \\\n", nvram_bufget(ralinkMode, "wan_l2tp_server"));
			if(!strncmp(tmBuf,"KeepAlive",9))
			{
				fprintf(fp, "persist=yes\n");
			}
			else if(!strncmp(tmBuf,"OnDemand",8))
			{
				fprintf(fp, "persist=no\n");
				fprintf(fp, "idle_timeout=%d\n", atoi(nvram_bufget(ralinkMode, "wan_l2tp_optime"))*60);
			}
			else
			{
			}
			fprintf(fp, "session create tunnel_name=ralink \\\n");
			fprintf(fp, "session_name=ralink \\\n");
			//fprintf(fp, "lcp-echo-failure 10\n");
			//fprintf(fp, "lcp-echo-interval 30\n");
			//fprintf(fp, "ktune\n");
			//fprintf(fp, "default-asyncmap nopcomp noaccomp\n");
			//fprintf(fp, "novj nobsdcomp nodeflate\n");
			fprintf(fp, "user_name=%s\\\n", nvram_bufget(ralinkMode, "wan_l2tp_user"));
			fprintf(fp, "user_password=%s\n", nvram_bufget(ralinkMode, "wan_l2tp_pass"));
			fclose(fp);
			system("openl2tpd");
			system("ppp_daemon &");
	}	
	else if(!strncmp(wanModeStr,"PPTP",4))
	{
		system("ifconfig ppp0 down");
		switchPowerControl(platform,wanPort,1);
		if(atoi(nvram_bufget(ralinkMode, "wan_pptp_mode")) == 0)
		{
			sprintf(cmdBuf,"ifconfig %s %s netmask %s",wanIfName,nvram_bufget(ralinkMode, "wan_pptp_ip"),nvram_bufget(ralinkMode, "wan_pptp_netmask"));
			system(cmdBuf);
			system("route del default");
			sprintf(cmdBuf,"route add default gw %s",nvram_bufget(ralinkMode, "wan_pptp_gateway"));
			system(cmdBuf);
			
		}
		else
		{
			sprintf(cmdBuf,"udhcpc -i %s  -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &",wanIfName);
			system(cmdBuf);
		}
		strcpy(tmBuf,nvram_bufget(ralinkMode, "wan_pptp_opmode"));
		system("mkdir -p /etc/ppp");
		fp = fopen("/etc/options.pptp", "w+");
		if(fp == NULL)
		{
			printf("\r\nopen pptp  config file error");
			nvram_close(ralinkMode);	
			return 0;
		}
		fprintf(fp, "noauth\n");
		fprintf(fp, "refuse-eap\n");
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "user %s\n", nvram_bufget(ralinkMode, "wan_pptp_user"));
		fprintf(fp, "password %s\n", nvram_bufget(ralinkMode, "wan_pptp_pass"));
		fprintf(fp, "connect true\n");
		//fprintf(fp, "require-mppe-128\n");
		//fprintf(fp, "nomppe-stateful\n");
		fprintf(fp, "plugin \"pptp.so\"\n");
		fprintf(fp, "pptp_server %s \n", nvram_bufget(ralinkMode, "wan_pptp_server"));
		//fprintf(fp, "lock\n");
		fprintf(fp, "maxfail 0\n");
		fprintf(fp, "usepeerdns\n");
			
		if(!strncmp(tmBuf,"KeepAlive",9))
		{
			fprintf(fp, "persist\n");
			//fprintf(fp, "holdoff 0\n");
			fprintf(fp, "holdoff %s\n", nvram_bufget(ralinkMode, "wan_pptp_optime"));

		}
		else if(!strncmp(tmBuf,"OnDemand",8))
		{
			fprintf(fp, "demand\n");
			fprintf(fp, "idle %d\n", atoi(nvram_bufget(ralinkMode, "wan_pptp_optime"))*60);
		}
		else
		{
		}
		fprintf(fp, "defaultroute\n");
		fprintf(fp, "ipcp-accept-remote ipcp-accept-local noipdefault\n");
		fprintf(fp, "ktune\n");
		fprintf(fp, "default-asyncmap nopcomp noaccomp\n");
		fprintf(fp, "novj nobsdcomp nodeflate\n");
		fprintf(fp, "lcp-echo-interval 10\n");
		fprintf(fp, "lcp-echo-failure 6\n");
		fprintf(fp, "unit 0\n");
		fclose(fp);
		system("pppd file /etc/options.pptp  &");	
		system("ppp_daemon &");
	}	
	else if(!strncmp(wanModeStr,"3G",2))
	{
		//this use the script
		switchPowerControl(platform,wanPort,1);
		system("3gdial.sh &");
	}
	else
	{
		printf("unknow type!!\r\n");
		switchPowerControl(platform,wanPort,1);
	}
	nvram_close(ralinkMode);
	return 1;
}

int lktos_networkconfig_init_lan(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned char* errormsg)
{
	int resultFlag=0;

	strcpy(errormsg,"nothing!");
	if(errormsg == NULL)
	{
		printf("\r\n the second papa error");
		return resultFlag;
	}
	if((platform <= UNDEFINED) || (platform >= UNKNOW) )
	{
		printf("\r\n the first para(platform type) error");
		return resultFlag;
	}
	switch(platform)
	{
		case RALINK3052_STD:
			
			break;
		case RALINK3050_STD:
			
			break;
		case RALINK5350_STD:
			resultFlag=ralinkLanInit(platform,0);
			break;
		case MTK7620_STD:
			resultFlag=ralinkLanInit(platform,0);
			break;
		default:
			strcpy(errormsg,"wrong palt form!");
			return resultFlag;
			
	}
	return resultFlag;
}


int lktos_networkconfig_init_testmode(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned char* errormsg)
{
	int resultFlag=0;

	strcpy(errormsg,"nothing!");
	if(errormsg == NULL)
	{
		printf("\r\n the second papa error");
		return resultFlag;
	}
	if((platform <= UNDEFINED) || (platform >= UNKNOW) )
	{
		printf("\r\n the first para(platform type) error");
		return resultFlag;
	}
	switch(platform)
	{
		case RALINK3052_STD:
			
			break;
		case RALINK3050_STD:
			
			break;
		case RALINK5350_STD:
			resultFlag=ralinkTestmodeInit(platform,0);
			break;
		case MTK7620_STD:
			resultFlag=ralinkLanInit(platform,0);
			break;
		default:
			strcpy(errormsg,"wrong palt form!");
			return resultFlag;
			
	}
	return resultFlag;
}


int lktos_networkconfig_init_wan(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned char* errormsg)
{
	int resultFlag=0;

	strcpy(errormsg,"nothing!");
	if(errormsg == NULL)
	{
		printf("\r\n the second papa error");
		return resultFlag;
	}
	if((platform <= UNDEFINED) || (platform >= UNKNOW) )
	{
		printf("\r\n the first para(platform type) error");
		return resultFlag;
	}
	switch(platform)
	{
		case RALINK3052_STD:
			
			break;
		case RALINK3050_STD:
			
			break;
		case RALINK5350_STD:
			resultFlag=ralinkWanInit(platform,4);//need action
			system("nat.sh");
//			system("3gdial.sh &");
			break;
		case MTK7620_STD:
			resultFlag=ralinkWanInit(platform,0);
			system("nat.sh");
			break;
		default:
			strcpy(errormsg,"wrong palt form!");
			return resultFlag;
			
	}
	return resultFlag;
}

int lktos_networkconfig_gen_lanDhcpdConfig(T_LKTOS_INITCONFIG_PLATFORM_TYPE_ platform,unsigned char* errormsg)
{
	int resultFlag=0;

	strcpy(errormsg,"nothing!");
	if(errormsg == NULL)
	{
		printf("\r\n the second papa error");
		return resultFlag;
	}
	if((platform <= UNDEFINED) || (platform >= UNKNOW) )
	{
		printf("\r\n the first para(platform type) error");
		return resultFlag;
	}
	switch(platform)
	{
		case RALINK3052_STD:
			
			break;
		case RALINK3050_STD:
			break;
		case RALINK5350_STD:
			resultFlag=genRalinkLanDhcpdConfigFile(platform,"br0");
			break;
		case MTK7620_STD:
			resultFlag=genRalinkLanDhcpdConfigFile(platform,"br0");
			break;
		default:
			strcpy(errormsg,"wrong palt form!");
			return resultFlag;
			
	}
	return resultFlag;
}


