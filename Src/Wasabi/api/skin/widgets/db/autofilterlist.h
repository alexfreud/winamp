#ifndef __AUTOFILTERLIST_H
#define __AUTOFILTERLIST_H

#include "../std.h"
#include "listwnd.h"
#include "../db/scanner.h"
#include "../timeslicer.h"
#include "../db/subqueryserver.h"
#include "../db/multiqueryclient.h"
#include "autoquerylist.h"

/**
  A filter list item is a data item of any type or 
  size to put on a FilterList. The type and size are accessible 
  for memory management purposes.
  
  @short Items for FilterLists
  @author Nullsoft
  @ver 1.0
  @see FilterListItemSort, MultiQueryServer
*/
class FilterListItem {
  public:
    
    /**
      Allocates memory for copying a data 
	    object, and then copies the object into newly allocated memory. 
    
      @see MemBlock
      @see VoidMemblock
      @param _data      Pointer to a data object
      @param _datalen   Length of data object
      @param _datatype  Type of data object
    */
    FilterListItem(void *_data, int _datalen, int _datatype);
    
    /**
      Does nothing.
    */
    virtual ~FilterListItem() {  }

    /**
      Get the data associated with the filter
      list item.
    
      @see getDatatype()
      @see getDataLen()
      @ret A pointer to the data.
    */
    const char *getData() { return data.getMemory(); }
    
    /**
      Get the type of the data associated with the
      filter list item.
    
      @see getData()
      @see getDataLen()
      @see metatags.h
      @ret Data type of item.
    */
    int getDatatype() { return data_type; }

    /**
      Get the length of the data (in bytes).

      @see getDatatype()
      @see getData()
      @ret Length of data (in bytes).
    */
    int getDataLen() { return data_len; }

  private:
  
    MemBlock<char> data;
    int data_type;
    int data_len;
};

/**
  Provides methods for sorting a FilterList.
  
  @short FilterList sorting.
  @author Nullsoft
  @ver 1.0
  @see FilterListItem
*/
class FilterListItemSort {
  public:
    /**
      Determines the data types of two objects and 
      calls an appropriate comparison function to compare them.
    
      @assert The two objects to be compared have the same data type.
      @see compareAttrib()
      @ret -1, _p1 < _p2; 0, _p1 = _p2; 1, _p1 > _p2
      @param _p1 Object to compare
      @param _p2 Object to compare
    */
    static int compareItem(void *_p1, void *_p2) {
      FilterListItem *p1 = ((FilterListItem *)_p1);
      FilterListItem *p2 = ((FilterListItem *)_p2);
      ASSERT(p1->getDatatype() == p2->getDatatype());
      switch (p1->getDatatype()) {
        case MDT_INT: 
        case MDT_TIME:
        case MDT_BOOLEAN:
        case MDT_TIMESTAMP: {
          int a = *(int *)p1->getData();
          int b = *(int *)p2->getData();
          if (a < b) return -1;
          if (a > b) return 1;
          return 0; }
        case MDT_STRINGZ:
          return STRICMP(p1->getData(), p2->getData());
      }
      return 0;
    }
    
    /**
      Compares the value of an item to an attribute string, 
      or to the value of the attribute string cast to an 
      appropriate data type.
    
      @see compareItem()
      @ret -1, _p1 < _p2; 0, _p1 = _p2; 1, _p1 > _p2
      @param attrib
      @param _item
    */
    static int compareAttrib(const wchar_t *attrib, void *_item) {
      FilterListItem *item = ((FilterListItem *)_item);
      switch (item->getDatatype()) {
        case MDT_INT: 
        case MDT_TIME:
        case MDT_BOOLEAN:
        case MDT_TIMESTAMP: {
          int a = *(int *)attrib;
          int b = *(int *)item->getData();
          if (a < b) return -1;
          if (a > b) return 1;
          return 0; }
        case MDT_STRINGZ:
          return STRICMP(attrib, item->getData());
      }
      return 0;
    }
};

class ButtHooker;
class FilenameI;

#define AUTOFILTERLIST_PARENT ListWnd
#define AUTOFILTERLIST_DBPARENTSRV SubQueryServerI
#define AUTOFILTERLIST_DBPARENTCLIENT MultiQueryClientI

