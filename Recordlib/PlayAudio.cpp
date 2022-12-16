#include "PlayAudio.h"

PlayAudio::PlayAudio()
{
		waveform.wFormatTag = WAVE_FORMAT_PCM;				// ������Ƶ�ĸ�ʽ
		waveform.nSamplesPerSec = 8000;					// ������
		waveform.wBitsPerSample = 16;						// ������Ƶ���ֽ�,���
		waveform.nChannels = 1;								// ����
		waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // �����
		waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // ��������
		waveform.cbSize = 0;

		wait = CreateEvent(NULL, 0, 0, NULL);
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);		// WAVE_MAPPER:���ŵ��豸id -1ΪĬ���豸
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
//	//w д  b ������
//	fopen_s(&file, FileName, "rb");
//	buf = new char[1024 * 1024 * 4];
//	cnt = fread(buf, sizeof(char), 1024 * 1024 * 4, file);
//}

void PlayAudio::InitFile(const char* FileName , DWORD Size)
{
	//w д  b ������
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

//����Ƶ���ж�����������������
LPSTR PlayAudio::ReadFile()
{
	return buf + *dolength;
}

size_t PlayAudio::ReadFileSize()
{
	return *playsize;
}

//ּ�ڴ�InitFile �ļ��в�������
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

//ּ�ڴ��ֽ����в�������
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
