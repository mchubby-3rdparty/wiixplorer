/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * 3Band resampling thanks to libmad
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "SoundDecoder.hpp"

SoundDecoder::SoundDecoder()
{
	file_fd = NULL;
	Init();
}

SoundDecoder::SoundDecoder(const char * filepath)
{
	file_fd = new CFile(filepath, "rb");
	Init();
}

SoundDecoder::SoundDecoder(const u8 * buffer, int size)
{
	file_fd = new CFile(buffer, size);
	Init();
}

SoundDecoder::~SoundDecoder()
{
	ExitRequested = true;
	while(Decoding)
		usleep(1000);

	if(file_fd)
		delete file_fd;
	file_fd = NULL;
}

void SoundDecoder::Init()
{
	SoundType = SOUND_RAW;
	SoundBlocks = Settings.SoundblockCount;
	SoundBlockSize = Settings.SoundblockSize;
	CurPos = 0;
	whichLoad = 0;
	Loop = false;
	EndOfFile = false;
	Decoding = false;
	ExitRequested = false;
	SoundBuffer.SetBufferBlockSize(SoundBlockSize);
	SoundBuffer.Resize(SoundBlocks);
}

int SoundDecoder::Rewind()
{
	CurPos = 0;
	EndOfFile = false;
	file_fd->rewind();

	return 0;
}

int SoundDecoder::Read(u8 * buffer, int buffer_size, int pos UNUSED)
{
	int ret = file_fd->read(buffer, buffer_size);
	CurPos += ret;

	return ret;
}

void SoundDecoder::Decode()
{
	if(!file_fd || ExitRequested || EndOfFile)
		return;

	// check if we are not at the pre-last buffer (last buffer is playing)
	u16 whichPlaying = SoundBuffer.Which();
	if(	   (whichLoad == (whichPlaying-2))
		|| ((whichPlaying == 0) && (whichLoad == SoundBuffer.Size()-2))
		|| ((whichPlaying == 1) && (whichLoad == SoundBuffer.Size()-1)))
	{
		return;
	}

	Decoding = true;

	int done  = 0;
	u8 * write_buf = SoundBuffer.GetBuffer(whichLoad);
	if(!write_buf)
	{
		ExitRequested = true;
		Decoding = false;
		return;
	}

	while(done < SoundBlockSize)
	{
		int ret = Read(&write_buf[done], SoundBlockSize-done, Tell());

		if(ret <= 0)
		{
			if(Loop)
			{
				Rewind();
				continue;
			}
			else
			{
				EndOfFile = true;
				break;
			}
		}

		done += ret;
	}

	if(done > 0)
	{
		SoundBuffer.SetBufferSize(whichLoad, done);
		SoundBuffer.SetBufferReady(whichLoad, true);
		if(++whichLoad >= SoundBuffer.Size())
			whichLoad = 0;
	}

	// check if next in queue needs to be filled as well and do so
	if(!SoundBuffer.IsBufferReady(whichLoad))
		Decode();

	Decoding = false;
}

