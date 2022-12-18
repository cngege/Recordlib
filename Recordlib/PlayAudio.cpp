#include "PlayAudio.h"
#include <thread>

PlayAudio::PlayAudio()
{
		waveform.wFormatTag = WAVE_FORMAT_PCM;				// 播放音频的格式
		waveform.nSamplesPerSec = 44100;					// 采样率
		waveform.wBitsPerSample = 16;						// 播放音频的字节,深度
		waveform.nChannels = 2;								// 声道
		waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // 块对齐
		waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // 传输速率 每秒的字节数
		waveform.cbSize = 0;
}

PlayAudio::~PlayAudio()
{
	Close();
}

void PlayAudio::Init()
{
	if (isInit) {
		return;
	}
	pBufferA = new char[(size_t)buffsize + 4];
	pBufferB = new char[(size_t)buffsize + 4];
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD_PTR)callback, (DWORD_PTR)this, CALLBACK_FUNCTION);		// WAVE_MAPPER:播放的设备id -1为默认设备
	isInit = true;
}

void PlayAudio::Close()
{
	if (!isInit) {
		return;
	}
	if (pBufferA == nullptr) {
		return;
	}
	isreset = true;
	waveOutReset(hWaveOut);
	isreset = false;
	waveOutClose(hWaveOut);
	delete[] pBufferA;
	pBufferA = nullptr;
	delete[] pBufferB;
	pBufferB = nullptr;
}

void PlayAudio::Stop()
{
	if (!isInit) {
		return;
	}
	if (!isPause) {
		waveOutPause(hWaveOut);
		isPause = true;
	}
}

void PlayAudio::Play()
{
	if (!isInit) {
		return;
	}
	if (isPause) {
		waveOutRestart(hWaveOut);
		isPause = false;
		return;
	}
	if (NeedWriteData != NULL) {
		NeedWriteData(pBufferA + 4, (int*)pBufferA);	// 向用户调用函数取回数据
		//*(int*)pBufferA = buffsize;
		wHdr1.lpData = pBufferA + 4;
		wHdr1.dwBufferLength = *(int*)pBufferA;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1L;
		waveOutPrepareHeader(hWaveOut, &wHdr1, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, &wHdr1, sizeof(WAVEHDR));
		buffNUM = 0;

		NeedWriteData(pBufferB + 4, (int*)pBufferB);	// 向用户调用函数取回数据
		//*(int*)pBufferB = buffsize;
	}
}

bool PlayAudio::IsPause()
{
	return isPause;
}

void PlayAudio::SetBuffSize(int size)
{
	if (isInit) {
		return;
	}
	buffsize = size;
}

int PlayAudio::GetBuffSize()
{
	return buffsize;
}

//bool PlayAudio::HaveLoop()
//{
//	return cnt > 0;
//}

//void PlayAudio::InitFile(const char* FileName)
//{
//	//w 写  b 二进制
//	fopen_s(&file, FileName, "rb");
//	buf = new char[1024 * 1024 * 4];
//	cnt = fread(buf, sizeof(char), 1024 * 1024 * 4, file);
//}

//void PlayAudio::InitFile(const char* FileName , DWORD Size)
//{
//	//w 写  b 二进制
//	fopen_s(&file, FileName, "rb");
//	buf = new char[Size];
//	dolength = new int(0);
//	//playsize = new int(10240*100);
//
//	cnt = fread(buf, sizeof(char), Size, file);
//}
//
//int PlayAudio::CloseFile()
//{
//	delete[] buf;
//	//delete playsize;
//	delete dolength;
//	waveOutClose(hWaveOut);
//	return fclose(file);
//}
//
////从音频流中读接下来的声音数据
//LPSTR PlayAudio::ReadFile()
//{
//	return buf + *dolength;
//}
//
//size_t PlayAudio::ReadFileSize()
//{
//	return buffsize;
//}
//
////旨在从InitFile 文件中播放声音
//void PlayAudio::Play(LPSTR PlayData)
//{
//	wHdr1.lpData = PlayData;
//	wHdr1.dwBufferLength = buffsize;
//	wHdr1.dwFlags = 0;
//	wHdr1.dwLoops = 1L;
//
//	waveOutPrepareHeader(hWaveOut, &wHdr1, sizeof(WAVEHDR));
//	waveOutWrite(hWaveOut, &wHdr1, sizeof(WAVEHDR));
//	//WaitForSingleObject(wait, INFINITE);
//}
//
////旨在从字节流中播放声音
//void PlayAudio::Play(LPSTR PlayData, size_t PlaySize)
//{
//	wHdr1.lpData = PlayData;
//	wHdr1.dwBufferLength = PlaySize;
//	wHdr1.dwFlags = 0;
//	wHdr1.dwLoops = 1L;
//
//	waveOutPrepareHeader(hWaveOut, &wHdr1, sizeof(WAVEHDR));
//	waveOutWrite(hWaveOut, &wHdr1, sizeof(WAVEHDR));
//	//WaitForSingleObject(wait, INFINITE);
//}
//
//void PlayAudio::ReadFileEnd()
//{
//	*dolength = *dolength + buffsize;
//	cnt = cnt - buffsize;
//}

