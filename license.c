#include "license.h"
#include "sha1.h"
#include "licgen.h"


#define INT_VAL_TYPE    0
#define STR_VAL_TYPE    1

#define MAC_FIX_LEN     16 
#define MAX_SN_LEN      22 
#define DIGEST_LEN      20

#define MAX_BUF         4096

#define LIC_PREFIX      "grgrant."
#define LIC_SUFFIX      ".lic"

static void sha_digest(unsigned char digest[DIGEST_LEN], unsigned char *data, int len)
{
	int i;
	SHA1_CTX ctx;
	SHA1Init(&ctx);

	for(i = 0; i<500; i++)
	{
		SHA1Update(&ctx, data, len);
	}
	SHA1Final(digest, &ctx);

	/*printf("SHA1=");
	  for(i = 0; i < DIGEST_LEN; i++)
	  {
	  printf("%02x", digest[i]);
	  }
	  printf("\n");*/
}

static int sha_file(const char *file, int offset, unsigned char digest[DIGEST_LEN])
{
	FILE *fp;
	int nread, total;
	int begin, end;
	int len;
	unsigned char buf[4096] = {0};
	char *data, *pos;

	fp = fopen(file, "rb");
	if (fp == NULL)
		return -1;
	begin = fseek(fp, offset, SEEK_SET);
	if (begin == -1)
	{
		fclose(fp);
		return -1;
	}
	begin = ftell(fp);
	if (begin == -1)
	{
		fclose(fp);
		return -1;
	}

	end = fseek(fp, 0, SEEK_END);
	if (end == -1)
	{
		fclose(fp);
		return -1;
	}
	end = ftell(fp);
	if (end == -1)
	{
		fclose(fp);
		return -1;
	}
	total = end - begin;
	if (total <= 0)
	{
		fclose(fp);
		return -1;
	}
	data = (char*)calloc(1, total);
	if (data == NULL)
	{
		fclose(fp);
		return -1;
	}
	fseek(fp, offset, SEEK_SET);
	pos = data;
	len = total;

	while (total > 0)
	{
		nread = fread(buf, 1, MAX_BUF, fp);
		if (nread > 0)
		{
			memcpy(pos, buf, nread);
			pos += nread;
			total -= nread;
		}
		else
		{
			break;
		}
	}
	sha_digest(digest, (unsigned char*)data, len);
	fclose(fp);
	free(data);
	return 0;
}

int grlic_load(const char* Afname, gr_license_t* Alic)
{
	int ret, nread;
	uint32_t i, valtype;
	uint64_t id, number;
	char *str;
	void *blob;
	FILE *fp;
	gr_license_entry_t *entry;
	unsigned char digest[DIGEST_LEN];

	fp = fopen(Afname, "rb");
	if (fp == NULL)
		return -2;

	//read header 
	nread = fread(&Alic->head, sizeof(Alic->head), 1, fp);

	if (nread != 1)
		goto failure;

	Alic->body.entries = (gr_license_entry_t*) calloc(Alic->head.entries, sizeof(gr_license_entry_t));
	if (Alic->body.entries == NULL)
		goto failure;

	//read body
	for (i = 0; i<Alic->head.entries; ++i)
	{
		entry = &(Alic->body.entries[i]);
		id = 0;
		nread = fread(&id, sizeof(uint64_t), 1, fp);

		if (nread != 1)
			goto failure;
		entry->id = id ;

		nread = fread(&entry->credit, sizeof(entry->credit), 1, fp);

		if (nread != 1)
			goto failure;

		valtype = entry->credit.meta.valType;
		number = 0;
		str = NULL;
		blob = NULL;

		if (valtype == 1)
		{
			nread = fread(&number, sizeof(uint64_t), 1, fp);
			if(nread != 1) goto failure;

			if (number > entry->credit.quota.upper)
				number = entry->credit.quota.upper;
			if (number < entry->credit.quota.lower)
				number = entry->credit.quota.lower;

			entry->val.numval = number;
		}
		else if (valtype == 2)
		{
			if (entry->credit.meta.len > 0)
			{
				str = (char*)calloc(entry->credit.meta.len, sizeof(char));
				nread = fread(str, entry->credit.meta.len, 1, fp);
				if (nread != 1) goto failure;
				entry->val.strval = str;
			}
		}
		else if (valtype == 3)
		{
			if (entry->credit.meta.len > 0)
			{
				blob = (char*)calloc(entry->credit.meta.len, sizeof(char));
				nread = fread(blob, entry->credit.meta.len, 1, fp);
				if (nread != 1) goto failure;
				entry->val.blobval = str;
			}
		}
	}
	fclose(fp);

	//verify sn
	ret = grlic_cmp_sn(Alic->head.sn);
	if (ret == -1)
		return -3;

	//verify digest
	memset(digest, 0, sizeof(digest));
	ret = sha_file(Afname, sizeof(digest), digest);
	if (ret == -1)
		return -1;
	if (!memcmp(digest, Alic->head.digest, DIGEST_LEN))
		return 0;
	else
		return -1; //no match
failure:
	fclose(fp);
	return -1;
}

