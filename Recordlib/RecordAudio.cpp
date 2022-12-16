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
	waveform.nSamplesPerSec = 8000;						// 采样率,决定了音质和录制后数据的大小
	waveform.wBitsPerSample = 16;						// 录制音频的字节,深度,精度
	waveform.nChannels = 1;								// 声道个数 1、2
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
	//wait = CreateEvent(NULL, 0, 0, NULL);
	if (IsInit) {
		return ReInit;
	}
	InitError ret = (InitError)waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)(&callback), (DWORD_PTR)this, CALLBACK_FUNCTION);		// WAVE_MAPPER:录制的麦克风id -1为默认麦克风
	if (ret == NoError) {
		IsInit = true;
	}
	return ret;
}

void RecordAudio::Resize(int NewSize)
{
	bufsize = NewSize;
}

void RecordAudio::Record()
{
	if (Recording) {
		return;
	}
	Recording = true;
	BYTE* pBuffer1 = new BYTE[bufsize];
	wHdr1.lpData = (LPSTR)pBuffer1;
	wHdr1.dwBufferLength = bufsize;
	wHdr1.dwBytesRecorded = 0;
	wHdr1.dwFlags = 0;
	wHdr1.dwLoops = 1;

	waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));

	waveInStart(hWaveIn);		//表示开始录制了
	
}
//
//BYTE* RecordAudio::Record(int RecordTime_ms, DWORD NewSize)
//{
//	BYTE* pBuffer1 = new BYTE[NewSize];
//	wHdr1.lpData = (LPSTR)pBuffer1;
//	wHdr1.dwBufferLength = NewSize;			// buff 大小
//	wHdr1.dwBytesRecorded = 0;				// buff 存放大小?
//	wHdr1.dwFlags = 0;
//	wHdr1.dwLoops = 1;
//
//	waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));
//	waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));
//
//	waveInStart(hWaveIn);
//	Sleep(RecordTime_ms);
//	waveInReset(hWaveIn);
//
//	return pBuffer1;
//}
//
//size_t RecordAudio::RecordSize()
//{
//	return wHdr1.dwBytesRecorded;
//}

void RecordAudio::Stop()
{
	if (!Recording) {
		return;
	}
	Recording = false;
	waveInReset(hWaveIn);
	delete[] pBuffer1;
	pBuffer1 = nullptr;
}

void RecordAudio::Close()
{
	if (Recording) {
		Stop();
	}
	waveInClose(hWaveIn);
}

bool RecordAudio::IsRecording()
{
	return Recording;
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
	WAVEHDR& pwhdr = _this->wHdr1;
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
		DWORD bytrecd = pwhdr.dwBytesRecorded;	// 距离上传缓冲区已满到这次录制的音频的大小

		// 这里再调用一次自定义函数 将录制的音频数据传递出去
		// 音频数据:pwhdr->lpData， 音频的大小: bytrecd
		if(_this->HasBufferStream) _this->HasBufferStream(pwhdr.lpData, bytrecd);

		if (_this->Recording) {
			waveInAddBuffer(hwi, &pwhdr, sizeof(WAVEHDR));
		}
		break;
	}
	case WIM_CLOSE:     // 关闭录音设备
	{
		if(_this->StopRecording) _this->StopRecording();
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
void RecordAudio::onStopRecordingEvent(StopRecordingEvent e)
{
	StopRecording = e;
}
