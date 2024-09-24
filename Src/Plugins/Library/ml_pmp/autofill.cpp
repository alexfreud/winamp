/*
** This is adapted from the autofill code in ml_ipod. It was originally written by and is
** Copyright (C) Will Fisher and Justin Frankel. It remains under the following licence:
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
*/

#include <windows.h>
#include <time.h>
#include "../../General/gen_ml/ml.h"
#include "pmp.h"
#include "../../General/gen_ml/itemlist.h"
#include "DeviceView.h"
#include "mt19937ar.h"
#include "config.h"
#include "../nu/AutoChar.h"
#include "metadata_utils.h"
#include "api__ml_pmp.h"

extern winampMediaLibraryPlugin plugin;

extern C_ItemList * getSelectedItems(bool all=false);

typedef struct {
	wchar_t * albumName;
	int rating;
	float ratingf;
	int lastplayed;
	int size;
	bool album;
	itemRecordW * ice;
} AutoFillItem;

static void randomizeList(void *list, int elems, int elemsize) {
	if (elems < 2) return;
	if (elemsize < 1) return;
	char *plist=(char*)list;
	int p;
	char *tmp=(char*)calloc(elemsize, sizeof(char));
	if (!tmp) return;
	for (p = 0; p < elems; p ++) {
		int np=genrand_int31()%elems;
		if (p != np) {
			np*=elemsize;
			int pp=p*elemsize;
			memcpy(tmp,plist+pp,elemsize);
			memcpy(plist+pp,plist+np,elemsize);
			memcpy(plist+np,tmp,elemsize);
		}
	}
	free(tmp);
}

#define atoi_NULLOK(s) ((s)?atoi(s):0)
#define STRCMP_NULLOK(a,b) _wcsicmp(a?a:L"",b?b:L"")

static int sortFunc_icealbums(const void *elem1, const void *elem2) {
	itemRecordW * a = (itemRecordW *)elem1;
	itemRecordW * b = (itemRecordW *)elem2;
	return STRCMP_NULLOK(a->album,b->album);
}


static void autoFill_songfilter(Device * dev, C_ItemList * list, itemRecordListW * results) {
	if (results) {
		for(int i=0; i < results->Size; i++) {
			AutoFillItem * a = (AutoFillItem *)calloc(sizeof(AutoFillItem),1);
			a->rating = results->Items[i].rating;
			a->lastplayed = (int)results->Items[i].lastplay;
			a->size = (int)dev->getTrackSizeOnDevice(&results->Items[i]);
			a->album = false;
			a->ice = &results->Items[i];
			a->albumName=L"arse";
			list->Add(a);
		}
	}
}

static void autoFill_songfilter(Device * dev, C_ItemList * list, C_ItemList * results) {
	for(int i=0; i < results->GetSize(); i++) {
		itemRecordW *r = (itemRecordW*)results->Get(i);
		AutoFillItem * a = (AutoFillItem *)calloc(sizeof(AutoFillItem),1);
		a->rating = r->rating;
		a->lastplayed = (int)r->lastplay;
		a->size = (int)dev->getTrackSizeOnDevice(r);
		a->album = false;
		a->ice = r;
		a->albumName=L"arse";
		list->Add(a);
	}
}