int grlic_save(const char* Afname, const gr_license_t* Alic)
{
	FILE *fp;
	size_t nwrite;
	uint32_t i, valtype;
	uint64_t id, number;
	char *str;
	void *blob;
	unsigned char digest[DIGEST_LEN];
	gr_license_entry_t *entry;

	fp = fopen(Afname, "wb");
	if (fp == NULL)
		return -1;

	//write head
	nwrite = fwrite(&Alic->head, sizeof(Alic->head), 1, fp);

	if (nwrite!= 1)
		goto failure;

	//write body
	for (i = 0; i<Alic->head.entries; ++i)
	{
		entry = &(Alic->body.entries[i]);
		id = entry->id;
		nwrite = fwrite(&id, sizeof(uint64_t), 1, fp);

		if (nwrite != 1)
			goto failure;
		nwrite = fwrite(&entry->credit, sizeof(entry->credit), 1, fp);

		if (nwrite != 1)
			goto failure;

		valtype = entry->credit.meta.valType;
		number = entry->val.numval;
		str = entry->val.strval;
		blob = entry->val.blobval;

		if (valtype == 1)
		{
			if (number > entry->credit.quota.upper)
				number = entry->credit.quota.upper;
			if (number < entry->credit.quota.lower)
				number = entry->credit.quota.lower;
			nwrite = fwrite(&number, sizeof(uint64_t), 1, fp);
			if(nwrite != 1) goto failure;
		}
		else if (valtype == 2)
		{
			if (entry->credit.meta.len > 0)
			{
				nwrite = fwrite(str, entry->credit.meta.len, 1, fp);
				if (nwrite != 1) goto failure;
			}
		}
		else if (valtype == 3)
		{
			if (entry->credit.meta.len > 0)
			{
				nwrite = fwrite(blob, entry->credit.meta.len, 1, fp);
				if (nwrite != 1) goto failure;
			}
		}
	}
	fclose(fp);

	//write digest
	memset(digest, 0, sizeof(digest));
	sha_file(Afname, sizeof(digest), digest);
	fp = fopen(Afname, "r+b");
	if (fp == NULL)
		return -1;
	nwrite = fwrite(digest, sizeof(digest), 1, fp);
	if (nwrite != 1)
		goto failure;
	fclose(fp);
	return 0;
failure:
	fclose(fp);
	return -1;
}

int grlic_take(gr_license_t* Alic)
{
	int max = 8;
	int len = 22;
	int i, num, ret;
	unsigned char *p = (unsigned char*)calloc(max, sizeof(unsigned char)*len);
	unsigned char *sn[8];
	unsigned char *pos = p;
	char name[48];

	for (i = 0; i<max; ++i)
	{
		sn[i] = pos;
		pos += sizeof(unsigned char)*len;
	}
	num = grlic_gen_sns(sn, max);
	for (i = 0; i<num; ++i)
	{
		memset(name, 0, sizeof(name));
		sprintf(name, "%s%s%s", LIC_PREFIX, (char*)sn[i], LIC_SUFFIX);
		ret = grlic_load(name, Alic);
		if (ret == 0)
		{
			free(p);
			return ret;
		}
	}
	free(p);
	return -1;
}

int grlic_gen_sn(unsigned char Asn[17])
{
	int len;
	unsigned char mac[17] = {0};
	get_mac_sn(mac); 
	len = MAC_FIX_LEN;
	memcpy(Asn, mac, len);
	Asn[len] = '\0';
	return 0;
}

