#include "win32_avi_reader.h"
#include <strsafe.h>

const int _BUFFER_SIZE_ = 16384;

AVIReaderWin32::AVIReaderWin32()
{
	hFile = 0;

	_ring_buffer.reserve( _BUFFER_SIZE_ );

	end_of_file       = false;

	position.QuadPart = 0;
	local_filename    = 0;
}

AVIReaderWin32::~AVIReaderWin32()
{
	free( local_filename );
}

void AVIReaderWin32::Close()
{
	if ( hFile && hFile != INVALID_HANDLE_VALUE )
	{
		//CancelIo(hFile);
		CloseHandle( hFile );
	}
}

uint64_t AVIReaderWin32::GetContentLength()
{
	LARGE_INTEGER position;
	position.QuadPart = 0;
	position.LowPart  = GetFileSize( hFile, (LPDWORD)&position.HighPart );

	if ( position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR )
		return 0;
	else
		return position.QuadPart;
}

void AVIReaderWin32::GetFilename( wchar_t *fn, size_t len )
{
	StringCchCopyW( fn, len, local_filename );
}

int AVIReaderWin32::Open( const wchar_t *filename )
{
	free( local_filename );
	local_filename = _wcsdup( filename );

	hFile = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0 );
	if ( hFile == INVALID_HANDLE_VALUE )
		return nsavi::READ_NOT_FOUND;

	return nsavi::READ_OK;
}

/* used by RingBuffer::fill() */
size_t AVIReaderWin32::Read( void *dest, size_t len )
{
	// TODO: use overlapped I/O so can we wait on the read simultaneously with the killswitch and seek_event
	DWORD bytes_read = 0;	
	if ( ReadFile( hFile, dest, (DWORD)len, &bytes_read, NULL ) && bytes_read != len )
		end_of_file = true;

	return bytes_read;
}

int AVIReaderWin32::Read( void *p_read_buffer, uint32_t read_length, uint32_t *bytes_read )
{
	if ( end_of_file && _ring_buffer.empty() )
		return nsavi::READ_EOF;

	size_t total_bytes_read = 0;

	while ( read_length && !( end_of_file && _ring_buffer.empty() ) )
	{
		// read what we can from the buffer
		size_t bytes_read  = _ring_buffer.read( p_read_buffer, read_length );
		p_read_buffer      = (uint8_t *)p_read_buffer + bytes_read;
		read_length       -= (uint32_t)bytes_read;
		total_bytes_read  += bytes_read;
		position.QuadPart += bytes_read;

		if ( read_length > _BUFFER_SIZE_ )
		{
			// read directly from the file if we have a large read
			bytes_read         = Read( p_read_buffer, read_length );
			p_read_buffer      = (uint8_t *)p_read_buffer + bytes_read;
			read_length       -= (uint32_t)bytes_read;
			total_bytes_read  += bytes_read;
			position.QuadPart += bytes_read;
		}
		else
		{
			// refill buffer if necessary
			_ring_buffer.fill( this, _BUFFER_SIZE_ );
		}
	}

	*bytes_read = (uint32_t)total_bytes_read;

	return nsavi::READ_OK;
}

int AVIReaderWin32::Peek( void *read_buffer, uint32_t read_length, uint32_t *bytes_read )
{
	if ( end_of_file && _ring_buffer.empty() )
		return nsavi::READ_EOF;

	// refill buffer if necessary
	if ( _ring_buffer.size() < read_length )
		_ring_buffer.fill( this, _BUFFER_SIZE_ );

	*bytes_read = (uint32_t)_ring_buffer.peek( read_buffer, read_length );

	return nsavi::READ_OK;
}

static LONGLONG Seek64( HANDLE hf, __int64 distance, DWORD MoveMethod )
{
	LARGE_INTEGER li;
	li.QuadPart = distance;
	li.LowPart = SetFilePointer( hf, li.LowPart, &li.HighPart, MoveMethod );
	if ( li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

int AVIReaderWin32::Seek( uint64_t new_position )
{
	_ring_buffer.clear();

	position.QuadPart = Seek64( hFile, new_position, SEEK_SET );
	end_of_file       = ( position.QuadPart != new_position );

	return nsavi::READ_OK;
}

uint64_t AVIReaderWin32::Tell()
{
	return position.QuadPart;
}

int AVIReaderWin32::Skip( uint32_t skip_bytes )
{
	if ( end_of_file && _ring_buffer.empty() )
		return nsavi::READ_EOF;

	if ( skip_bytes < _ring_buffer.size() )
	{
		_ring_buffer.advance( skip_bytes );

		position.QuadPart += skip_bytes;

		return nsavi::READ_OK;
	}
	else
	{
		return Seek( position.QuadPart + skip_bytes );
	}
}

void AVIReaderWin32::OverlappedHint( uint32_t read_length )
{
	if ( read_length > _ring_buffer.size() )
		_ring_buffer.fill( this, _BUFFER_SIZE_ );
}
