#include "zlib/zlib.h"

////////////////////////////////////////////////////////////////////////////////

/*
typedef struct z_stream_s {
    Bytef    *next_in;					// 38
    uInt     avail_in;					// 34
    uLong    total_in;					// 30

    Bytef    *next_out;					// 2C
    uInt     avail_out;					// 28
    uLong    total_out;					// 24

    char     *msg;						// 20
    struct internal_state FAR *state;	// 1C

    alloc_func zalloc;					// 18
    free_func  zfree;					// 14
    voidpf     opaque;					// 10

    int     data_type;					// 0C
    uLong   adler;						// 08
    uLong   reserved;					// 04
} z_stream;
*/


int __cdecl cps_compress(Bytef* dst, uLongf* dstlen, const Bytef* src, uLongf srclen)
{
	z_stream stream;
	int result;

	stream.next_in = const_cast<Bytef*>(src);
	stream.avail_in = static_cast<uInt>(srclen);
	stream.next_out = dst;
	stream.avail_out = static_cast<uInt>(*dstlen);
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	result = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	if (result != Z_OK)
		return result;

	result = deflate(&stream, Z_FINISH);
	if (result != Z_STREAM_END)
	{
		deflateEnd(&stream);

		if (result == Z_OK)
			result = Z_BUF_ERROR;

		return result;
	}

	*dstlen = stream.total_out;
	result = deflateEnd(&stream);

	return result;
}


int __cdecl cps_uncompress(Bytef* dst, uLongf* dstlen, const Bytef* src, uLongf srclen)
{
	z_stream stream;
	int result;

	stream.next_in = const_cast<Bytef*>(src);
	stream.avail_in = static_cast<uInt>(srclen);
	stream.next_out = dst;
	stream.avail_out = static_cast<uInt>(*dstlen);
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;

	result = inflateInit(&stream);
	if (result != Z_OK)
		return result;

	result = inflate(&stream, Z_FINISH);
	if (result != Z_STREAM_END)
	{
		inflateEnd(&stream);

		if (result == Z_OK)
			result = Z_BUF_ERROR;

		return result;
	}

	*dstlen = stream.total_out;
	result = inflateEnd(&stream);

	return result;
}
