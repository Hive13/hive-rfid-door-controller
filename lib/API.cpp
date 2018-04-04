#include "config.h"

#include <Arduino.h>
#include <SHA512.h>

#include "API.h"
#include "http.h"

#include "cJSON.h"

static char *hex = "0123456789ABCDEF";

unsigned char val(char *i)
	{
	unsigned char ret = 0;
	char c;

	c = (*i++) | 0x20;
	if (c >= '0' && c <= '9')
		ret = ((c - '0') & 0x0F) << 4;
	else if (c >= 'a' && c <= 'f')
		ret = ((c - 'a' + 10) & 0x0F) << 4;

	c = (*i) | 0x20;
	if (c >= '0' && c <= '9')
		ret |= ((c - '0') & 0x0F);
	else if (c >= 'a' && c <= 'f')
		ret |= ((c - 'a' + 10) & 0x0F);

	return ret;
	}

void print_hex(char *str, char *src, unsigned char sz)
	{
	unsigned char i;

	for (i = 0; i < sz; i++)
		{
		*(str++) = hex[((src[i] & 0xF0) >> 4)];
		*(str++) = hex[(src[i] & 0x0F)];
		}
	*str = 0;
	}

unsigned char parse_response(char *in, struct cJSON **out, char *key, unsigned char key_len, char *rv, unsigned char rv_len)
	{
	unsigned char i, j;
	struct cJSON *result, *data, *response, *cs;
	char *cksum, provided_cksum[SHA512_SZ];
	char code;

	result = cJSON_Parse(in);

	if (!result)
		return RESPONSE_BAD_JSON;
	
	cs = cJSON_GetObjectItem(result, "checksum");
	if (!cs)
		{
		cJSON_Delete(result);
		return RESPONSE_BAD_JSON;
		}

	cksum = cs->valuestring;
	data  = cJSON_DetachItemFromObject(result, "data");

	if (!data)
		{
		cJSON_Delete(result);
		return RESPONSE_BAD_JSON;
		}

	get_hash(data, provided_cksum, key, key_len);

	for (i = 0; i < SHA512_SZ; i++)
		{
		j = val(cksum + (2 * i));
		code = (provided_cksum[i] - j);
		if (code)
			break;
		}

	cJSON_Delete(result);

	if (code)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_CKSUM;
		}
	
	response = cJSON_DetachItemFromObject(data, "nonce_valid");
	if (!response)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_NONCE;
		}
	if (response->type != cJSON_True)
		{
		cJSON_Delete(data);
		cJSON_Delete(response);
		return RESPONSE_BAD_NONCE;
		}
	cJSON_Delete(response);
	
	response = cJSON_DetachItemFromObject(data, "response");
	if (!response)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_JSON;
		}

	if (response->type != cJSON_True)
		{
		cJSON_Delete(data);
		cJSON_Delete(response);
		return RESPONSE_BAD_JSON;
		}
	
	cJSON_Delete(response);

	response = cJSON_DetachItemFromObject(data, "random_response");
	if (!response)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_CKSUM;
		}
	
	cs = response->child;
	
	for (i = 0; i < rv_len && cs; i++)
		{
		if (cs->type != cJSON_Number)
			break;
		if (cs->valueint != rv[i])
			break;

		cs = cs->next;
		}
	
	cJSON_Delete(response);
	
	if (i < rv_len)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_CKSUM;
		}

	if (out)
		*out = data;
	else
		cJSON_Delete(data);

	return RESPONSE_GOOD;
	}

void get_hash(struct cJSON *data, char *sha_buf, char *key, unsigned char key_len)
	{
	unsigned char i;
	unsigned long r;
	SHA512 sha;
	char *out;
	
	out = cJSON_Print(data);
	cJSON_Minify(out);

	sha.reset();
	sha.update(key, key_len);
	sha.update(out, strlen(out));
	sha.finalize(sha_buf, sha.hashSize());
	
	free(out);
	}
