#ifndef __CHK_NIC_H__
#define __CHK_NIC_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uint64_t hash(unsigned char* Astr);
int enum_nics(char *Anics[], int Anum);
void get_mac_sn(unsigned char Asn[17]);
int get_all_mac_sn(unsigned char **Asn, int Anum);

#endif /* __CHK_NIC_H__ */

