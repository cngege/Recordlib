#pragma once
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib") 

class PlayAudio {
public:
	/// <summary>
	/// ���������豸ʱ����
	/// </summary>
	using OpenPlayDeviceEvent = void(__fastcall*)();
	/// <summary>
	/// ��ǰ�����������ݲ�����ɵ��õ��¼�
	/// </summary>
	using PlayDoneEvent = void(__fastcall*)();
	/// <summary>
	/// ��ʾ��Ҫд�����ݣ�AB������,A��������������ˣ����ڿ�ʼ����B������,��ʱ�ص��ô��¼����û���A��������д������
	/// </summary>
	using NeedWriteDataEvent = bool(__fastcall*)(LPSTR,int*);
	/// <summary>
	/// �����豸���ر��� �󴥷�
	/// </summary>
	using StopPlayEvent = void(__fastcall*)();
public:
	PlayAudio();
	~PlayAudio();

	void Init();

	/// <summary>
	/// �����ر�
	/// </summary>
	void Close();
	/// <summary>
	/// �ڱ��λ������������֮��ر�
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
		HWAVEOUT  hwo,							   // �豸���
		UINT      uMsg,							   // ��Ϣ
		DWORD_PTR dwInstance,					   // ����
		DWORD_PTR dwParam1,						   // ����1
		DWORD_PTR dwParam2);					   // ����2
	void WaveOutProcess(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
public:


	bool isloop = false;
	
private:
	HWAVEOUT hWaveOut;				// �򿪵�һ����Ƶ����豸
	WAVEFORMATEX waveform{};
	WAVEHDR wHdr1{};

	char* pBufferA = nullptr;		// Ҫ���ŵ���Ƶ�ֽڻ��洦 ������A
	char* pBufferB = nullptr;		// Ҫ���ŵ���Ƶ�ֽڻ��洦 ������B
	bool isInit = false;			// �Ƿ��ʼ��
	bool isPause = false;			// ���ڲ���
	bool isreset = false;

	int buffsize = 1024 * 1000;		// ���Ż����С ���ų���(���粥��5s������buf������ֻ������2s,֮���������0,��ô������2s��û������,ֱ���ٹ�3s�����callback)
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