#pragma once

#ifndef PLAYAUDIO_H
#define PLAYAUDIO_H

#include <Windows.h>
#include <iostream>
#include <functional>

#ifndef EXTERNAL_LINK_WINMMLIB
#pragma comment(lib, "winmm.lib") 
#endif // !EXTERNAL_LINK_WINMMLIB

class PlayAudio {
public:
    /// <summary>
    /// 开启播放设备时触发
    /// </summary>
    using OpenPlayDeviceEvent = std::function<void()>;
    /// <summary>
    /// 当前缓冲区的数据播放完成调用的事件
    /// </summary>
    using PlayDoneEvent = std::function<void()>;
    /// <summary>
    /// 表示需要写入数据，AB缓冲区,A缓冲区播放完成了，现在开始播放B缓冲区,此时回调用此事件让用户向A缓冲区中写入数据
    /// </summary>
    using NeedWriteDataEvent = std::function<bool(LPSTR,int*)>;
    /// <summary>
    /// 播放设备被关闭了 后触发
    /// </summary>
    using ClosePlayDeviceEvent = std::function<void()>;
public:
    PlayAudio();
    ~PlayAudio();
    /// <summary>
    /// 初始化
    /// </summary>
    void Init();
    /// <summary>
    /// 立即关闭 如果要接着使用这个类 得重新初始化
    /// </summary>
    void Close();
    /// <summary>
    /// 在本次缓冲区播放完成之后关闭
    /// </summary>
    void CloseInPlayDoneAfter();

    // 暂停播放
    void Stop();
    // 初始化后播放和暂停后播放
    void Play();
    // 是否是暂停播放状态
    bool IsPause();
    // 设置缓冲区大小
    void setBuffSize(int size);
    // 获取缓冲区大小
    int getBuffSize();
    // 设置播放的音频的格式 默认:WAVE_FORMAT_PCM
    void setFormatTag(WORD);
    // 设置采样率 默认:44100
    void setSamplesPerSec(DWORD);
    // 设置录制音频的精度 默认:16
    void setBitsPerSample(WORD);
    /// <summary>
    /// 向输出流中写入音频数据
    /// </summary>
    /// <param name="data">数据</param>
    /// <param name="size">数据中包含音频字节的大小</param>
    /// <returns>成功写入的数据大小</returns>
    int WriteAudioData(LPSTR data, int size);
    //设置当前使用的麦克风设备 from Num
    void setDevive(UINT);
    //获取当前使用的麦克风设备
    WAVEOUTCAPS getCurrentDevice();
    //获取当前使用的麦克风设备的编号
    UINT getCurrentDeviceNum();
public:
    // 获取所有播放设备
    static std::vector<WAVEOUTCAPS> GetAllDevs();
    /**
     * @brief 获取波形输出设备的数量
     * @return 
     */
    static int GetDevsNum();
    /**
     * @brief 获取一个指定编号的麦克风对象
     * @param id
     * @return
     */
    static WAVEOUTCAPS GetDevsFromId(UINT id);

private:
    static void CALLBACK callback(
        HWAVEOUT  hwo,								// 设备句柄
        UINT      uMsg,								// 消息
        DWORD_PTR dwInstance,						// 对象
        DWORD_PTR dwParam1,							// 参数1
        DWORD_PTR dwParam2);						// 参数2
    void WaveOutProcess(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
private:
    HWAVEOUT hWaveOut{};							// 打开的一个音频输出设备
    WAVEFORMATEX waveform{};
    WAVEHDR wHdr1{};

    char* pBufferA = nullptr;						// 要播放的音频字节缓存处 缓冲区A
    char* pBufferB = nullptr;						// 要播放的音频字节缓存处 缓冲区B
    bool isInit = false;							// 是否初始化
    bool isPause = false;							// 正在播放
    bool isreset = false;

    int buffsize = 1024 * 300;						// 播放缓存大小 播放长度(比如播放5s，但是buf的数据只够播放2s,之后的数据是0,那么将播放2s后没有声音,直到再过3s后调用callback)
    int buffNUM = 0;
    UINT currentDeviceNum = WAVE_MAPPER;			// 使用设备的编号
public:
    void onOpenPlayDevice(OpenPlayDeviceEvent e);
    void onPlayDone(PlayDoneEvent e);
    void onNeedWriteData(NeedWriteDataEvent e);
    void onClosePlayDevice(ClosePlayDeviceEvent e);

private:
    OpenPlayDeviceEvent OpenPlayDevice = NULL;
    PlayDoneEvent PlayDone = NULL;
    NeedWriteDataEvent NeedWriteData = NULL;
    ClosePlayDeviceEvent ClosePlayDevice = NULL;
};

#endif // !PLAYAUDIO_H
