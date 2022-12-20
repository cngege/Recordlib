#pragma once
#include <Windows.h>
#include <iostream>
#include <functional>
#pragma comment(lib, "winmm.lib") 

class PlayAudio {
public:
	/// <summary>
	/// ���������豸ʱ����
	/// </summary>
	using OpenPlayDeviceEvent = std::function<void()>;
	/// <summary>
	/// ��ǰ�����������ݲ�����ɵ��õ��¼�
	/// </summary>
	using PlayDoneEvent = std::function<void()>;
	/// <summary>
	/// ��ʾ��Ҫд�����ݣ�AB������,A��������������ˣ����ڿ�ʼ����B������,��ʱ�ص��ô��¼����û���A��������д������
	/// </summary>
	using NeedWriteDataEvent = std::function<bool(LPSTR,int*)>;
	/// <summary>
	/// �����豸���ر��� �󴥷�
	/// </summary>
	using ClosePlayDeviceEvent = std::function<void()>;
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

	// ��ͣ����
	void Stop();
	// ��ʼ���󲥷ź���ͣ�󲥷�
	void Play();
	// �Ƿ�����ͣ����״̬
	bool IsPause();
	// ���û�������С
	void setBuffSize(int size);
	// ��ȡ��������С
	int getBuffSize();
	// ���ò��ŵ���Ƶ�ĸ�ʽ Ĭ��:WAVE_FORMAT_PCM
	void setFormatTag(WORD);
	// ���ò����� Ĭ��:44100
	void setSamplesPerSec(DWORD);
	// ����¼����Ƶ�ľ��� Ĭ��:16
	void setBitsPerSample(WORD);
	/// <summary>
	/// ���������д����Ƶ����
	/// </summary>
	/// <param name="data">����</param>
	/// <param name="size">�����а�����Ƶ�ֽڵĴ�С</param>
	/// <returns>�ɹ�д������ݴ�С</returns>
	int WriteAudioData(LPSTR data, int size);

private:
	static void CALLBACK callback(
		HWAVEOUT  hwo,							   // �豸���
		UINT      uMsg,							   // ��Ϣ
		DWORD_PTR dwInstance,					   // ����
		DWORD_PTR dwParam1,						   // ����1
		DWORD_PTR dwParam2);					   // ����2
	void WaveOutProcess(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
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
	void onClosePlayDevice(ClosePlayDeviceEvent e);

private:
	OpenPlayDeviceEvent OpenPlayDevice = NULL;
	PlayDoneEvent PlayDone = NULL;
	NeedWriteDataEvent NeedWriteData = NULL;
	ClosePlayDeviceEvent ClosePlayDevice = NULL;
};