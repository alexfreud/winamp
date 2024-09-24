#pragma once
#ifndef base64_H_
#define base64_H_

#include "unicode/uniString.h"

// no decoding... yet
namespace base64
{
	template<typename OUTT, typename ITER>
	OUTT encode(ITER start, ITER finish) throw()
	{
		static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		int shift = 0;
		int accum = 0;

		ITER i = start;
		OUTT result;
	
		while (i != finish)
		{
			accum <<= 8;
			shift += 8;
			accum |= *(i++);
			while (shift >= 6)
			{
				shift -= 6;
				result.push_back((typename OUTT::value_type)alphabet[(accum >> shift) & 0x3F]);
			}
		}
		if (shift == 4)
		{
			result.push_back((typename OUTT::value_type)alphabet[(accum & 0xF)<<2]);
			result.push_back((typename OUTT::value_type)'=');  
		}
		else if (shift == 2)
		{
			result.push_back((typename OUTT::value_type)alphabet[(accum & 0x3)<<4]);
			result.push_back((typename OUTT::value_type)'=');  
			result.push_back((typename OUTT::value_type)'=');  
		}
		return result;
	}
	
	template<typename OUTT,typename CONT>
	OUTT encode(const CONT &c) throw() 
	{ 
		return base64::encode<OUTT>(c.begin(), c.end()); 
	}

	const std::vector<__uint8> decode(std::string const& encoded_string);
}

#endif
