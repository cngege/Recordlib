#include "RecordAudio.h"

RecordAudio::RecordAudio()
{
	waveform.wFormatTag = WAVE_FORMAT_PCM;				// ¼����Ƶ�ĸ�ʽ
	waveform.nSamplesPerSec = 8000;						// ��Ƶ��HZ,���������ʺ�¼�ƺ����ݵĴ�С
	waveform.wBitsPerSample = 16;						// ¼����Ƶ���ֽ�,���
	waveform.nChannels = 1;								// ����
	waveform.nAvgBytesPerSec = 16000;
	waveform.nBlockAlign = 2;
	waveform.cbSize = 0;

	wait = CreateEvent(NULL, 0, 0, NULL);
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);		// WAVE_MAPPER:¼�Ƶ���˷�id -1ΪĬ����˷�
}

RecordAudio::~RecordAudio()
{
	waveInClose(hWaveIn);
}

void RecordAudio::Resize(size_t NewSize)
{
	bufsize = NewSize;
}

BYTE* RecordAudio::Record()
{
	BYTE* pBuffer1 = new BYTE[bufsize];
	wHdr1.lpData = (LPSTR)pBuffer1;
	wHdr1.dwBufferLength = bufsize;
	wHdr1.dwBytesRecorded = 0;
	wHdr1.dwFlags = 0;
	wHdr1.dwLoops = 1;

	waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));

	waveInStart(hWaveIn);
	Sleep(1000);
	waveInReset(hWaveIn);
	
	return pBuffer1;
}

void RecordAudio::InitFile(const char* Path)
{
	//w д  b ������
	fopen_s(&file, Path, "wb");
}

void RecordAudio::WriteInFile(BYTE* Record)
{
	fwrite(Record, 1, wHdr1.dwBytesRecorded, file);
	delete[] Record;
}

int RecordAudio::CloseFile()
{
	waveInClose(hWaveIn);
	return fclose(file);
}


