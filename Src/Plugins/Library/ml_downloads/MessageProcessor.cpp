#include "main.h"
#include "MessageProcessor.h"

#define CBCLASS MessageProcessor
START_DISPATCH;
CB(API_MESSAGEPROCESSOR_PROCESS_MESSAGE, ProcessMessage)
END_DISPATCH;