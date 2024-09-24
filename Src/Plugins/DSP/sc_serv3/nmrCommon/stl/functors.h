#pragma once
#ifndef functors_H_
#define functors_H_

namespace stlx
{
	template<typename T>
	void delete_fntr(T *t)
	{
		if (t)
		{
			try
			{
				delete t;
				t = 0;
			}
			catch(...)
			{
			}
		}
	}
}

#endif
