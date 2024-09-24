//  The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//
//  Mon Nov 23 18:34:01 1998


#include "id3_tag.h"


luint ID3_Tag::instances = 0;


ID3_Tag &operator << (ID3_Tag& tag, ID3_Frame& frame)
{
	tag.AddFrame (&frame);

	return tag;
}


ID3_Tag &operator << (ID3_Tag& tag, ID3_Frame *frame)
{
	tag.AddFrame (frame);

	return tag;
}


ID3_Tag::ID3_Tag()
{
	syncBuffer=0;
	forcedPadding=0;
	SetupTag();

	instances++;
}

void ID3_Tag::ForcePading(luint _padding)
{
	forcedPadding=_padding;
}

void ID3_Tag::SetupTag()
{
	version = ID3_TAGVERSION;
	revision = ID3_TAGREVISION;
	frameList = NULL;
	binaryList = NULL;
	findCursor = NULL;
	syncOn = false;
	compression = false;
	padding = true;
	extendedHeader = false;
	globalUnsync = false;


	Clear();



	return ;
}


ID3_Tag::~ID3_Tag(void)
{

	Clear();

	instances--;

	if (instances == 0)
	{}}


void ID3_Tag::Clear(void)
{
	if (frameList)
	{
		ClearList (frameList);
		frameList = NULL;
	}

	if (binaryList)
	{
		ClearList (binaryList);
		binaryList = NULL;
	}


	findCursor = NULL;
	hasChanged = true;

	free(syncBuffer);
	syncBuffer=0;

	return ;
}


void ID3_Tag::DeleteElem(ID3_Elem *cur)
{
	if (cur)
	{
		if (cur->tagOwns)
		{
			if (cur->frame)
			{
				delete cur->frame;
				cur->frame = NULL;
			}

			if (cur->binary)
			{
				free(cur->binary);
				cur->binary = NULL;
				cur->binarySize=0;
			}
		}

		findCursor = NULL;
		hasChanged = true;

		free(cur);
	}

	return ;
}


void ID3_Tag::ClearList(ID3_Elem *list)
{
	ID3_Elem *cur = list;

	while (cur)
	{
		ID3_Elem *next;

		next = cur->next;
		DeleteElem (cur);
		cur = next;
	}

	return ;
}


void ID3_Tag::AddFrame(ID3_Frame *newFrame, bool freeWhenDone)
{
	ID3_Elem *elem;

	if (newFrame)
	{
		if (elem = (ID3_Elem *)calloc(1, sizeof(ID3_Elem)))
		{
			elem->next = frameList;
			elem->frame = newFrame;
			elem->binary = NULL;
			elem->binarySize = 0;
			elem->tagOwns = freeWhenDone;

			frameList = elem;
			findCursor = NULL;

			hasChanged = true;
		}
		else
			ID3_THROW (ID3E_NoMemory);
	}
	else
		ID3_THROW (ID3E_NoData);

	return ;
}

void ID3_Tag::RemoveFrame(ID3_Frame *frame)
{
	ID3_Elem *elem = NULL;

	if (elem = Find (frame))
		RemoveFromList (elem, &frameList);

	return ;
}


void ID3_Tag::RemoveFromList (ID3_Elem *which, ID3_Elem **list)
{
	ID3_Elem *cur = *list;

	if (cur == which)
	{
		*list = which->next;
		DeleteElem (which);
	}
	else
	{
		while (cur)
		{
			if (cur->next == which)
			{
				cur->next = which->next;
				DeleteElem (which);
				break;
			}
			else
				cur = cur->next;
		}
	}

	return ;
}


bool ID3_Tag::HasChanged (void)
{
	bool changed = hasChanged;

	if (! changed)
	{
		ID3_Elem *cur = frameList;

		while (cur)
		{
			if (cur->frame)
				changed = cur->frame->HasChanged();

			if (changed)
				break;
			else
				cur = cur->next;
		}
	}

	return changed;
}


void ID3_Tag::SetVersion(uchar ver, uchar rev)
{
	if (version != ver || revision != rev)
		hasChanged = true;

	version = ver;
	revision = rev;

	return ;
}


void ID3_Tag::SetUnsync(bool newSync)
{
	if (syncOn != newSync)
		hasChanged = true;

	syncOn = newSync;

	return ;
}

void ID3_Tag::SetExtendedHeader(bool ext)
{
	if (extendedHeader != ext)
		hasChanged = true;

	extendedHeader = ext;

	return ;
}


void ID3_Tag::SetCompression (bool comp)
{
	if (compression != comp)
		hasChanged = true;

	compression = comp;

	return ;
}


void ID3_Tag::SetPadding (bool pad)
{
	if (padding != pad)
		hasChanged = true;

	padding = pad;

	return ;
}


luint ID3_Tag::NumFrames (void)
{
	luint numFrames = 0;
	ID3_Elem *cur = frameList;

	while (cur)
	{
		numFrames++;

		cur = cur->next;
	}

	return numFrames;
}


