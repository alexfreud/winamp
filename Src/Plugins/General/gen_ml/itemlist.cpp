/*
** Copyright (C) 2003-2006 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#include <windows.h>
#include "itemlist.h"

C_ItemList::C_ItemList()
{
}

C_ItemList::~C_ItemList()
{
	if (m_list) ::free(m_list);
}

void *C_ItemList::Add(void *i)
{
	// check if we have enough space to add an element
	if (!m_list || alloc_size == 0 || current_index >= alloc_size - 1)//|| !(m_size&31))
	{
		alloc_size += MEMORY_STEP;
		void **new_m_list=(void**)::realloc(m_list,sizeof(void*)*(alloc_size));
		if (new_m_list)
			m_list = new_m_list;
		else
		{
			m_list=NULL;
			return NULL;
		}
	}
	// add the element and increase the index
	m_list[current_index]=i;
	current_index++;
	return i;
}

void C_ItemList::Set(int w, void *newv) 
{ 
	if (w >= 0 && w < current_index) 
	{ 
		m_list[w]=newv; 
	}
}

void *C_ItemList::Get(int w) const 
{
	if (w >= 0 && w < current_index)
	{
		return m_list[w]; 
	}
	return NULL; 
}

void C_ItemList::Del(int idx)
{
	if (m_list && idx >= 0 && idx < current_index)
	{
		current_index--;
		if (idx !=  current_index) 
		{ 
			::memcpy(m_list + idx, m_list + idx + 1, sizeof(void*) * (current_index - idx));
		}
		if (!(current_index &31)&& current_index) // resize down
		{
			void** new_m_list=(void**)::realloc(m_list,sizeof(void*)* current_index);
			if (new_m_list) m_list = new_m_list;
			else
			{
				new_m_list=(void**)::malloc(sizeof(void*)* current_index);
				if (new_m_list)
				{
					memcpy(new_m_list, m_list, sizeof(void*)* current_index);
					free(m_list);
					m_list = new_m_list;
				}
			}
		}
	}
}

void *C_ItemList::Insert(void *i, int pos) 
{
	if (!m_list || !(current_index &31))
	{
		void** new_m_list=(void**)::realloc(m_list,sizeof(void*)*(current_index +32));
		if (new_m_list)
			m_list = new_m_list;
		else
		{
			new_m_list=(void**)::malloc(sizeof(void*)*(current_index +32));
			if (new_m_list)
			{
				memcpy(new_m_list, m_list, sizeof(void*)* current_index);
				free(m_list);
				m_list = new_m_list;
			}
			else
				return i;
		}

	}
	current_index++;

	for ( int j = current_index - 1; j > pos; j-- )
		m_list[ j ] = m_list[ j - 1 ];

	m_list[ pos ] = i;

	return i;
}

void C_ItemList::for_all(void (*for_all_func)(void *))
{
	if (m_list)
	{
		for (int i=0;i< current_index;i++)
		{
			for_all_func(m_list[i]);
		}
	}
}

void C_ItemList::for_all_ctx(void (*for_all_func)(void *, void *), void *ctx)
{
	if (m_list)
	{
		for (int i=0;i< current_index;i++)
		{
			for_all_func(m_list[i], ctx);
		}
	}
}