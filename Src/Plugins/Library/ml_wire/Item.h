#pragma once
#include <bfc/platform/types.h>
#include <time.h>
#include <windows.h>

namespace RSS
{
	class Item
	{
	public:
		Item();
		~Item();
		Item(const Item &copy);
		const Item &operator =(const Item &copy);

		HRESULT GetDownloadFileName(const wchar_t *channelName, wchar_t *buffer, int bufferMax, BOOL fValidatePath) const;
		bool listened;
		bool downloaded;
		__time64_t publishDate;
		bool generatedDate;

	//protected:
		wchar_t *itemName;
		wchar_t *url;
		wchar_t *sourceUrl;
		wchar_t *guid;
		wchar_t *description;
		wchar_t *link;
		wchar_t *duration;
		int64_t size;
		
	private:
		void Init();
		void Reset();
	};

	class MutableItem : public Item
	{
	public:
		void SetItemName(const wchar_t *value);
		void SetLink(const wchar_t *value);
		void SetURL(const wchar_t *value);
		void SetSourceURL(const wchar_t *value);
		void SetGUID(const wchar_t *value);
		void SetDescription(const wchar_t *value);
		void SetDuration(const wchar_t *value);
		void SetSize(const wchar_t * _size);
	};

}