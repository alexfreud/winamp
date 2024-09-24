// proxydt.h
#ifndef PROXYDT_H
#define PROXYDT_H

#include <string>
#include <stdio.h>
#include <atlbase.h>

char* detectBrowserProxy();

char* DetectIEProxy();

char* DetectNS4Proxy();
char* DetectNS6Proxy();

#endif