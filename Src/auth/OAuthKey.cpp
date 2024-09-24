#include "OAuthKey.h"
#include "b64.h"

OAuthKey::OAuthKey(const void *key, size_t key_len)
{
	HMAC_SHA256_Init(&ctx);
	HMAC_SHA256_UpdateKey(&ctx, (unsigned char *)key, (int)key_len);
	HMAC_SHA256_EndKey(&ctx);
	HMAC_SHA256_StartMessage(&ctx);
}

OAuthKey::~OAuthKey()
{
	HMAC_SHA256_Done(&ctx);
}

void OAuthKey::FeedMessage(const void *data, size_t data_len)
{
	HMAC_SHA256_UpdateMessage(&ctx, (unsigned char *)data, (unsigned int)data_len);
}

void OAuthKey::EndMessage()
{
	HMAC_SHA256_EndMessage(buf, &ctx);
}

void OAuthKey::GetBase64(char *output, size_t cch)
{
	size_t len = b64::b64_encode(buf, HMAC_SHA256_DIGEST_LENGTH, output, cch);
	output[len] = '\0';
}