static void autoFill_albumfilter(Device * dev, C_ItemList * list, itemRecordListW * results, C_ItemList * res = NULL) {
	if (results)
		qsort(results->Items,results->Size,sizeof(itemRecordW),sortFunc_icealbums);
	AutoFillItem * curalbum = NULL;
	wchar_t * albumName=NULL;
	int albumSize=0;
	__int64 lastplayacc=0;
	int size = res?res->GetSize():(results?results->Size:0);
	int i;
	for(i=0; i < size; i++) {
		itemRecordW * item = res?(itemRecordW*)res->Get(i):&results->Items[i];
		if(!curalbum || STRCMP_NULLOK(item->album,albumName) != 0) {
			if(curalbum && albumSize) {
				curalbum->lastplayed = (int)(lastplayacc / (__int64)albumSize);
				curalbum->ratingf /= (float)albumSize;
				list->Add(curalbum);
			}
			AutoFillItem * a = (AutoFillItem *)calloc(sizeof(AutoFillItem),1);
			a->ratingf = (float)item->rating;
			lastplayacc = item->lastplay;
			a->size = (int)dev->getTrackSizeOnDevice(item);
			a->album = true;
			a->albumName = item->album;
			curalbum = a;
			albumName = a->albumName;
			albumSize++;
		} else {
			albumSize++;
			curalbum->size += (int)dev->getTrackSizeOnDevice(item);
			curalbum->ratingf += (float)item->rating;
			lastplayacc += item->lastplay;
		}
	}
	if(curalbum && albumSize) {
		curalbum->lastplayed = (int)(lastplayacc / (__int64)albumSize);
		curalbum->ratingf /= (float)albumSize;
		list->Add(curalbum);
	}

	//FUCKO: think of something clever to make the ratings relevant
	for(i=0; i < list->GetSize(); i++) {
		AutoFillItem * a = (AutoFillItem *)list->Get(i);
		a->rating = (int)(a->ratingf + 0.5);
		if(a->rating > 5) a->rating = 5;
	}
}

// FUCKO lame O(n^2) gayness. Make this better.
static void autoFill_addAlbums(C_ItemList * albums, itemRecordListW * songs, C_ItemList * dest) {
	if (songs) {
		for(int i=0; i < songs->Size; i++)
			for(int j=0; j < albums->GetSize(); j++)
				if(STRCMP_NULLOK(songs->Items[i].album,(wchar_t *)albums->Get(j))==0) 
					dest->Add(&songs->Items[i]);
	}
}

static int sortFunc_autofill(const void *elem1, const void *elem2)
{
	AutoFillItem *a=(AutoFillItem *)*(void **)elem1;
	AutoFillItem *b=(AutoFillItem *)*(void **)elem2;
	return a->lastplayed - b->lastplayed;
}

