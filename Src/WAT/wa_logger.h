#pragma once

#ifndef _WA_LOGGER_HEADER
#define _WA_LOGGER_HEADER

#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/async.h" //support for async logging.
#include "spdlog/sinks/basic_file_sink.h"

#include "WAT.h"

#include "..\Winamp\buildType.h"

std::shared_ptr<spdlog::logger> init_log()
{
	std::string l_app_version( STR_WINAMP_FILEVER );
	wa::strings::replaceAll( l_app_version, ",", "." );

	std::string l_app_data_folder( getenv( "APPDATA" ) );
	l_app_data_folder.append( "\\Winamp\\winamp.log" );

	auto my_wa_logger = spdlog::basic_logger_mt( l_app_version.c_str(), l_app_data_folder.c_str());

	spdlog::set_default_logger( my_wa_logger );
	spdlog::set_level( spdlog::level::trace );

	return my_wa_logger;
}


static std::shared_ptr<spdlog::logger> wa_logger = init_log();


#ifdef _DEBUG
#define LOG_DEBUG(text)     spdlog::debug( wa::strings::convert::to_string( text ) );
#define LOG_INFO(text)      spdlog::info( wa::strings::convert::to_string( text ) );
#define LOG_WARN(text)      spdlog::warn( wa::strings::convert::to_string( text ) );
#define LOG_ERROR(text)     spdlog::error( wa::strings::convert::to_string( text ) );
#define LOG_CRITICAL(text)  spdlog::critical( wa::strings::convert::to_string( text ) );
#else
#define LOG_DEBUG(text)     /* nop */
#define LOG_INFO(text)      /* nop */
#define LOG_WARN(text)      /* nop */
#define LOG_ERROR(text)     spdlog::error( wa::strings::convert::to_string( text ) );
#define LOG_CRITICAL(text)  spdlog::critical( wa::strings::convert::to_string( text ) );
#endif // _DEBUG


wchar_t     _log_message_w[ DEFAULT_STR_BUFFER_SIZE ];
char        _log_message_a[ DEFAULT_STR_BUFFER_SIZE ];
std::string _log_message;

#endif // !_WA_LOGGER_HEADER