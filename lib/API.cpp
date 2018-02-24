#include <Arduino.h>
#include <SHA512.h>

#include "API.h"

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
		return RESPONSE_BAD_JSON;

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

char *get_request(unsigned long badge_num, char *operation, char *location, char *device, char *key, unsigned char key_len, char *rv, unsigned char rv_len, char *nonce)
	{
	struct cJSON
		*data = cJSON_CreateObject(),
		*root = cJSON_CreateObject(),
		*ran  = cJSON_CreateArray(),
		*json, *prev, *result;
	unsigned char i;
	unsigned long r;
	char sha_buf[SHA512_SZ], sha_buf_out[2 * SHA512_SZ + 1];
	char *out;

	for (i = 0; i < rv_len; i++)
		{
		json = cJSON_CreateNumber((long)rv[i]);
		if (!i)
			ran->child = json;
		else
			{
			prev->next = json;
			json->prev = prev;
			}
		prev = json;
		}
	
	cJSON_AddItemToObjectCS(data, "badge",           cJSON_CreateNumber(badge_num));
	if (location)
		cJSON_AddItemToObjectCS(data, "item",          cJSON_CreateString(location));
	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	if (operation)
		cJSON_AddItemToObjectCS(data, "operation",     cJSON_CreateString(operation));
	else
		cJSON_AddItemToObjectCS(data, "operation",     cJSON_CreateString("access"));
		
	cJSON_AddItemToObjectCS(data, "random_response", ran);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	get_hash(data, sha_buf, key, key_len);

	print_hex(sha_buf_out, sha_buf, SHA512_SZ);
	
	cJSON_AddItemToObjectCS(root, "data", data);
	cJSON_AddItemToObjectCS(root, "device",  cJSON_CreateString(device));
	cJSON_AddItemToObjectCS(root, "checksum", cJSON_CreateString(sha_buf_out));
	
	out = cJSON_Print(root);
	cJSON_Delete(root);

	return out;
	}

char *get_nonce(char *device, char *key, unsigned char key_len, char *rv, unsigned char rv_len)
	{
	struct cJSON
		*data = cJSON_CreateObject(),
		*root = cJSON_CreateObject(),
		*ran  = cJSON_CreateArray(),
		*json, *prev, *result;
	unsigned char i;
	unsigned long r;
	char sha_buf[SHA512_SZ], sha_buf_out[2 * SHA512_SZ + 1];
	char *out;

	for (i = 0; i < rv_len; i++)
		{
		json = cJSON_CreateNumber((long)rv[i]);
		if (!i)
			ran->child = json;
		else
			{
			prev->next = json;
			json->prev = prev;
			}
		prev = json;
		}

	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("get_nonce"));
	cJSON_AddItemToObjectCS(data, "random_response", ran);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	get_hash(data, sha_buf, key, key_len);

	print_hex(sha_buf_out, sha_buf, SHA512_SZ);

	cJSON_AddItemToObjectCS(root, "data", data);
	cJSON_AddItemToObjectCS(root, "device",  cJSON_CreateString(device));
	cJSON_AddItemToObjectCS(root, "checksum", cJSON_CreateString(sha_buf_out));

	out = cJSON_Print(root);
	cJSON_Delete(root);

	return out;
	}

char *log_data(struct cJSON *l_data, char *device, char *key, unsigned char key_len, char *rv, unsigned char rv_len, char *nonce)
	{
	struct cJSON
		*data = cJSON_CreateObject(),
		*root = cJSON_CreateObject(),
		*ran  = cJSON_CreateArray(),
		*json, *prev, *result;
	unsigned char i;
	unsigned long r;
	char sha_buf[SHA512_SZ], sha_buf_out[2 * SHA512_SZ + 1];
	char *out;

	for (i = 0; i < rv_len; i++)
		{
		json = cJSON_CreateNumber((long)rv[i]);
		if (!i)
			ran->child = json;
		else
			{
			prev->next = json;
			json->prev = prev;
			}
		prev = json;
		}

	cJSON_AddItemToObjectCS(data, "log_data",        l_data);
	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("log"));
	cJSON_AddItemToObjectCS(data, "random_response", ran);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(1));
	get_hash(data, sha_buf, key, key_len);

	print_hex(sha_buf_out, sha_buf, SHA512_SZ);

	cJSON_AddItemToObjectCS(root, "data", data);
	cJSON_AddItemToObjectCS(root, "device",  cJSON_CreateString(device));
	cJSON_AddItemToObjectCS(root, "checksum", cJSON_CreateString(sha_buf_out));

	out = cJSON_Print(root);
	cJSON_Delete(root);

	return out;
	}
