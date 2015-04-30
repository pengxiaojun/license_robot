#include "licgen.h"

uint64_t hash(unsigned char* Astr)
{
    uint64_t hash = 5381;
    int c;

    while ((c = *Astr++))
        hash = ((hash << 5) + hash) + c; 
    return hash;
}

#if defined _WIN32

#include <winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MAGIC	"CA268A5829BB427EBBEC10F397298DB8"

#define snprintf	_snprintf

typedef struct{
	char name[48];		//adapter name
	char mac[16];		//adapter mac address
}win_mac_t;

int enum_nics(win_mac_t *Anic, int Anum)
{
	/* Declare and initialize variables */
    DWORD dwRetVal = 0;

    unsigned int i = 0;
	int count = 0;
	int len = 0;
	char bit[3] = {0};
	char *name;
	char *pos;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    // default to unspecified address family (both)
    ULONG family = AF_INET;
    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;  

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
        if (pAddresses == NULL) {
            return -1;
        }

        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses && count < Anum) {
            if (pCurrAddresses->IfType != IF_TYPE_SOFTWARE_LOOPBACK &&
				pCurrAddresses->PhysicalAddressLength != 0) 
			{
				pos = Anic[count].mac;
				//get adapter name
				strcpy(Anic[count].name, MAGIC);	//avoid adapter name is empty
				name = pCurrAddresses->AdapterName;
				len = 0;
				for (i = 0; i<strlen(name); ++i)
				{
					if (isalnum(name[i]))
						Anic[count].name[len++] = name[i];
				}

                for (i = 0; i < (int) pCurrAddresses->PhysicalAddressLength; i++) 
				{
					sprintf(bit, "%.2X", (int) pCurrAddresses->PhysicalAddress[i]);
					strcpy(pos, bit);
					pos += 2;
                }
				count++;
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    } else {
        printf("Call to GetAdaptersAddresses failed with error: %d\n", dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            printf("\tNo addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   
                    // Default language
                    (LPTSTR) & lpMsgBuf, 0, NULL)) {
                printf("\tError: %s", lpMsgBuf);
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    FREE(pAddresses);
                return -1;
            }
        }
    }

    if (pAddresses) {
        FREE(pAddresses);
    }
	return count;
}
void get_mac_sn(unsigned char Asn[17])
{
	unsigned char *sn[] = {Asn};
	get_all_mac_sn(sn, 1);
}

int get_all_mac_sn(unsigned char **Asn, int Anum)
{
	int max = 8;
	win_mac_t wmts[8] = {0};
    unsigned char mb[64];
    int i, j, num;

    num = enum_nics(wmts, max);

    for (i = 0; i<Anum && i<num; ++i)
    {
        memset(mb, 0, sizeof(mb));
        for (j = 0; j<16; ++j)
		{
			if (j % 2 == 0 && j < 12)
			{
				Asn[i][j] = wmts[i].mac[j];
			}
			else
			{
				Asn[i][j] = wmts[i].name[j];
			}
		}
    }
    return Anum;
}

#else /* linux */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <ifaddrs.h>

int enum_nics(char *Anic[], int Anum)
{
	struct ifaddrs *ifp, *ibuf = NULL;
	if (getifaddrs (&ibuf) < 0 || ibuf == NULL)
		return 0;
    int num = 0;
	for (ifp = ibuf; ifp; ifp = ifp->ifa_next) 
    {
		if (ifp->ifa_addr && 
           (AF_PACKET == ifp->ifa_addr->sa_family) &&
           strcmp(ifp->ifa_name, "lo") != 0 &&
           num < Anum)
        {
            strcpy(Anic[num++], ifp->ifa_name);
		}
	}
    freeifaddrs(ibuf);
	return num;
}

int get_nic_mac(char Amac[13], const char* Anic)
{
    struct ifreq ifreq;
    int sfd;
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    strcpy(ifreq.ifr_name, Anic);
    if (ioctl(sfd, SIOCGIFHWADDR, &ifreq) < 0)
    {
		close(sfd);
        return -1;
    }
    snprintf(Amac, 13, "%X%X%X%X%X%X", 
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
	close(sfd);
    return 0;
}

#define BOARD_VENDOR    "/sys/class/dmi/id/board_vendor"
#define BOARD_NAME      "/sys/class/dmi/id/board_name"
#define BOARD_VERSION   "/sys/class/dmi/id/board_version" 

void get_board_info(char Ainfo[96])
{
    char *file[64] = {BOARD_VENDOR, BOARD_NAME, BOARD_VERSION};
    char buf[32] = {0};
    char *str = NULL;
    int i = 0;
    int len = 0;

    for (; i<3 && len < 96; ++i)
    {
        FILE *fp = fopen(file[i], "r");
        if (!fp) continue;
        str = fgets(buf, 32, fp);
        if (str != NULL)
        {
            if (str[strlen(str) - 1] == '\n')
                str[strlen(str) - 1] = '\0';
            strncpy(Ainfo, buf, strlen(str));
            Ainfo += strlen(str);
            len += strlen(str);
        }
        fclose(fp);
    }
}

void get_mac_sn(unsigned char Asn[17])
{
    char nic[16] = {0};
    char mac[13] = {0};
    char board[96] = {0};
    unsigned char mb[128] = {0};
    char *nics[1] = {nic};
    uint64_t hs;

    enum_nics(nics, 1);
    get_nic_mac(mac, nic);
    get_board_info(board);

    strncpy((char*)mb, board, strlen(board));
    strncpy((char*)(mb+strlen(board)), mac, strlen(mac));
    hs = hash(mb);
    snprintf((char*)Asn, 17, "%016lX", hs);
}

int get_all_mac_sn(unsigned char **Asn, int Anum)
{
    int max = 8;
    char *pnic = (char*)calloc(max, sizeof(unsigned char)*16);
    char *pmac = (char*)calloc(max, sizeof(unsigned char)*13);
    char *nics[8];
    char *macs[8];
    char *pos1 = pnic;
    char *pos2 = pmac;

    char board[96] = {0};
    unsigned char mb[128];
    uint64_t hs;
    int i;

    for (i = 0; i<max; ++i)
    {
        nics[i] = pos1;
        macs[i] = pos2;
        pos1 += sizeof(unsigned char)*16;
        pos2 += sizeof(unsigned char)*13;
    }

    get_board_info(board);
    Anum = enum_nics(nics, Anum);

    for (i = 0; i<Anum; ++i)
    {
        get_nic_mac(macs[i], nics[i]);
        memset(mb, 0, sizeof(mb));
        strncpy((char*)mb, board, strlen(board));
        strncpy((char*)(mb+strlen(board)), macs[i], strlen(macs[i]));
        hs = hash(mb);
        snprintf((char*)Asn[i], 17, "%016lX", hs);
    }
    free(pnic);
    free(pmac);
    return Anum;
}
#endif

/*int main(int argc, const char *argv[])
{
    int max = 8;
    int i, num;
    unsigned char *p = (unsigned char*)calloc(max, sizeof(unsigned char)*17);
    unsigned char *sn[8];
    unsigned char *pos = p;
    
    for (i = 0; i<max; ++i)
    {
        sn[i] = pos;
        pos += sizeof(unsigned char)*17;
    }
    num = get_all_mac_sn(sn, max);
    for (i = 0; i<num; ++i)
    {
        printf("SN=%s len=%d\n", (char*)sn[i], (int)strlen((char*)sn[i]));
    }
    free(p);
    return 0;
}*/

