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
	if (isInit) return;
	pBufferA = new char[(size_t)buffsize + 4];
	pBufferB = new char[(size_t)buffsize + 4];
	memset(pBufferA, 0, (size_t)buffsize + 4);
	memset(pBufferB, 0, (size_t)buffsize + 4);
	waveOutOpen(&hWaveOut, currentDeviceNum, &waveform, (DWORD_PTR)callback, (DWORD_PTR)this, CALLBACK_FUNCTION);		// WAVE_MAPPER:播放的设备id -1为默认设备
	isInit = true;
}

void PlayAudio::Close()
{
	if (!isInit) {
		return;
	}
	isInit = false;
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
	}
	//*(int*)pBufferA = buffsize;
	wHdr1.lpData = pBufferA + 4;
	wHdr1.dwBufferLength = *(int*)pBufferA;
	wHdr1.dwFlags = 0;
	wHdr1.dwLoops = 1L;
	waveOutPrepareHeader(hWaveOut, &wHdr1, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wHdr1, sizeof(WAVEHDR));
	buffNUM = 0;

	if (NeedWriteData != NULL) {
		NeedWriteData(pBufferB + 4, (int*)pBufferB);	// 向用户调用函数取回数据
		//*(int*)pBufferB = buffsize;
	}
}

bool PlayAudio::IsPause()
{
	return isPause;
}

void PlayAudio::setBuffSize(int size)
{
	if (isInit) return;
	buffsize = size;
}

int PlayAudio::getBuffSize()
{
	return buffsize;
}

void PlayAudio::setFormatTag(WORD tag)
{
	if (isInit) return;
	waveform.wFormatTag = tag;
}

void PlayAudio::setSamplesPerSec(DWORD samplesPerSec)
{
	if (isInit) return;
	waveform.nSamplesPerSec = samplesPerSec;
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;
}

void PlayAudio::setBitsPerSample(WORD bit)
{
	if (isInit) return;
	waveform.wBitsPerSample = bit;
	waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // 块对齐
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // 传输速率
}

int PlayAudio::WriteAudioData(LPSTR data, int size)
{
	if (isInit) {
		// 正在播放缓存A
		if (buffNUM == 0) {
			// 那么向缓存B写入
			// 判断缓存B是否可以写入
			int freesize = buffsize - *(int*)pBufferB;
			if (freesize > 0) {
				if (size > freesize) {
					// 将剩余的空间写满
					memcpy(pBufferB + 4 + *(int*)pBufferB, data, freesize);
					*(int*)pBufferB = *(int*)pBufferB + freesize;				// *(int*)pBufferB = buffsize;
					return freesize;
				}
				else {
					// 将数据全部写入
					memcpy(pBufferB + 4 + *(int*)pBufferB, data, size);
					*(int*)pBufferB = *(int*)pBufferB + size;
					return size;
				}
			}
		}
		else if (buffNUM == 1) {
			// 那么向缓存A写入
			// 判断缓存A是否可以写入
			int freesize = buffsize - *(int*)pBufferA;
			if (freesize > 0) {
				if (size > freesize) {
					// 将剩余的空间写满
					memcpy(pBufferA + 4 + *(int*)pBufferA, data, freesize);
					*(int*)pBufferA = *(int*)pBufferA + freesize;				// *(int*)pBufferB = buffsize;
					return freesize;
				}
				else {
					// 将数据全部写入
					memcpy(pBufferA + 4 + *(int*)pBufferA, data, size);
					*(int*)pBufferA = *(int*)pBufferA + size;
					return size;
				}
			}
		}
	}
	return 0;
}

void PlayAudio::setDevive(UINT id) {
	currentDeviceNum = id;
}

WAVEOUTCAPS PlayAudio::getCurrentDevice() {
	auto allDevs = GetAllDevs();
	return allDevs[currentDeviceNum];
}

UINT PlayAudio::getCurrentDeviceNum() {
	return currentDeviceNum;
}



