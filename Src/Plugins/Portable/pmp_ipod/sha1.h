typedef struct {
	unsigned long state[5];
	unsigned long count[2];
	unsigned char buffer[64];
} SHA1_CTX;

extern "C" void SHA1Init(SHA1_CTX* context);
extern "C" void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
extern "C" void SHA1Final(unsigned char digest[20], SHA1_CTX* context);
