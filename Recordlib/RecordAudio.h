// CNGEGE
// 运行逻辑: 首先实例化类, 然后传入录制参数
// 运行Record() 自动创建线程开始录制 并将状态Recording 设置为true
// 运行stop()停止录制 自动将 Recording 设置为false 这时还可以运行 Record()接着录制
// 运行Close 关闭并销毁,释放指针，表示这个实例不再进行录制了


#pragma once
#include <Windows.h>
#include <iostream>
#include <thread>
#include <functional>
#pragma comment(lib, "winmm.lib") 

class RecordAudio {
public:
	enum InitError : int;
public:
	/// <summary>
	/// 成功打开录音设备时触发
	/// </summary>
	using OpenRecordDeviceEvent = void(__fastcall*)();
	/// <summary>
	/// 当录制的数据撑满了缓冲区时触发,它告诉前端用户赶紧将录制的数据拿走,接下来的录制将覆盖掉这部分的数据
	/// </summary>
	using HasBufferStreamEvent = void(__fastcall*)(LPSTR, DWORD);
	/// <summary>
	/// 由设备事件发出的停止录音消息,一般在用户手动调用Stop()或Close() 后触发
	/// </summary>
	using StopRecordingEvent = void(__fastcall*)();
public:
	RecordAudio();
	~RecordAudio();
	/// <summary>
	/// 初始化 在初始化前 设置好录制参数 和回调事件
	/// </summary>
	/// <returns>初始化时是否发生错误</returns>
	InitError Init();
	void Resize(int);
	void Record();
	void Stop();
	void Close();
	bool IsRecording();
private:
	static void CALLBACK callback(
		HWAVEIN   hwi,							   // 设备句柄
		UINT      uMsg,							   // 消息
		DWORD_PTR dwInstance,					   // 对象
		DWORD_PTR dwParam1,						   // 参数1
		DWORD_PTR dwParam2);					   // 参数2
	void WaveInProcess(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

private:
	HWAVEIN hWaveIn{};				// 打开的一个音频输入设备
	WAVEFORMATEX waveform{};		// 录制参数、规格 waveform.nChannels 声道个数
	WAVEHDR wHdr1{};
	BYTE* pBuffer1 = nullptr;		// 录制的音频字节缓存处
	DWORD bufsize = 1024 * 15;		// 默认缓冲大小
	bool Recording = false;			// 是否正在录制中
	bool IsInit = false;			// 是否初始化过了

private:
	/// <summary>
	/// 当成功打开录音设备时调用
	/// </summary>
	OpenRecordDeviceEvent OpenRecordDevice = NULL;
	/// <summary>
	/// 当录制的数据撑满了缓冲区时触发,它告诉前端用户赶紧将录制的数据拿走,接下来的录制将覆盖掉这部分的数据
	/// </summary>
	HasBufferStreamEvent HasBufferStream = NULL;
	/// <summary>
	/// 由设备事件发出的停止录音消息,一般在用户手动调用Stop()或Close() 后触发
	/// </summary>
	StopRecordingEvent StopRecording = NULL;

public:
	/// <summary>
	/// 当成功打开录音设备时调用
	/// </summary>
	/// <param name=""></param>
	void onOpenRecordDeviceEvent(OpenRecordDeviceEvent);
	/// <summary>
	/// 当录制的数据撑满了缓冲区时触发,它告诉前端用户赶紧将录制的数据拿走,接下来的录制将覆盖掉这部分的数据
	/// </summary>
	/// <param name=""></param>
	void onHasBufferStreamEvent(HasBufferStreamEvent);
	/// <summary>
	/// 由设备事件发出的停止录音消息,一般在用户手动调用Stop()或Close() 后触发
	/// </summary>
	/// <param name=""></param>
	void onStopRecordingEvent(StopRecordingEvent);

};