void PlayAudio::callback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	((PlayAudio*)dwInstance)->WaveOutProcess(hwo, uMsg, dwInstance, dwParam1, dwParam2);
}

void PlayAudio::WaveOutProcess(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	PlayAudio* _this = (PlayAudio*)dwInstance;
	LPWAVEHDR pWaveHeader= (LPWAVEHDR)dwParam1;
	//MM_WOM_OPEN
	if (uMsg == WOM_OPEN) {
		//事件
	}
	if (uMsg == WOM_DONE) {
		if (PlayDone != NULL) PlayDone();
		pWaveHeader->dwFlags = 0;
		if (!isreset) {
			waveOutUnprepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));

			//此处填充WAVEHDR的lpdate缓冲
			if (NeedWriteData != NULL) {
				if (buffNUM == 0) {
					// 表示当时播放完的数据流是 buffA
					if (*(int*)pBufferB != 0) {
						// 首先播放buffB
						pWaveHeader->lpData = pBufferB + 4;
						pWaveHeader->dwBufferLength = *(int*)pBufferB;
						pWaveHeader->dwFlags = 0;
						waveOutPrepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));
						waveOutWrite(hwo, pWaveHeader, sizeof(WAVEHDR));
						buffNUM = 1;

						// 然后创建线程 读取字节缓存到buffA
						std::thread readbuff([_this]() {
							memset(_this->pBufferA, 0, (size_t)_this->buffsize + 4);
							bool hasdata = _this->NeedWriteData(_this->pBufferA + 4, (int*)_this->pBufferA);
							if (!hasdata) {
								*(int*)_this->pBufferA = 0;
							}
						});
						readbuff.detach();

					}// pBufferB 有效大小为0
				}
				else if(buffNUM == 1) {
					// 表示当时播放完的数据流是 buffB
					if (*(int*)pBufferA != 0) {
						// 首先播放buffA
						pWaveHeader->lpData = pBufferA + 4;
						pWaveHeader->dwBufferLength = *(int*)pBufferA;
						pWaveHeader->dwFlags = 0;
						waveOutPrepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));
						waveOutWrite(hwo, pWaveHeader, sizeof(WAVEHDR));
						buffNUM = 0;

						// 然后创建线程 读取字节缓存到buffB
						std::thread readbuff([_this]() {
							memset(_this->pBufferB, 0, (size_t)_this->buffsize + 4);
							bool hasdata = _this->NeedWriteData(_this->pBufferB + 4, (int*)_this->pBufferB);
							if (!hasdata) {
								*(int*)_this->pBufferB = 0;
							}
							});
						readbuff.detach();
					}// pBufferB 有效大小为0
				}
			}
		}
	}
	if (uMsg == WOM_CLOSE) {
		//事件
	}
}


void PlayAudio::onOpenPlayDevice(OpenPlayDeviceEvent e)
{
	OpenPlayDevice = e;
}

void PlayAudio::onPlayDone(PlayDoneEvent e)
{
	PlayDone = e;
}

void PlayAudio::onNeedWriteData(NeedWriteDataEvent e)
{
	NeedWriteData = e;
}

void PlayAudio::onStopPlay(StopPlayEvent e)
{
	StopPlay = e;
}
