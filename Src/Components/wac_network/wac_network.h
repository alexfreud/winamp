/*
** JNetLib
** Copyright (C) 2000-2006 Nullsoft, Inc.
** Author: Justin Frankel
** File: jnetlib.h - JNL main include file (not really necessary).
**
** For documentation, look at the following files:
**  Generic network initialization: netinc.h
**  DNS: asyncdns.h
**  TCP connections: connection.h
**  HTTP GET connections: wac_network_http_receiver.h
**  TCP listen: wac_network_web_server_listen.h
**
**  license:
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
*/

#ifndef NULLSOFT_WAC_NETWORK_H
#define NULLSOFT_WAC_NETWORK_H

#include "netinc.h"
#include "util.h"

#include <time.h>

#include "wac_network_dns.h"
#include "wac_network_connection.h"
#include "wac_network_http_receiver.h"
#include "wac_network_http_server.h"
#include "wac_network_web_server_listen.h"

#endif  //!NULLSOFT_WAC_NETWORK_H
