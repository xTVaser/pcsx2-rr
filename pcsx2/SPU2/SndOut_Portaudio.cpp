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

#include "PrecompiledHeader.h"
#include "Global.h"
#include "SndOut.h"
#include "SPU2/wx/CfgHelpers.h"

#include "SndOut_Portaudio.h"

#include <cwchar>
#include <vector>

#ifdef _WIN32
#include <portaudio/include/pa_win_wasapi.h>
#endif
#include "Debug.h"

int PaCallback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData);

PortaudioOutModule::PortaudioOutModule()
{
	m_SuggestedLatencyMinimal = true;
	m_UseHardware = false;
	m_WasapiExclusiveMode = false;
	started = false;
	stream = nullptr;
	ActualPaCallback = nullptr;
	m_ApiId = -1;
	m_SuggestedLatencyMS = 20;
	actualUsedChannels = 0;
	writtenSoFar = 0;
	writtenLastTime = 0;
	availableLastTime = 0;
}

s32 PortaudioOutModule::Init()
{
	started = false;
	stream = nullptr;

	ReadSettings();

	PaError err = Pa_Initialize();
	if (err != paNoError)
	{
		fprintf(stderr, "* SPU2: PortAudio error: %s\n", Pa_GetErrorText(err));
		return -1;
	}
	started = true;

	int deviceIndex = -1;

	fprintf(stderr, "* SPU2: Enumerating PortAudio devices:\n");
	for (int i = 0, j = 0; i < Pa_GetDeviceCount(); i++)
	{
		const PaDeviceInfo* info = Pa_GetDeviceInfo(i);

		if (info->maxOutputChannels > 0)
		{
			const PaHostApiInfo* apiinfo = Pa_GetHostApiInfo(info->hostApi);

			fprintf(stderr, " *** Device %d: '%s' (%s)", j, info->name, apiinfo->name);

			if (apiinfo->type == m_ApiId)
			{
				if (m_Device == wxString::FromUTF8(info->name))
				{
					deviceIndex = i;
					fprintf(stderr, " (selected)");
				}
			}
			fprintf(stderr, "\n");

			j++;
		}
	}
	fflush(stderr);

	if (deviceIndex < 0 && m_ApiId >= 0)
	{
		for (int i = 0; i < Pa_GetHostApiCount(); i++)
		{
			const PaHostApiInfo* apiinfo = Pa_GetHostApiInfo(i);
			if (apiinfo->type == m_ApiId)
			{
				deviceIndex = apiinfo->defaultOutputDevice;
			}
		}
	}

	if (deviceIndex >= 0)
	{
		void* infoPtr = nullptr;

		const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(deviceIndex);

		int speakers;
		switch (numSpeakers) // speakers = (numSpeakers + 1) *2; ?
		{
			case 0:
				speakers = 2;
				break; // Stereo
			case 1:
				speakers = 4;
				break; // Quadrafonic
			case 2:
				speakers = 6;
				break; // Surround 5.1
			case 3:
				speakers = 8;
				break; // Surround 7.1
			default:
				speakers = 2;
		}
		actualUsedChannels = std::min(speakers, devinfo->maxOutputChannels);

		switch (actualUsedChannels)
		{
			case 2:
				ConLog("* SPU2 > Using normal 2 speaker stereo output.\n");
				ActualPaCallback = new ConvertedSampleReader<Stereo20Out32>(&writtenSoFar);
				break;

			case 3:
				ConLog("* SPU2 > 2.1 speaker expansion enabled.\n");
				ActualPaCallback = new ConvertedSampleReader<Stereo21Out32>(&writtenSoFar);
				break;

			case 4:
				ConLog("* SPU2 > 4 speaker expansion enabled [quadraphenia]\n");
				ActualPaCallback = new ConvertedSampleReader<Stereo40Out32>(&writtenSoFar);
				break;

			case 5:
				ConLog("* SPU2 > 4.1 speaker expansion enabled.\n");
				ActualPaCallback = new ConvertedSampleReader<Stereo41Out32>(&writtenSoFar);
				break;

			case 6:
			case 7:
				switch (dplLevel)
				{
					case 0:
						ConLog("* SPU2 > 5.1 speaker expansion enabled.\n");
						ActualPaCallback = new ConvertedSampleReader<Stereo51Out32>(&writtenSoFar); //"normal" stereo upmix
						break;
					case 1:
						ConLog("* SPU2 > 5.1 speaker expansion with basic ProLogic dematrixing enabled.\n");
						ActualPaCallback = new ConvertedSampleReader<Stereo51Out32Dpl>(&writtenSoFar); // basic Dpl decoder without rear stereo balancing
						break;
					case 2:
						ConLog("* SPU2 > 5.1 speaker expansion with experimental ProLogicII dematrixing enabled.\n");
						ActualPaCallback = new ConvertedSampleReader<Stereo51Out32DplII>(&writtenSoFar); //gigas PLII
						break;
				}
				actualUsedChannels = 6; // we do not support 7.0 or 6.2 configurations, downgrade to 5.1
				break;

			default: // anything 8 or more gets the 7.1 treatment!
				ConLog("* SPU2 > 7.1 speaker expansion enabled.\n");
				ActualPaCallback = new ConvertedSampleReader<Stereo71Out32>(&writtenSoFar);
				actualUsedChannels = 8; // we do not support 7.2 or more, downgrade to 7.1
				break;
		}

#ifdef _WIN32
		PaWasapiStreamInfo info = {
			sizeof(PaWasapiStreamInfo),
			paWASAPI,
			1,
			paWinWasapiExclusive};

		if ((m_ApiId == paWASAPI) && m_WasapiExclusiveMode)
		{
			// Pass it the Exclusive mode enable flag
			infoPtr = &info;
		}
#endif

		PaStreamParameters outParams = {

			//	PaDeviceIndex device;
			//	int channelCount;
			//	PaSampleFormat sampleFormat;
			//	PaTime suggestedLatency;
			//	void *hostApiSpecificStreamInfo;
			deviceIndex,
			actualUsedChannels,
			paInt32,
			m_SuggestedLatencyMinimal ? (SndOutPacketSize / (float)SampleRate) : (m_SuggestedLatencyMS / 1000.0f),
			infoPtr};

		err = Pa_OpenStream(&stream,
			nullptr, &outParams, SampleRate,
			SndOutPacketSize,
			paNoFlag,
			PaCallback,

			nullptr);

		if (err == paInvalidSampleRate && SampleRate == 48000)
		{
			DevCon.Warning("Failed to create device at 48khz, trying 96khz");
			SampleRate = 96000;
			err = Pa_OpenStream(&stream,
				nullptr, &outParams, SampleRate,
				SndOutPacketSize,
				paNoFlag,
				PaCallback,

				nullptr);
		}

		if (err == paInvalidSampleRate && SampleRate == 96000) // It didn't work, so lets just put the samplerate back
			SampleRate = 48000;
	}
	else
	{
		err = Pa_OpenDefaultStream(&stream,
			0, actualUsedChannels, paInt32, 48000,
			SndOutPacketSize,
			PaCallback,
			nullptr);
	}
	if (err != paNoError)
	{
		if (err == paInvalidSampleRate)
			Console.Warning("Failed to create Port Audio Device %dkhz, Please use Exclusive Mode", SampleRate / 1000);
		fprintf(stderr, "* SPU2: PortAudio error: %s\n", Pa_GetErrorText(err));
		Pa_Terminate();
		return -1;
	}

	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		fprintf(stderr, "* SPU2: PortAudio error: %s\n", Pa_GetErrorText(err));
		Pa_CloseStream(stream);
		stream = nullptr;
		Pa_Terminate();
		return -1;
	}

	return 0;
}



