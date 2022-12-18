#include "RecordAudio.h"


enum RecordAudio::InitError : int {
	// 重复初始化
	ReInit = -1,
	// 工作正常 没有错误
	NoError = 0,
	// 设备id超界
	BadDeviceId = 2,
	// 指定的资源已被分配
	AllocAted = 4,
	// 没有安装驱动程序
	NoDriver = 6,
	// 不能分配或锁定内存
	NoMem = 7,
	// 设备不支持请求的波形格式
	BadFormat = 32
};

RecordAudio::RecordAudio()
{
	// 设置音频流格式
	waveform.wFormatTag = WAVE_FORMAT_PCM;				// 录制音频的格式
	waveform.nSamplesPerSec = 44100;					// 采样率,决定了音质和录制后数据的大小
	waveform.wBitsPerSample = 16;						// 录制音频的字节,深度,精度
	waveform.nChannels = 2;								// 声道个数 1、2
	waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // 块对齐
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // 传输速率
	waveform.cbSize = 0;								// 额外空间	

}

RecordAudio::~RecordAudio()
{
	if (pBuffer1) {
		Close();
	}
}

RecordAudio::InitError RecordAudio::Init()
{
	if (isInit) {
		return ReInit;
	}
	InitError ret = (InitError)waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)(&callback), (DWORD_PTR)this, CALLBACK_FUNCTION);		// WAVE_MAPPER:录制的麦克风id -1为默认麦克风
	if (ret == NoError) {
		isInit = true;
		BYTE* pBuffer1 = new BYTE[bufsize];
		BYTE* pBuffer2 = new BYTE[bufsize];
		memset(pBuffer1, 0, bufsize);
		memset(pBuffer2, 0, bufsize);
		wHdr1.lpData = (LPSTR)pBuffer1;
		wHdr1.dwBufferLength = bufsize;
		wHdr1.dwBytesRecorded = 0;				// 在回调中此值表示已经录制的音频大小
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		wHdr2.lpData = (LPSTR)pBuffer2;
		wHdr2.dwBufferLength = bufsize;
		wHdr2.dwBytesRecorded = 0;				// 在回调中此值表示已经录制的音频大小
		wHdr2.dwFlags = 0;
		wHdr2.dwLoops = 1;
	}
	return ret;
}



void RecordAudio::Record()
{
	if (!isInit) {
		return;
	}
	if (Recording) {
		return;
	}
	Recording = true;
	waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));
	waveInPrepareHeader(hWaveIn, &wHdr2, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &wHdr2, sizeof(WAVEHDR));

	waveInStart(hWaveIn);		//表示开始录制了
	
}

void RecordAudio::Stop()
{
	if (!Recording) {
		return;
	}
	Recording = false;
	waveInReset(hWaveIn); // 调用一下waveInReset，这样可以清掉尚在等待录音的缓冲区
}

void RecordAudio::Close()
{
	if (Recording) {
		Stop();
	}
	if (isInit) {
		delete[] pBuffer1;
		pBuffer1 = nullptr;
		delete[] pBuffer2;
		pBuffer2 = nullptr;
	}
	waveInClose(hWaveIn);
}

bool RecordAudio::IsRecording()
{
	return Recording;
}

void RecordAudio::setBuffsize(int NewSize)
{
	bufsize = NewSize;
}

void RecordAudio::setFormatTag(WORD tag)
{
	if (isInit) return;
	waveform.wFormatTag = tag;
}

void RecordAudio::setSamplesPerSec(DWORD samplesPerSec)
{
	if (isInit) return;
	waveform.nSamplesPerSec = samplesPerSec;
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;
}

void RecordAudio::setBitsPerSample(WORD bit)
{
	if (isInit) return;
	waveform.wBitsPerSample = bit;
	waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // 块对齐
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // 传输速率

}

void CALLBACK RecordAudio::callback(HWAVEIN   hwi,                              // 设备句柄
	UINT      uMsg,							   // 消息
	DWORD_PTR dwInstance,					   // 对象 this
	DWORD_PTR dwParam1,						   // 参数1
	DWORD_PTR dwParam2)						   // 参数2
{
	((RecordAudio*)dwInstance)->WaveInProcess(hwi, uMsg, dwInstance, dwParam1, dwParam2);
}
void RecordAudio::WaveInProcess(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	// 当前对象
	RecordAudio* _this = (RecordAudio*)dwInstance;
	// 获取音频头
	PWAVEHDR pwhdr = (PWAVEHDR)dwParam1;
	// 处理消息
	switch (uMsg)
	{
	case WIM_OPEN:      // 打开录音设备
	{
		//printf("成功打开设备..\n");		// 这里调用一个自定义事件
		if (_this->OpenRecordDevice) _this->OpenRecordDevice();
		break;
	}
	case WIM_DATA:      // 缓冲区已满 表示我已经录制了指定的大小的音频了
	{
		//这里扩充缓冲区
		DWORD bytrecd = pwhdr->dwBytesRecorded;	// 距离上传缓冲区已满到这次录制的音频的大小

		// 这里再调用一次自定义函数 将录制的音频数据传递出去
		// 音频数据:pwhdr->lpData， 音频的大小: bytrecd
		if(_this->HasBufferStream) _this->HasBufferStream(pwhdr->lpData, bytrecd);

		if (_this->Recording) {
			waveInAddBuffer(hwi, pwhdr, sizeof(WAVEHDR));
		}
		break;
	}
	case WIM_CLOSE:     // 关闭录音设备
	{
		if(_this->CloseRecordDevice) _this->CloseRecordDevice();
		break;
	}
	default:
		break;
	}
}
void RecordAudio::onOpenRecordDeviceEvent(OpenRecordDeviceEvent e)
{
	OpenRecordDevice = e;
}
void RecordAudio::onHasBufferStreamEvent(HasBufferStreamEvent e)
{
	HasBufferStream = e;
}
void RecordAudio::onCloseRecordDeviceEvent(CloseRecordDeviceEvent e)
{
	CloseRecordDevice = e;
}
