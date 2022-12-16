#include "PlayAudio.h"

PlayAudio::PlayAudio()
{
		waveform.wFormatTag = WAVE_FORMAT_PCM;				// 播放音频的格式
		waveform.nSamplesPerSec = 8000;					// 采样率
		waveform.wBitsPerSample = 16;						// 播放音频的字节,深度
		waveform.nChannels = 1;								// 声道
		waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // 块对齐
		waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // 传输速率
		waveform.cbSize = 0;

		wait = CreateEvent(NULL, 0, 0, NULL);
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);		// WAVE_MAPPER:播放的设备id -1为默认设备
}

PlayAudio::~PlayAudio()
{
	waveOutClose(hWaveOut);
}

bool PlayAudio::HaveLoop()
{
	return cnt > 0;
}

//void PlayAudio::InitFile(const char* FileName)
//{
//	//w 写  b 二进制
//	fopen_s(&file, FileName, "rb");
//	buf = new char[1024 * 1024 * 4];
//	cnt = fread(buf, sizeof(char), 1024 * 1024 * 4, file);
//}

void PlayAudio::InitFile(const char* FileName , DWORD Size)
{
	//w 写  b 二进制
	fopen_s(&file, FileName, "rb");
	buf = new char[Size];
	dolength = new int(0);
	playsize = new int(1024);

	cnt = fread(buf, sizeof(char), Size, file);
}

int PlayAudio::CloseFile()
{
	delete[] buf;
	delete playsize;
	delete dolength;
	waveOutClose(hWaveOut);
	return fclose(file);
}

//从音频流中读接下来的声音数据
LPSTR PlayAudio::ReadFile()
{
	return buf + *dolength;
}

size_t PlayAudio::ReadFileSize()
{
	return *playsize;
}

//旨在从InitFile 文件中播放声音
void PlayAudio::Play(LPSTR PlayData)
{
	wHdr1.lpData = PlayData;
	wHdr1.dwBufferLength = *playsize;
	wHdr1.dwFlags = 0;
	wHdr1.dwLoops = 1L;

	waveOutPrepareHeader(hWaveOut, &wHdr1, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wHdr1, sizeof(WAVEHDR));
	WaitForSingleObject(wait, INFINITE);
}

//旨在从字节流中播放声音
void PlayAudio::Play(LPSTR PlayData, size_t PlaySize)
{
	wHdr1.lpData = PlayData;
	wHdr1.dwBufferLength = PlaySize;
	wHdr1.dwFlags = 0;
	wHdr1.dwLoops = 1L;

	waveOutPrepareHeader(hWaveOut, &wHdr1, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wHdr1, sizeof(WAVEHDR));
	WaitForSingleObject(wait, INFINITE);
}

void PlayAudio::ReadFileEnd()
{
	*dolength = *dolength + *playsize;
	cnt = cnt - *playsize;
}
