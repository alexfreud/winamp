#pragma once

class SpillBuffer
{
public:
	SpillBuffer();
	~SpillBuffer();
	bool reserve(size_t bytes);
	void clear();
	void reset();
	size_t write(const void *src, size_t len);
	bool get(void **buffer, size_t *len);
	bool full() const;
	bool empty() const;
	void remove(size_t len); // removes len bytes from the start of the spill buffer
	size_t remaining() const; // how many bytes to fill it up
	size_t length() const; /* buffer length when full */

private:
	volatile size_t spillBufferUsed;
	size_t spillBufferSize;
	char *spillBuffer;
};