// ----------- PUBLIC STATIC
#pragma region PUBLIC STATIC
std::vector<WAVEOUTCAPS> PlayAudio::GetAllDevs() {
	std::vector<WAVEOUTCAPS> devs;
	UINT devsnum = waveOutGetNumDevs();
	if(devsnum == 0) {
		return std::vector<WAVEOUTCAPS>();
	}
	else {
		for(UINT i = 0; i < devsnum; i++) {
			WAVEOUTCAPS waveOutcaps;
			MMRESULT _ = waveOutGetDevCaps(i, &waveOutcaps, sizeof(WAVEOUTCAPS));
			devs.push_back(waveOutcaps);
		}
		return devs;
	}
}

int PlayAudio::GetDevsNum() {
	return waveOutGetNumDevs();
}

WAVEOUTCAPS PlayAudio::GetDevsFromId(UINT id) {
	WAVEOUTCAPS waveOutcaps;
	MMRESULT _ = waveOutGetDevCaps(id, &waveOutcaps, sizeof(WAVEOUTCAPS));
	return waveOutcaps;
}

#pragma endregion

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
		if (OpenPlayDevice != NULL) OpenPlayDevice();
	}
	if (uMsg == WOM_DONE) {
		if (PlayDone != NULL) PlayDone();
		pWaveHeader->dwFlags = 0;
		if (!isreset) {
			//waveOutUnprepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));
			if (buffNUM == 0) {
				// 将自己的数据标签大小 置0
				memset(pBufferA, 0, (size_t)buffsize + 4);
				//*(int*)pBufferA = 0;
				// 表示当时播放完的数据流是 buffA
				// 首先播放buffB
				pWaveHeader->lpData = pBufferB + 4;
				if (*(int*)pBufferB == 0) *(int*)pBufferB = 1024 * 100;
				pWaveHeader->dwBufferLength = *(int*)pBufferB;
				pWaveHeader->dwFlags = 0;
				waveOutPrepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));
				waveOutWrite(hwo, pWaveHeader, sizeof(WAVEHDR));
				buffNUM = 1;

				if (NeedWriteData != NULL) {
					// 然后创建线程 读取字节缓存到buffA
					std::thread readbuff([_this]() {
						//memset(_this->pBufferA, 0, (size_t)_this->buffsize + 4);
						bool hasdata = _this->NeedWriteData(_this->pBufferA + 4, (int*)_this->pBufferA);
						if (!hasdata) {
							*(int*)_this->pBufferA = 0;
						}
						});
					readbuff.detach();
				}
			}
			else if (buffNUM == 1) {
				// 将刚刚播放完成的音频数据标签大小置0
				//*(int*)pBufferB = 0;
				memset(pBufferB, 0, (size_t)buffsize + 4);
				// 表示当时播放完的数据流是 buffB
				// 首先播放buffA
				pWaveHeader->lpData = pBufferA + 4;
				if (*(int*)pBufferA == 0) *(int*)pBufferA = 1024 * 100;
				pWaveHeader->dwBufferLength = *(int*)pBufferA;
				pWaveHeader->dwFlags = 0;
				waveOutPrepareHeader(hwo, pWaveHeader, sizeof(WAVEHDR));
				waveOutWrite(hwo, pWaveHeader, sizeof(WAVEHDR));
				buffNUM = 0;

				if (NeedWriteData != NULL) {
					// 然后创建线程 读取字节缓存到buffB
					std::thread readbuff([_this]() {
						memset(_this->pBufferB, 0, (size_t)_this->buffsize + 4);
						bool hasdata = _this->NeedWriteData(_this->pBufferB + 4, (int*)_this->pBufferB);
						if (!hasdata) {
							*(int*)_this->pBufferB = 0;
						}
						});
					readbuff.detach();
				}
			}
		}
	}
	if (uMsg == WOM_CLOSE) {
		//事件
		if (ClosePlayDevice != NULL) ClosePlayDevice();
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

void PlayAudio::onClosePlayDevice(ClosePlayDeviceEvent e)
{
	ClosePlayDevice = e;
}
