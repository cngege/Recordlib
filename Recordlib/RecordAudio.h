#pragma once
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib") 

class RecordAudio {
public:
	RecordAudio();
	~RecordAudio();
	void Resize(size_t);
	BYTE* Record();					// 每次调用录制一秒钟,将录制的数据存储在缓存中
	size_t RecordSize();
	void InitFile(const char* Path);
	void WriteInFile(BYTE* Record);
	int CloseFile();

private:
	HWAVEIN hWaveIn;				// 打开的一个音频输入设备
	WAVEFORMATEX waveform;			// 录制参数、规格 waveform.nChannels 0单声道 1双声道
	WAVEHDR wHdr1;
	BYTE* pBuffer1;					// 录制的音频字节缓存处
	HANDLE wait;					// 保存事件

	DWORD bufsize = 1024 * 1024;	// 默认缓冲大小
	FILE* file;
};