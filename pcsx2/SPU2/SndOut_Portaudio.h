/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "SndOutModule.h"

#define _WIN32_DCOM
#ifdef _WIN32
#include <portaudio/include/portaudio.h>
#elif defined(__linux__) || defined(__APPLE__)
#include "portaudio.h"
#endif

#include <vector>

class PortaudioOutModule : public SndOutModule
{
public:
	bool m_UseHardware; // unused

	bool m_WasapiExclusiveMode;
	bool m_SuggestedLatencyMinimal;
	int m_SuggestedLatencyMS;

	int m_ApiId;
	std::vector<std::string> m_apiChoices;

	wxString m_Device;
	std::vector<std::string> m_deviceChoices;

private:
	//////////////////////////////////////////////////////////////////////////////////////////
	// Instance vars

	int writtenSoFar;
	int writtenLastTime;
	int availableLastTime;

	int actualUsedChannels;

	bool started;
	PaStream* stream;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Stuff necessary for speaker expansion
	class SampleReader
	{
	public:
		virtual int ReadSamples(const void* inputBuffer, void* outputBuffer,
			unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void* userData) = 0;
	};

	template <class T>
	class ConvertedSampleReader : public SampleReader
	{
		int* written;

	public:
		ConvertedSampleReader(int* pWritten)
		{
			written = pWritten;
		}

		virtual int ReadSamples(const void* inputBuffer, void* outputBuffer,
			unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void* userData)
		{
			T* p1 = (T*)outputBuffer;

			int packets = framesPerBuffer / SndOutPacketSize;

			for (int p = 0; p < packets; p++, p1 += SndOutPacketSize)
				SndBuffer::ReadSamples(p1);

			(*written) += packets * SndOutPacketSize;

			return 0;
		}
	};

public:
	SampleReader* ActualPaCallback;

	PortaudioOutModule();

	s32 Init();

	void Close();

	s32 Test() const
	{
		return 0;
	}

	int GetEmptySampleCount();

	const std::string GetIdent() const
	{
		return "portaudio";
	}

	const std::string GetLongName() const
	{
		return "PortAudio (Cross-platform)";
	}

	void ReadSettings();

	void SetApiSettings(wxString api);

	void WriteSettings() const;

};