itemRecordListW * generateAutoFillList(DeviceView * dev, C_Config * config)
{
	// Settings for an autofill
	int lastAutofill=config->ReadInt(L"LastAutoFill",(int)time(NULL));
	int fillpc = config->ReadInt(L"FillPercent",90);; // fill device 90% full.
	bool byAlbum = dev->dev->getDeviceCapacityTotal() > (__int64)1500000000;
	byAlbum = config->ReadInt(L"AlbumAutoFill",byAlbum?1:0)==1;
	wchar_t * afquery = _wcsdup(config->ReadString(L"AutoFillQuery",L"length > 30"));
	wchar_t * squery = _wcsdup(config->ReadString(L"SyncQuery",L"type = 0"));
	char * tmp2 = AutoCharDup(config->ReadString(L"AutoFillRatings",L"")); // ratings ratio string (eg "3:1:1:1:0:0")
	int len = 2*strlen(tmp2)+2;
	char * tmp = (char*)calloc(len,sizeof(char));
	strncpy(tmp,tmp2,len); free(tmp2);

	int ratingsRatios[6]={0,0,0,0,0,0,};
	bool useratings=true;
	if(tmp[0]==0) useratings=false;
	int i=0;
	int ratingsRatiosTotal=0;
	if(useratings) {
		int len = strlen(tmp);
		while(tmp[++i]!=0) if(tmp[i]==':') tmp[i]=0;
		tmp[i+1]=0;
		char * p = &tmp[0];
		for(i=0; i<6; i++) {
			if(p > len + tmp) break;
			if(*p == 0) break;
			ratingsRatios[i] = atoi_NULLOK(p);
			ratingsRatiosTotal+=ratingsRatios[i];
			p+=strlen(p)+1;
		}
	}
	if(ratingsRatiosTotal==0) {
		ratingsRatiosTotal=6;
		for(i=0; i<6; i++) ratingsRatios[i]=1;
	}

	//construct query
	wchar_t query[2048]=L"";
	if(afquery[0]==0) wsprintf(query,L"%s",squery);
	else wsprintf(query,L"(%s) AND (%s)",squery,afquery);

	//run query
	itemRecordListW *items = 0;
	itemRecordListW *results = AGAVE_API_MLDB?AGAVE_API_MLDB->Query(query):0;
	if (results)
	{
		C_ItemList * songList = new C_ItemList;

		if(!byAlbum) autoFill_songfilter(dev->dev,songList, results);
		else autoFill_albumfilter(dev->dev,songList, results);

		//dump into ratings "bins"
		C_ItemList * songs[7];
		for(i=0; i<7; i++) songs[i] = new C_ItemList;
		for(i=0; i<songList->GetSize(); i++) {
			if(useratings) {
				int rating = ((AutoFillItem*)songList->Get(i))->rating;
				switch (rating) {
			case 1: songs[1]->Add(songList->Get(i)); break;
			case 2: songs[2]->Add(songList->Get(i)); break;
			case 3: songs[3]->Add(songList->Get(i)); break;
			case 4: songs[4]->Add(songList->Get(i)); break;
			case 5: songs[5]->Add(songList->Get(i)); break;
			default: songs[0]->Add(songList->Get(i)); break;
				}
			} else songs[0]->Add(songList->Get(i));
		}

		for(i=0; i<6; i++)  
		{
			randomizeList(songs[i]->GetAll(),songs[i]->GetSize(),sizeof(void*)); // randomize
			qsort(songs[i]->GetAll(),songs[i]->GetSize(),sizeof(void*),sortFunc_autofill); // sort by date
		}


		__int64 sizeToFill, totalSize=0;

		sizeToFill = (((__int64)fillpc) * dev->dev->getDeviceCapacityTotal()) / ((__int64)100);
		//sizeToFill = dev->dev->getDeviceCapacityTotal();

		C_ItemList * alreadyIn = new C_ItemList;
		C_ItemList * notGoingIn = new C_ItemList;
		C_ItemList * songsToSend = new C_ItemList;

		if(!byAlbum) {

			int numTracks = dev->dev->getPlaylistLength(0);
			/* Hmm. This should make autofill just replace selected songs
			** gonna leave it out for now, until i have good ipod shuffle (et al) support.
			C_ItemList * selected = getSelectedItems(false);
			int j=0;
			if(selected) {
			if(selected->GetSize() > 0 && selected->GetSize() < numTracks) {
			for(i=0; i<numTracks; i++) {
			if(j < selected->GetSize()) if((songid_t)selected->Get(j) == dev->dev->getPlaylistTrack(0,i)) {
			j++;
			notGoingIn->Add(selected->Get(j));
			} else alreadyIn->Add(dev->dev->getPlaylistTrack(0,i));
			}
			}
			delete selected;
			}
			*/
			// otherwise replace ones played since last autofill, unless none played, in which case replace all
			if(notGoingIn->GetSize() == 0) {
				delete alreadyIn;
				alreadyIn = new C_ItemList;
				if(lastAutofill > 0)
				{
					for(i=0; i<numTracks; ++i)
						if(dev->dev->getTrackLastPlayed(dev->dev->getPlaylistTrack(0,i)) >= lastAutofill) break;

					if (i >= numTracks)  // this means nothing has been played, so in this case we set everything to be replaced
					{
						for(i=0; i<numTracks; ++i) notGoingIn->Add((void*)dev->dev->getPlaylistTrack(0,i));
					}       
					else 
					{
						for(i=0; i < numTracks; ++i) 
						{
							songid_t s = dev->dev->getPlaylistTrack(0,i);
							if(dev->dev->getTrackLastPlayed(s) <= lastAutofill)  alreadyIn->Add((void*)s);
						}
					}
				}
			}

			//remove our already selected songs from the bins
			for(i=0; i<notGoingIn->GetSize(); ++i) {
				songid_t s = (songid_t)notGoingIn->Get(i);
				for (int b=0; b < 6; b ++)
				{
					for(int j=0; j<songs[b]->GetSize(); ++j) 
					{
						if(!compareItemRecordAndSongId(((AutoFillItem *)songs[b]->Get(j))->ice,s, dev->dev))
						{ // FUCKO
							songs[6]->Add(songs[b]->Get(j));
							songs[b]->Del(j);
							b=6;
							break;
						}
					}
				}
			}
			for(i=0; i<alreadyIn->GetSize(); ++i) {
				songid_t s = (songid_t)alreadyIn->Get(i);
				for (int b= 0; b < 6; b ++)
				{
					for(int j=0; j<songs[b]->GetSize(); ++j) 
					{
						if(!compareItemRecordAndSongId(((AutoFillItem *)songs[b]->Get(j))->ice,s, dev->dev)) 
						{ 
							songsToSend->Add(((AutoFillItem *)songs[b]->Get(j))->ice);
							songs[b]->Del(j);
							b=6;
							break;
						}
					}
				}

				__int64 size = (__int64)dev->dev->getTrackSize(s);

				if(totalSize + size >= sizeToFill) break;
				totalSize += size;
			}
		} // end if(!byAlbum)

		//select the rest!
		C_ItemList * albumsToSend = new C_ItemList;

		while(true) {
			int bin=0;
			if (useratings) 
			{
				int val = genrand_int31() % ratingsRatiosTotal;
				bin=-1;
				while(val>=0 && bin < 5) val -= ratingsRatios[++bin];

				if (bin > 5) bin=5;
				else if(bin<0) bin=0;

				bin = 5-bin; // work in reverse mapped bin now (since ratingsRatio is 5-0)

				int sbin=bin;
				while(bin<6&&songs[bin]->GetSize()<=0) { bin++; } // if our bin is empty, go to a higher bin

				if (bin > 5) // out of higher rated bins, go lower
				{
					bin=sbin-1; // start at one lower than where we started
					while (bin >= 0 && songs[bin]->GetSize()<=0) bin--; // if our bin is still empty, go to a lower bin
				}

				if (bin < 0) bin = 6; 
			} else bin = songs[0]->GetSize()?0:6;
			if(!songs[bin])
				break;
			int bsize=songs[bin]->GetSize();
			if (bsize<1)
				break;

			// JF> will had a %(bsize/2) here effectively, which I think is too major.
			// I propose a nice simple weighted thing, where stuff near the beginning is more likely
			// to be picked.
			int snum = genrand_int31() % bsize;
			if (genrand_int31()&1) snum/=2; // half the time, just use the first half (less recently played items)

			int size = ((AutoFillItem *)songs[bin]->Get(snum))->size;
			if(size <= 0) 
			{
				songs[bin]->Del(snum);
				continue;
			}
			totalSize += (__int64)size;

			/*{
			wchar_t buf[100] = {0};
			wsprintf(buf,L"not full. must fill: %d, so far: %d, this track: %d",(int)sizeToFill,(int)totalSize,(int)size);
			OutputDebugString(buf);
			}*/

			if(totalSize >= sizeToFill) 
			{ 
				/*OutputDebugString(L"full");*/
				break;
			}

			if(byAlbum) {
				albumsToSend->Add(((AutoFillItem *)songs[bin]->Get(snum))->albumName);
			} else  {
				songsToSend->Add(((AutoFillItem *)songs[bin]->Get(snum))->ice);
			}
			songs[bin]->Del(snum);
		}

		// dump our albums into songsToSend
		if(albumsToSend->GetSize() > 0)
			autoFill_addAlbums(albumsToSend, results, songsToSend);

		items = new itemRecordListW;
		items->Alloc = songsToSend->GetSize();
		items->Size = songsToSend->GetSize();
		items->Items = (itemRecordW*)calloc(items->Alloc, sizeof(itemRecordW));

		for(i=0; i<songsToSend->GetSize(); ++i) copyRecord(&items->Items[i],(itemRecordW*)songsToSend->Get(i));

		// clear stuff up
		for(i=0; i < songList->GetSize(); i++) free(songList->Get(i));
		for(i=0; i<7; ++i) delete songs[i];
		delete songsToSend;
		delete albumsToSend;
		delete songList;

		delete notGoingIn;
		delete alreadyIn;

		AGAVE_API_MLDB->FreeRecordList(results); //free memory
	}

	free(afquery);
	free(squery);
	free(tmp);

	return items;
}
