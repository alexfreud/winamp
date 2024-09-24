//PORTABLE
#ifndef _PTRLIST_H
#define _PTRLIST_H

#define POS_LAST -1

// 1k each, leaving 16 bytes for MALLOC overhead
#define PTRLIST_INCREMENT 252

template<class T>
class PtrList
{
public:
    PtrList()
    {
        nitems = 0;
        nslots = 0;
        items = NULL;
    }

    PtrList( PtrList<T> *other )
    {
        nitems = other->nitems;
        nslots = other->nslots;
        items = (T **)memdup( other->items, nslots * sizeof( T * ) );
    }

    virtual ~PtrList()
    {
        if ( items )
            free( items );
    }

    int getNumItems()                                { return nitems; };

    T *enumItem( int n )
    {
        if ( items == NULL )
            return NULL;

        if ( n < 0 )
            return NULL;

        if ( n >= nitems )
            return NULL;

        return items[ n ];
    }

    T *operator[]( int n )                           { return enumItem( n ); }

// this will safely return NULL if 0 items due to enumItems's boundscheck
    T *getFirst()                                    { return enumItem( 0 ); }

    T *getLast()                                     { return enumItem( nitems - 1 ); }

    virtual T *addItem( T *item, int pos = POS_LAST )
    {
//    ASSERTPR(item != NULL, "can't put NULLs into ptrlists");
//    ASSERT(nitems <= nslots);
        if ( items == NULL )
        {
            nslots = PTRLIST_INCREMENT;
            items = (T **)malloc( sizeof( T * ) * nslots );
            nitems = 0;
        }
        else if ( nslots == nitems )
        {	// time to expand
            T **newitems;
            nslots += PTRLIST_INCREMENT;
            newitems = (T **)malloc( sizeof( T * ) * nslots );
            memcpy( newitems, items, nitems * sizeof( T * ) );

            if ( items )
                free( items );

            items = newitems;
        }

        _addToList( item );
        if ( pos != POS_LAST )
            moveItem( nitems - 1, pos );

        return item;
    }

    // FG> avoid using this before i tested it more
    void moveItem( int from, int to )
    {
        if ( from == to )
            return;

        T *ptr = items[ from ];

        if ( nitems > from + 1 ) // if moving from the end, there is nothing to shift behind our src position
          //IMPORTANT note for future ports: This assumes MEMCPY accepts overlapping segments.
            memcpy( &items[ from ], &items[ from + 1 ], ( nitems - from ) * sizeof( T * ) );

        if ( to > from )
            to--;

        if ( nitems > to ) // if moving to the end, there is nothing to shift behind our target position
            memcpy( &items[ to + 1 ], &items[ to ], ( nitems - to - 1 ) * sizeof( T * ) );

        items[ to ] = ptr;
    }

    // deletes first instance of a pointer in list, returns how many are left
    int delItem( T *item )
    {
        int c = 0;

        if ( item == NULL || items == NULL || nitems <= 0 )
            return 0;

        // count occurences of item in items, remember the first one
        T **p = items;
        int first = -1;

        for ( int i = 0; i < nitems; i++, p++ )
        {
            if ( *p == item )
            {
                if ( c++ == 0 )
                    first = i;
            }
        }

        // if we found one, remove it
        if ( c > 0 )
        {
            delByPos( first ); // delByPos is faaast
            c--; // decrease count
        }

        return c; // returns how many occurences of this item left
    }

    // removes all instances of this pointer
    void delEveryItem( T *item ) { while ( delItem( item ) ); }

    // removes pointer at specified pos
    void delByPos( int pos )
    {
        if ( pos < 0 || pos >= nitems )
            return;  //JC

        --nitems;
        if ( pos == nitems )
            return;	// last one? nothing to copy over

        memcpy( items + pos, items + ( pos + 1 ), sizeof( T * ) * ( nitems - pos ) );  // CT:not (nitems-(pos+1)) as nitems has been decremented earlier
    }

    // removes last item, leaving pointer alone
    void removeLastItem()
    {
        if ( nitems == 0 || items == NULL )
            return;

        nitems--;	// hee hee
    }

    // removes all entries, leaving pointers alone
    void removeAll()
    {
        if ( items )
            free( items );

        items = NULL;
        nitems = 0;
        nslots = 0;
    }

    // removes all entries, calling FREE on the pointers
    void freeAll()
    {
        for ( int i = 0; i < nitems; i++ )  //JC
            if ( items )
                free( items[ i ] );

        removeAll();
    }