void PortaudioOutModule::Close()
{
	PaError err;
	if (started)
	{
		if (stream)
		{
			if (Pa_IsStreamActive(stream))
			{
				err = Pa_StopStream(stream);
				if (err != paNoError)
					fprintf(stderr, "* SPU2: PortAudio error: %s\n", Pa_GetErrorText(err));
			}

			err = Pa_CloseStream(stream);
			if (err != paNoError)
				fprintf(stderr, "* SPU2: PortAudio error: %s\n", Pa_GetErrorText(err));

			stream = nullptr;
		}

		// Seems to do more harm than good.
		//PaError err = Pa_Terminate();
		//if( err != paNoError )
		//	fprintf(stderr,"* SPU2: PortAudio error: %s\n", Pa_GetErrorText( err ) );

		started = false;
	}
}



int PortaudioOutModule::GetEmptySampleCount()
{
	long availableNow = Pa_GetStreamWriteAvailable(stream);

	int playedSinceLastTime = (writtenSoFar - writtenLastTime) + (availableNow - availableLastTime);
	writtenLastTime = writtenSoFar;
	availableLastTime = availableNow;

	// Lowest resolution here is the SndOutPacketSize we use.
	return playedSinceLastTime;
}

void PortaudioOutModule::ReadSettings()
{
	wxString api(L"EMPTYEMPTYEMPTY");
	m_Device = L"EMPTYEMPTYEMPTY";
#ifdef __linux__
	// By default on linux use the ALSA API (+99% users) -- Gregory
	CfgReadStr(L"PORTAUDIO", L"HostApi", api, L"ALSA");
#elif defined(__APPLE__)
	// Suppose OSX only has CoreAudio...
	CfgReadStr(L"PORTAUDIO", L"HostApi", api, L"CoreAudio");
#else
	CfgReadStr(L"PORTAUDIO", L"HostApi", api, L"WASAPI");
#endif
	CfgReadStr(L"PORTAUDIO", L"Device", m_Device, L"default");

	SetApiSettings(api);

	m_WasapiExclusiveMode = CfgReadBool(L"PORTAUDIO", L"Wasapi_Exclusive_Mode", false);
	m_SuggestedLatencyMinimal = CfgReadBool(L"PORTAUDIO", L"Minimal_Suggested_Latency", true);
	m_SuggestedLatencyMS = CfgReadInt(L"PORTAUDIO", L"Manual_Suggested_Latency_MS", 20);

	if (m_SuggestedLatencyMS < 10)
		m_SuggestedLatencyMS = 10;
	if (m_SuggestedLatencyMS > 200)
		m_SuggestedLatencyMS = 200;
}