/**
  A list of items to filter on.
  
  @short A list of items to filter on.
  @author Nullsoft
  @ver 1.0
  @see FilterListItemSort
  @see FilterListItem
*/
class AutoFilterList : public AUTOFILTERLIST_PARENT, 
                       public AUTOFILTERLIST_DBPARENTSRV, 
                       public AUTOFILTERLIST_DBPARENTCLIENT {
  public:

    /**
      Creates an empty filter list with a NULL local scanner.
    
      @see ~AutoFilterList()
    */
    AutoFilterList();
    
    /**
      Deletes the local scanner.

      @see AutoFilterList()
    */
    virtual ~AutoFilterList();

    /**
      Sets the field name and sets the scanner to NULL.
    
      @param field The name of the field.
    */
    void setMetadataField(const char *field);

    /**
      Creates and populates the list.
      
      @ret 1
    */
    virtual int onInit();
    
    /**
      Prepares a window for repainting in a new size.

      @ret 1
    */
    virtual int onResize();

    virtual void getClientRect(RECT *);
    virtual void rootwndholder_getRect(RECT *r);
    virtual void onNewContent();
    
    /**
      Displays a window in a specified position and state.

      @ret 1
      @param canvas
      @param pos
      @param r
      @param lParam
      @param isselected
      @param isfocused
    */
    virtual int ownerDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int isselected, int isfocused);

    /**
      Event is triggered when the user clicks (with the left mouse
      button) on an item in the auto filter list.
      
      @param itemnum The number of the item that was clicked.
    */
    virtual void onLeftClick(int itemnum);
    
    /**
      Event is triggered when the user double-clicks (with the left mouse
      button) on an item in the auto filter list.
      
      @param itemnum The number of the item that was double-clicked.
    */
    virtual void onDoubleClick(int itemnum);	// double-click on an item

    /**
      Gets the dependency pointer.
    
      @ret The dependency pointer.
    */
    virtual api_dependent *timeslicer_getDependencyPtr() { return rootwnd_getDependencyPtr(); }

    /**
      Returns the proper query string for "all items" 
      present in the list.
      
      @ret Query string for "all items".
    */
    virtual const char *getAllString() { return "All"; }


    /**
      Deletes the current scanner and sets up a new one.
      
      @param A shared database scanner
    */
    virtual void scannerserver_onNewScanner(SharedDbScannerI *scanner);

    /**
      Makes the necessary preparations for running a new query.
      
      @see SetQuery
      @see MultiQueryServer, SubQueryServer
      @param modifier refers to a SubQueryServer, which implements a simple filter.
      @param flag The type of query that will be performed.
    */
    virtual void mqc_onNewMultiQuery(SubQueryServer *modifier, int flag);
    
    /**
      Gets the dependency pointer.
      
      @see api_dependent
      @ret Dependency Dependency pointer
    */
    virtual api_dependent *mqc_getDependencyPtr() { return rootwnd_getDependencyPtr(); }

    /**
      Sets the order of the result list.
      <b>Not currently implemented!</b>
      
      @param order order desired for the result list.
    */
    void setOrder(int n) { order = n; }

    /**
      Gets the order number.
      
      @see setOrder()
      @ret Order number if linked, otherwise -1
      @param None
    */
    virtual int sqs_getCooperativeId() { return linked ? order : -1; }

    /**
      Event is triggered when a multi query has completed.
      Has no external effect.
    */
    virtual void mqc_onCompleteMultiQuery();

    /**
      Tests whether to enter a playstring in a filter entry, and does it if
	    necessary.
      
      @param playstring The playstring being added.
      @param nitems HELP
      @param thispos HELP
    */
    virtual void mqc_onAddPlaystring(const char *playstring, int nitems, int thispos);


    /**
      Registers a client with a query server.
      
      @see sqs_onDetachServer()
      @param s The server we will register to.
    */
    virtual void sqs_onAttachServer(MultiQueryServer *s);

    /**
      Unregisters a client with a query server.
      
      @see sqs_onAttachServer()
      @param s The server we will unregister from.
    */
    virtual void sqs_onDetachServer(MultiQueryServer *s);
    
    /**
      Resets the query.
    */
    virtual void sqs_reset();
    
    /**
      Generic deferred callback interface.
    */
    virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

    /**
      Sets the linking status according to the value v.
      
      @param v linking status to set
    */
    virtual void setLinked(int v) { linked = v; }

    /**
      Set the direction of the query.
      
      @see SetQuery
      @param glag The direction of the query.
    */
    void setQueryDirection(int glag);

    /**
      HELP
    */
    void doFieldPopup();
    
    /**
      HELP
    */
    virtual void onVScrollToggle(BOOL set);
    
    /**
      HELP
    */
    virtual int wantRenderBaseTexture() { return 1; }

    /**
      Event is triggered when an item is 
      being dragged.
      
      @param iItem The item being dragged.
      @ret 0, Drag not handled; 1, Drag handled;
    */
    virtual int onBeginDrag(int iItem);
    
    /**
      Gets called when the API thinks the drag and drop
      was successful.
      
      @param success Drag and Drop succeeded? (According to API).
      @ret 0, Drag and Drop not sucessful; 1, Drop and Drop completed successfully;
    */
    virtual int dragComplete(int success);

  private:
    /**
      HELP
    */
    void populate();
    
    /**
      Finds a specific entry in the database.
      
      @param playstring The playstring for the item.
      @param scanner    The db scanner to use.
      @param field      The field to be read.
    */
    void filterEntry(const char *playstring, dbScanner *scanner, const char *field);
    
    /**
      Filters and formats integer type data
      and inserts it into the list to be displayed.
    */
    void filterInt(int data);
    
    /**
      Filters and formats string type data
      and inserts it into the list to be displayed.
    */
    void filterString(const char *data);
    
    /**
      Inserts data into the list so that it may be
      displayed.
      
      @see metatags.h
      @param data The data to insert.
      @param len  The length of the data to insert.
      @param type The type of the data to insert.
    */
    void insertData(void *data, int len, int type);
    
    /**
      Sets the filter label to the meta data field we are
      currently sorting on.
      
      @see filterEntry()
    */
    void setLabelName();

    String field;
    dbScanner *local_scanner;
    PtrListInsertSorted<FilterListItem, FilterListItemSort> uniques;
    int data_type;
    int order;
    int grab_playstrings;
    int viscount;
    int linked;
    int needrestart;
    int querydirection;
    int last_populate_flag;
    ButtHooker *hooker;
    FilenameI *fn;
};

#endif
