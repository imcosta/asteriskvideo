#ifndef _MEDIA_H_
#define _MEDIA_H_

#include "H223Const.h"

enum MediaType
{
	e_Audio = 0,
	e_Video = 1
};

enum MediaCodec
{
	e_AMR	= 0,
	e_H263	= 1
};

class Frame
{
public:
	Frame(MediaType type,MediaCodec codec,BYTE *data,DWORD length);
	~Frame();

	MediaType	type;
	MediaCodec	codec;
	BYTE*		data;
	DWORD		dataLength;
};
#endif