void PortaudioOutModule::SetApiSettings(wxString api)
{
	m_ApiId = -1;
	if (api == L"InDevelopment")
		m_ApiId = paInDevelopment; /* use while developing support for a new host API */
	if (api == L"DirectSound")
		m_ApiId = paDirectSound;
	if (api == L"MME")
		m_ApiId = paMME;
	if (api == L"ASIO")
		m_ApiId = paASIO;
	if (api == L"SoundManager")
		m_ApiId = paSoundManager;
	if (api == L"CoreAudio")
		m_ApiId = paCoreAudio;
	if (api == L"OSS")
		m_ApiId = paOSS;
	if (api == L"ALSA")
		m_ApiId = paALSA;
	if (api == L"AL")
		m_ApiId = paAL;
	if (api == L"BeOS")
		m_ApiId = paBeOS;
	if (api == L"WDMKS")
		m_ApiId = paWDMKS;
	if (api == L"JACK")
		m_ApiId = paJACK;
	if (api == L"WASAPI")
		m_ApiId = paWASAPI;
	if (api == L"AudioScienceHPI")
		m_ApiId = paAudioScienceHPI;
}

void PortaudioOutModule::WriteSettings() const
{
	wxString api;
	switch (m_ApiId)
	{
		case paInDevelopment:
			api = L"InDevelopment";
			break; /* use while developing support for a new host API */
		case paDirectSound:
			api = L"DirectSound";
			break;
		case paMME:
			api = L"MME";
			break;
		case paASIO:
			api = L"ASIO";
			break;
		case paSoundManager:
			api = L"SoundManager";
			break;
		case paCoreAudio:
			api = L"CoreAudio";
			break;
		case paOSS:
			api = L"OSS";
			break;
		case paALSA:
			api = L"ALSA";
			break;
		case paAL:
			api = L"AL";
			break;
		case paBeOS:
			api = L"BeOS";
			break;
		case paWDMKS:
			api = L"WDMKS";
			break;
		case paJACK:
			api = L"JACK";
			break;
		case paWASAPI:
			api = L"WASAPI";
			break;
		case paAudioScienceHPI:
			api = L"AudioScienceHPI";
			break;
		default:
			api = L"Unknown";
	}

	CfgWriteStr(L"PORTAUDIO", L"HostApi", api);
	CfgWriteStr(L"PORTAUDIO", L"Device", m_Device);

	CfgWriteBool(L"PORTAUDIO", L"Wasapi_Exclusive_Mode", m_WasapiExclusiveMode);
	CfgWriteBool(L"PORTAUDIO", L"Minimal_Suggested_Latency", m_SuggestedLatencyMinimal);
	CfgWriteInt(L"PORTAUDIO", L"Manual_Suggested_Latency_MS", m_SuggestedLatencyMS);
}


int PaCallback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	PortaudioOutModule* pa = static_cast<PortaudioOutModule*>(PortaudioOut);
	return pa->ActualPaCallback->ReadSamples(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags, userData);
}
