#include "main.h"

TITLELISTTYPE TitleListTerminator;
TITLELISTTYPE *TitleLinkedList = &TitleListTerminator;

void initTitleList(void)
{
  TitleListTerminator.Next = NULL; 
  TitleListTerminator.timer = 0;
}


/* -----------------------------------------------------------------------------------------------
 Adds an entry in the list
 -----------------------------------------------------------------------------------------------*/
TITLELISTTYPE *newTitleListEntry(void)
{
TITLELISTTYPE *TitleObject = (TITLELISTTYPE *)calloc(1,sizeof(TITLELISTTYPE)); /* Allocate new entry */
TitleObject->Next = (void *)TitleLinkedList; /* New entry's next is old list _head */
TitleLinkedList = TitleObject; /* new _head is new entry */
return TitleObject; /* return pointer to new entry */
}

/* -----------------------------------------------------------------------------------------------
 Removes an entry from the list
 -----------------------------------------------------------------------------------------------*/
void removeTitleListEntry(TITLELISTTYPE *Entry)
{
TITLELISTTYPE *TitleObject = TitleLinkedList;

	if (TitleObject == &TitleListTerminator) return; /* List is empty */

	if (TitleObject == (void *)Entry)
    {
		TitleLinkedList = (TITLELISTTYPE *)TitleObject->Next;
		free(TitleObject);
    }
	else
		while (TitleObject->Next) /* While not terminator */
        {
        	if ((TITLELISTTYPE *)(TitleObject->Next) == (void *)Entry) /* If next entry is what we're looking for */
        	{
	            TitleObject->Next = ((TITLELISTTYPE *)(TitleObject->Next))->Next; /* Skip one entry */
    	        free(Entry); /* free the entry we dont' want anymore */
        	}
        	TitleObject = (TITLELISTTYPE *)TitleObject->Next; /* Get next entry */
		}
}

void clearTitleList()
{
  TITLELISTTYPE *TitleObject = TitleLinkedList;
  if (TitleObject == &TitleListTerminator) return; /* List is empty */

  while (TitleObject->Next) /* While not terminator */
  {
    TITLELISTTYPE *KillMe=TitleObject;
    TitleObject = (TITLELISTTYPE *)KillMe->Next; /* Get next entry */
    free(KillMe);
  }
  TitleLinkedList = &TitleListTerminator;
}