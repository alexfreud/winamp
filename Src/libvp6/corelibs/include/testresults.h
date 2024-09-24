#if !defined(_TestResults_h)
#define _TestResults_h
#include <string.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct TestResults_T
{
	char * results;
	int testResultMaxLength;
} TestResults;
int GetTestResults( TestResults *T ,int argc, char *argv[]);
const char * GetTestHelp(void);
inline void StoreTestResult(TestResults *p, char *msg)
{
    if (p)
    {
		sprintf(&(p->results[strlen(p->results)]),"%s",msg);
	}
}
#if defined(__cplusplus)
}
#endif
#endif /* include guards */
