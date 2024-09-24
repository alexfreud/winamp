#pragma once
#ifndef _AUTH_H
#define _AUTH_H

#include <vector>
#include "unicode/uniString.h"
#include "uvox2Common.h"


class protocol_shoutcastClient;

#define DNAS_AUTH_URL "https://auth.shoutcast.com/AddShout"

namespace auth
{
	using namespace uniString;
	class authService;

	typedef size_t streamID_t;
	struct auth_info
	{
		protocol_shoutcastClient    *client;
		utf8                        url;
		utf8                        post;
		std::vector<__uint8>        content;
		streamID_t					sid;
		int                         group;
        int                         m_dataType;
		bool                        authenticated;
		bool                        has_intro;
		bool						valid_response;
		bool						delayed_auth;

		explicit auth_info(const utf8& _url = "")
		{
			url = _url;
			client = NULL;
			group = -1;
			authenticated = false;
			has_intro = false;
			valid_response = false;
			delayed_auth = false;
			sid = 1;
            m_dataType = MP3_DATA; // default
		}

		~auth_info()
		{
			client = NULL;
			post.clear();
			group = -1;
			authenticated = false;
			has_intro = false;
			valid_response = false;
			delayed_auth = false;
			content.clear();
			sid = 0;
		}
	};

    extern utf8                    g_authURL;

	void schedule(auth_info *info);
    void updateServices (bool initial = false);
	void init();
	void cleanup();
}

#endif
