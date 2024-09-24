#pragma once

#include "header.h"
#include "nu/PtrDeque.h"
#include "frame.h"
#include "extendedheader.h"

/* benski> random thoughts 

Frames are stored as raw bytes.  Users of this library will have to parse/encode
on their own.  But we'll supply a helper library for that.

new frames must be created from the tag object (not standalone "new ID3v2::Frame")
so that revision & version information (and any relevant tag-wide flags) can be inherited
*/
namespace ID3v2
{
	class Tag : public ID3v2::Header
	{
	public:
		Tag(const ID3v2::Header &header);
		virtual ~Tag() {}
		/* finds the first frame with the desired identifier.  */
		ID3v2::Frame *FindFirstFrame(const int8_t *id) const;
		/* finds the next frame with the same identifier as the passed in frame 
		"frame" parameter MUST be a frame returned from the same Tag object! */
		ID3v2::Frame *FindNextFrame(const ID3v2::Frame *frame) const;
		void RemoveFrame(ID3v2::Frame *frame);
		void RemoveFrames(const int8_t *id);
		void AddFrame(ID3v2::Frame *frame);
		virtual int Parse(const void *data, size_t len)=0;
		virtual int SerializedSize(uint32_t *length, uint32_t padding_size, int flags) const=0;
		// tag will be padded up to length.  use SerializedSize() to retrieve the length to use!
		virtual int Serialize(void *data, uint32_t len, int flags) const=0;
		virtual ID3v2::Frame *FindFirstFrame(int frame_id) const = 0;
		virtual void RemoveFrames(int frame_id)=0;
		virtual ID3v2::Frame *NewFrame(int frame_id, int flags) const=0;
		ID3v2::Frame *EnumerateFrame(const ID3v2::Frame *position) const;
	protected:
		typedef nu::PtrDeque<ID3v2::Frame> FrameList;
		nu::PtrDeque<ID3v2::Frame> frames;
	};
}
namespace ID3v2_2
{
	class Tag : public ID3v2::Tag
	{
	public:
		Tag(const ID3v2::Header &header);
		~Tag();
		int Parse(const void *data, size_t len);
		int SerializedSize(uint32_t *length, uint32_t padding_size, int flags) const;
		int Serialize(void *data, uint32_t len, int flags) const;
		/* finds the first frame with the desired identifier.  */
		ID3v2_2::Frame *FindFirstFrame(int frame_id) const;
		/* finds the next frame with the same identifier as the passed in frame 
		"frame" parameter MUST be a frame returned from the same Tag object! */
		void RemoveFrames(int frame_id);
		ID3v2_2::Frame *NewFrame(int frame_id, int flags) const;
	private:
		ID3v2_3::ExtendedHeader extendedHeader;
	};
}

namespace ID3v2_3
{
	class Tag : public ID3v2::Tag
	{
	public:
		Tag(const ID3v2::Header &header);
		~Tag();
		int Parse(const void *data, size_t len);
		int SerializedSize(uint32_t *length, uint32_t padding_size, int flags) const;
		int Serialize(void *data, uint32_t len, int flags) const;
		/* finds the first frame with the desired identifier.  */
		ID3v2_3::Frame *FindFirstFrame(int frame_id) const;

		void RemoveFrames(int frame_id);
		ID3v2_3::Frame *NewFrame(int frame_id, int flags) const;
	private:
		ID3v2_3::ExtendedHeader extendedHeader;
	};
}

namespace ID3v2_4
{
	class Tag : public ID3v2::Tag
	{
	public:
		Tag(const ID3v2::Header &header);
		~Tag();
		int Parse(const void *data, size_t len);
		int SerializedSize(uint32_t *length, uint32_t padding_size, int flags) const;
		int Serialize(void *data, uint32_t len, int flags) const;
		/* finds the first frame with the desired identifier.  */
		ID3v2_4::Frame *FindFirstFrame(int frame_id) const;
		/* finds the next frame with the same identifier as the passed in frame 
		"frame" parameter MUST be a frame returned from the same Tag object! */
		ID3v2::Frame *FindNextFrame(const ID3v2::Frame *frame) const { return FindNextFrame((const Frame *)frame); }

		void RemoveFrames(int frame_id);
		ID3v2_4::Frame *NewFrame(int frame_id, int flags) const;
	private:
		ID3v2_4::ExtendedHeader extendedHeader;
	};
}
