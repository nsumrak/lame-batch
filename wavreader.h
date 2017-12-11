// WAV file reader class
//
// Copyright(C) 2017. Nebojsa Sumrak <nsumrak@yahoo.com>
//
//   This program is free software; you can redistribute it and / or modify
//	 it under the terms of the GNU General Public License as published by
//	 the Free Software Foundation; either version 2 of the License, or
//	 (at your option) any later version.
//
//	 This program is distributed in the hope that it will be useful,
//	 but WITHOUT ANY WARRANTY; without even the implied warranty of
//	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	 GNU General Public License for more details.
//
//	 You should have received a copy of the GNU General Public License along
//	 with this program; if not, write to the Free Software Foundation, Inc.,
//	 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.

#pragma once

#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "platform.h"

class WavReader
{
protected:
	FILE *inf;

	struct PACKED {
		int16_t tag;
		int16_t channel_number;
		int32_t sample_rate;
		int32_t byte_rate;
		int16_t byte_per_sample;
		int16_t bits_per_sample;
	} wavfmt;

	unsigned long sample_len, data_len, data_read;
	bool fmtfound;

	bool sanity_check()
	{
		return (
			fmtfound && // format descriptor found
			wavfmt.tag == 1 && // only PCM audio supported
			(wavfmt.channel_number == 1 || wavfmt.channel_number == 2) && // only mono and stereo
			(wavfmt.bits_per_sample == 8 || wavfmt.bits_per_sample == 16 || wavfmt.bits_per_sample == 24 || wavfmt.bits_per_sample == 32) // 8, 16, 24 or 32 bits per sample
		);
	}

	bool read_hdr_blocks()
	{
		while (!feof(inf)) {
			struct PACKED {
				char label[4];
				int32_t len;
			} hd;

			debuglog("readblock\n");

			// lead label string
			if (fread(&hd, sizeof(hd), 1, inf) != 1) return false;

			// Decode blocks according to their label
			if (!memcmp(hd.label, "fmt ", 4)) {
				debuglog("fmt block found\n");

				// is there difference in size of fmt block
				int pad = hd.len - sizeof(wavfmt);

				// read what is expected if larger
				if (pad > 0) hd.len = sizeof(wavfmt);

				if (fread(&wavfmt, hd.len, 1, inf) != 1) return false;

				// skip the extra data
				if (pad > 0) fseek(inf, pad, SEEK_CUR);

				fmtfound = true;
			}
			else if (!memcmp(hd.label, "fact", 4))
			{
				debuglog("fact block found\n");
				if (hd.len != 4)
					fseek(inf, hd.len, SEEK_CUR);
				else
					if (fread(&sample_len, 4, 1, inf) != 4) return false;
			}
			else if (!memcmp(hd.label, "data", 4))
			{
				debuglog("data block found\n");
				data_len = hd.len;
				return true;
			}
			else
			{
				debuglog("unknown block found\n");
				fseek(inf, hd.len, SEEK_CUR);
			}
		}
		debuglog("eof reached\n");
		return false;
	}

public:
	bool open(const char *fn)
	{
		close();
		inf = fopen(fn, "rb");
		if (!inf) {
			debuglog("file open error %s\n", fn);
			return false;
		}

		// check header descriptor
		struct PACKED {
			char riff[4];
			int32_t len;
			char wave[4];
		} hd;

		if (fread(&hd, sizeof(hd), 1, inf) != 1 ||
			memcmp("RIFF", hd.riff, 4) ||
			memcmp("WAVE", hd.wave, 4) ||
			!read_hdr_blocks() ||
			!sanity_check())
		{
			// not a WAV file
			close();
			return false;
		}

		return true;
	}

	void close()
	{
		if (inf) {
			fclose(inf);
			inf = 0;
		}
		sample_len = data_len = data_read = 0;
		fmtfound = false;
		memset(&wavfmt, 0, sizeof(wavfmt));
	}

	bool readsample(int &sample)
	{
		if (!inf || data_read >= data_len) return false;
		unsigned char in[4];
		int byte_per_sample = get_byte_per_sample();
		if (fread(in, byte_per_sample, 1, inf) != 1) return false;
		data_read += byte_per_sample;
		switch (byte_per_sample)
		{
		default:	sample = in[0] << 24; return true;
		case 2:		sample = (in[0] << 16) | (in[1] << 24); return true;
		case 3:		sample = (in[0] << 8) | (in[1] << 16) | (in[2] << 24); return true;
		case 4:		sample = in[0] | (in[1] << 8) | (in[1] << 16) | (in[1] << 24); return true;
		}
	}

	bool is_eof()
	{
		return (!inf || data_read >= data_len);
	}

	int get_byte_per_sample()
	{
		assert(inf);
		return (wavfmt.bits_per_sample == 32 ? 4 : wavfmt.bits_per_sample == 24 ? 3 : wavfmt.bits_per_sample == 16 ? 2 : 1);
	}

	int get_bits_per_sample()
	{
		assert(inf);
		return wavfmt.bits_per_sample;
	}

	int get_num_channels()
	{
		assert(inf);
		return wavfmt.channel_number;
	}

	int get_sample_rate()
	{
		assert(inf);
		return wavfmt.sample_rate;
	}

	int get_data_byte_len()
	{
		assert(inf);
		return data_len;
	}

	WavReader()
	{
		inf = 0;
		close();
	}

	~WavReader()
	{
		close();
	}
};
