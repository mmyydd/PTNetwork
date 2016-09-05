#include "common.h"
#include "file.h"

qboolean readfile(const char *filename, unsigned char **pbuf, uint32_t *len)
{
	FILE *fp;
	unsigned char *buf;
	uint32_t length;
	fp = fopen(filename, "rb");
	if(fp == NULL) return false;

	fseek(fp,0,SEEK_END);
	length = (uint32_t)ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	buf = MEM_MALLOC(length);
	if(fread(buf, 1, length, fp) != length)
	{
		MEM_FREE(buf);
		fclose(fp);
		return false;
	}

	*len = length;
	*pbuf = buf;
	fclose(fp);
	
	return true;
}
