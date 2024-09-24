#include "hmac_sha256.h"
#include <bfc/platform/types.h>

class OAuthKey
{
public:
	OAuthKey(const void *key, size_t key_len);
	~OAuthKey();
	void FeedMessage(const void *data, size_t data_len);
	void EndMessage();
	void GetBase64(char *output, size_t len);
	HMAC_SHA256_CTX ctx;
	uint8_t buf[HMAC_SHA256_DIGEST_LENGTH];
};

