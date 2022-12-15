#pragma once
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib") 

class RecordAudio {
public:
	RecordAudio();
	~RecordAudio();
	void Resize(size_t);
	BYTE* Record();					// ÿ�ε���¼��һ����,��¼�Ƶ����ݴ洢�ڻ�����
	size_t RecordSize();
	void InitFile(const char* Path);
	void WriteInFile(BYTE* Record);
	int CloseFile();

private:
	HWAVEIN hWaveIn;				// �򿪵�һ����Ƶ�����豸
	WAVEFORMATEX waveform;			// ¼�Ʋ�������� waveform.nChannels 0������ 1˫����
	WAVEHDR wHdr1;
	BYTE* pBuffer1;					// ¼�Ƶ���Ƶ�ֽڻ��洦
	HANDLE wait;					// �����¼�

	DWORD bufsize = 1024 * 1024;	// Ĭ�ϻ����С
	FILE* file;
};