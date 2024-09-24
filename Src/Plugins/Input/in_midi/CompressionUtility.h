#pragma once

/// <summary>
/// 
/// </summary>
class CompressionUtility
{
public:
	static int CompressAsGZip(const void* input, size_t input_size, void** ppvOut, size_t& out_size);
	static int DecompressGZip(const void* input, size_t input_size, void** ppvOut, size_t& out_size);
	static int DecompressPKZip(const char* fn, void** ppvOut, size_t& out_size);
};

