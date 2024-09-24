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
#ifndef _C_ITEMLIST_H_
#define _C_ITEMLIST_H_

#define MEMORY_STEP 32

class C_ItemList
{
public:
    C_ItemList();
    ~C_ItemList();

    void *Add( void *i );

    void Set( int w, void *newv );

    void *Get( int w ) const;

    void Del( int idx );

    void *Insert( void *i, int pos );

    int GetSize( void ) const                                         { return current_index; }

    void **GetAll()                                                   { return m_list; }

    void for_all( void ( *for_all_func )( void * ) );

    void for_all_ctx( void ( *for_all_func )( void *, void * ), void * );

protected:
    void **m_list = NULL;
    int current_index = 0;
    int alloc_size = 0;

};

#endif //_C_ITEMLIST_H_