    // removes all entries, calling delete on the pointers
    void deleteAll()
    {
        int i;
        if ( items == NULL || nitems <= 0 )
            return;  //JC

        for ( i = 0; i < nitems; i++ )
        {  //JC
            delete items[ i ];
        }

        removeAll();
    }

    virtual int haveItem( T *item )
    {// gross-ass linear search to see if we have it
        for ( int i = 0; i < nitems; i++ )  //JC
            if ( items[ i ] == item )
                return 1;

        return 0;
    }

    virtual int searchItem( T *item )
    { // gross-ass linear search to find index of item
        for ( int i = 0; i < getNumItems(); i++ )
            if ( items[ i ] == item )
                return i;

        return -1;
    }

protected:
    virtual void _addToList( T *item )               { items[ nitems++ ] = item; }

    int nitems, nslots;
    T **items;
};

template<class T>
class PtrListBaseSorted : public PtrList<T>
{
public:
    virtual T *findItem( char *attrib )
    {
#if 1
        if ( nitems == 0 || items == NULL )
            return NULL;

        int bot = 0, top = nitems - 1;

        for ( int c = 0; c < nitems + 16; c++ )
        {
            if ( bot > top )
                return NULL;

            int mid = ( bot + top ) / 2;
            int r = compareAttrib( attrib, items[ mid ] );

            if ( r == 0 )
                return items[ mid ];

            if ( r < 0 )
            {
                top = mid - 1;
            }
            else
            {
                bot = mid + 1;
            }
        }
    //    ASSERTPR(0, "binary search fucked up");
#else
        for ( int i = 0; i < nitems; i++ )
        {
            if ( compareAttrib( attrib, items[ i ] ) == 0 )
                return items[ i ];
        }
#endif
        return NULL;
    }

protected:
    // comparator for searching -- override
    virtual int compareAttrib( char *attrib, T *item ) = 0;

    // comparator for sorting -- override	, -1 p1 < p2, 0 eq, 1 p1 > p2
    virtual int compareItem( T *p1, T *p2 ) = 0;
};


#if 0
// try not to use this
template<class T>
class PtrListSortedInsertion : public PtrListBaseSorted<T>
{
protected:
    virtual void _addToList( T *item )
    {
        if ( nitems == 0 )
        {
            items[ nitems++ ] = item;

            return;
        }

        for ( int i = 0; i < nitems; i++ )
        {
            if ( compareItem( items[ i ], item ) == 1 )
            {
                T *tmp = items[ i ];
                items[ i ] = item;
                for ( int j = i + 1; j < nitems; j++ )
                {
                    T *tmp2 = items[ j ];
                    items[ j ] = tmp;
                    tmp = tmp2;
                }

                items[ nitems++ ] = tmp;

                return;
            }
        }

        items[ nitems++ ] = item;

        return;
    }
};
#endif

// a base class to defer sorting until lookup
template<class T>
class PtrListSorted : public PtrListBaseSorted<T>
{
public:
    PtrListSorted()
    {
        nitems = 0;
        nslots = 0;
        items = NULL;
        need_sorting = 0;
    }

    virtual T *addItem( T *item )
    {
        need_sorting = 1;

        return PtrList<T>::addItem( item );
    }

    virtual T *findItem( char *attrib )
    {
        sort();

        return PtrListBaseSorted<T>::findItem( attrib );
    }

    int needSort()                                   { return need_sorting; }

    void sort()
    {
        if ( need_sorting )
            _sort();

        need_sorting = 0;
    }

private:
    int need_sorting;

    virtual void _sort() = 0;
};

template<class T>
class PtrListQuickSorted : public PtrListSorted<T>
{
public:
    virtual void _sort()
    {
        if ( items == NULL || nitems <= 1 )
            return;

        Qsort( 0, nitems - 1 );
    }

    void swap( int a, int b )
    {
        T *tmp = items[ a ];
        items[ a ] = items[ b ];
        items[ b ] = tmp;
    }

    void Qsort( int lo0, int hi0 )
    {
        int lo = lo0, hi = hi0;
        if ( hi0 > lo0 )
        {
            T *mid = enumItem( ( lo0 + hi0 ) / 2 );
            while ( lo <= hi )
            {
                while ( ( lo < hi0 ) && ( compareItem( items[ lo ], mid ) < 0 ) )
                    lo++;

                while ( ( hi > lo0 ) && ( compareItem( items[ hi ], mid ) > 0 ) )
                    hi--;

                if ( lo <= hi )
                {
                    swap( lo, hi );
                    lo++;
                    hi--;
                }
            }

            if ( lo0 < hi )
                Qsort( lo0, hi );

            if ( lo < hi0 )
                Qsort( lo, hi0 );
        }
    }
};

#endif
