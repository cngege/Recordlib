#pragma once
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib") 

class PlayAudio {
public:
	PlayAudio();
	~PlayAudio();

	bool HaveLoop();
	//void InitFile(const char* FileName);
	void InitFile(const char* FileName, DWORD Size = 1024 * 1024 * 4);
	int CloseFile();

	LPSTR ReadFile();
	size_t ReadFileSize();
	void Play(LPSTR PlayData);
	void Play(LPSTR PlayData, size_t PlaySize);
	void ReadFileEnd();
public:
	HWAVEOUT hWaveOut;				// �򿪵�һ����Ƶ����豸
	WAVEFORMATEX waveform;
	WAVEHDR wHdr1;
	BYTE* pBuffer1;					// ¼�Ƶ���Ƶ�ֽڻ��洦
	HANDLE wait;					// �����¼�
	
	FILE* file;
	char* buf;
	int cnt;
	int* dolength;
	int* playsize;
};