int grlic_gen_sns(unsigned char **Asn, int len)
{
	int max = 8;
	int mac = 17;
	int i, num;
	unsigned char *p = (unsigned char*)calloc(max, sizeof(unsigned char)*mac);
	unsigned char *sn[8];
	unsigned char *pos = p;

	for (i = 0; i<max; ++i)
	{
		sn[i] = pos;
		pos += sizeof(unsigned char)*mac;
	}
	num = get_all_mac_sn(sn, max);
	for (i = 0; i<num && i<len; ++i)
	{
		len = MAC_FIX_LEN;
		memcpy(Asn[i], sn[i], len);
		Asn[i][len] = '\0';
	}
	free(p);
	return num;
}

int grlic_cmp_sn(unsigned char Asn[22])
{
	int i, num;
	int max = 8;
	unsigned char *p = (unsigned char*)calloc(max, sizeof(unsigned char)*22);
	unsigned char *sn[8];
	unsigned char *pos = p;
	for (i = 0; i<max; ++i)
	{
		sn[i] = pos;
		pos += sizeof(unsigned char)*22;
	}

	num = grlic_gen_sns(sn, max);
	for (i = 0; i<num; ++i)
	{
		if (!memcmp(sn[i], Asn, sizeof(unsigned char) * 22))
			goto match;
	}
	free(p);
	return -1;
match:
	free(p);
	return 0;
}

uint64_t grlic_path_id(gr_credit_path_t* Apath)
{
	uint64_t id = Apath->module;
	id = Apath->facet > 0 ? id * 100 + Apath->facet : id;
	id = Apath->func > 0 ? id * 100 + Apath->func : id;
	id = Apath->point > 0 ? id * 100 + Apath->point: id;
	return id;
}

int grlic_free(gr_license_t* Alic)
{
	uint32_t i, valtype;
	gr_license_entry_t *entry;

	for (i = 0; i<Alic->head.entries; ++i)
	{
		entry = &(Alic->body.entries[i]);
		valtype = entry->credit.meta.valType;

		if (valtype == 2)
		{
			free(entry->val.strval);
		}
		else if (valtype == 3)
		{
			free(entry->val.blobval);
		}
	}
	free(Alic->body.entries);
	return 0;
}

static gr_license_entry_t* get_lic_entry(gr_license_t* Alic, gr_credit_path_t* Apath)
{
	uint32_t i;
	uint64_t id;
	gr_license_entry_t *entry;

	if (!Alic || !Apath)
		return NULL;

	id = grlic_path_id(Apath);

	if (Alic->head.entries > UINT8_MAX)
		return NULL;

	for (i = 0; i<Alic->head.entries; ++i)
	{
		entry = &(Alic->body.entries[i]);
		if (entry->id == id)
		{
			return entry;
		}
	}
	return NULL;
}

int grlic_get_bool(gr_license_t* Alic, gr_credit_path_t* Apath, int* Aval)
{
	return grlic_get_int(Alic, Apath, Aval);
}

int grlic_get_int(gr_license_t* Alic, gr_credit_path_t* Apath, int* Aval)
{
	gr_license_entry_t *entry = get_lic_entry(Alic, Apath);    
	if (entry == NULL)
		return -1;
	*Aval = (int)entry->val.numval;
	return 0;
}

int grlic_get_str(gr_license_t* Alic, gr_credit_path_t* Apath, char* Astr, int* Alen)
{
	gr_license_entry_t *entry;
	entry = get_lic_entry(Alic, Apath);    
	if (entry == NULL)
		return -1;
	*Alen = entry->credit.meta.len;
	if (entry->val.strval)
		strncpy(Astr, entry->val.strval, *Alen);
	return 0;
}

int grlic_get_blob(gr_license_t* Alic, gr_credit_path_t* Apath, void* Astr, int* Alen)
{
	gr_license_entry_t *entry;
	entry = get_lic_entry(Alic, Apath);    
	if (entry == NULL)
		return -1;
	*Alen = entry->credit.meta.len;
	if (entry->val.blobval)
		memcpy(Astr, entry->val.blobval, *Alen);
	return 0;
}

