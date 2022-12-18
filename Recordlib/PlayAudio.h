#pragma once
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib") 

class PlayAudio {
public:
	/// <summary>
	/// 开启播放设备时触发
	/// </summary>
	using OpenPlayDeviceEvent = void(__fastcall*)();
	/// <summary>
	/// 当前缓冲区的数据播放完成调用的事件
	/// </summary>
	using PlayDoneEvent = void(__fastcall*)();
	/// <summary>
	/// 表示需要写入数据，AB缓冲区,A缓冲区播放完成了，现在开始播放B缓冲区,此时回调用此事件让用户向A缓冲区中写入数据
	/// </summary>
	using NeedWriteDataEvent = bool(__fastcall*)(LPSTR,int*);
	/// <summary>
	/// 播放设备被关闭了 后触发
	/// </summary>
	using StopPlayEvent = void(__fastcall*)();
public:
	PlayAudio();
	~PlayAudio();

	void Init();

	/// <summary>
	/// 立即关闭
	/// </summary>
	void Close();
	/// <summary>
	/// 在本次缓冲区播放完成之后关闭
	/// </summary>
	void CloseInPlayDoneAfter();

	void Stop();
	void Play();
	bool IsPause();
	void SetBuffSize(int size);
	int GetBuffSize();

	//bool HaveLoop();
	////void InitFile(const char* FileName);
	//void InitFile(const char* FileName, DWORD Size = 1024 * 1024 * 4);
	//int CloseFile();

	//LPSTR ReadFile();
	//size_t ReadFileSize();
	//void Play(LPSTR PlayData);
	//void Play(LPSTR PlayData, size_t PlaySize);
	//void ReadFileEnd();
private:
	static void CALLBACK callback(
		HWAVEOUT  hwo,							   // 设备句柄
		UINT      uMsg,							   // 消息
		DWORD_PTR dwInstance,					   // 对象
		DWORD_PTR dwParam1,						   // 参数1
		DWORD_PTR dwParam2);					   // 参数2
	void WaveOutProcess(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
public:


	bool isloop = false;
	
private:
	HWAVEOUT hWaveOut;				// 打开的一个音频输出设备
	WAVEFORMATEX waveform{};
	WAVEHDR wHdr1{};

	char* pBufferA = nullptr;		// 要播放的音频字节缓存处 缓冲区A
	char* pBufferB = nullptr;		// 要播放的音频字节缓存处 缓冲区B
	bool isInit = false;			// 是否初始化
	bool isPause = false;			// 正在播放
	bool isreset = false;

	int buffsize = 1024 * 1000;		// 播放缓存大小 播放长度(比如播放5s，但是buf的数据只够播放2s,之后的数据是0,那么将播放2s后没有声音,直到再过3s后调用callback)
	int buffNUM = 0;

public:
	void onOpenPlayDevice(OpenPlayDeviceEvent e);
	void onPlayDone(PlayDoneEvent e);
	void onNeedWriteData(NeedWriteDataEvent e);
	void onStopPlay(StopPlayEvent e);

private:
	OpenPlayDeviceEvent OpenPlayDevice = NULL;
	PlayDoneEvent PlayDone = NULL;
	NeedWriteDataEvent NeedWriteData = NULL;
	StopPlayEvent StopPlay = NULL;
};