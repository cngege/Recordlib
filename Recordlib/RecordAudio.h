// CNGEGE
// �����߼�: ����ʵ������, Ȼ����¼�Ʋ���
// ����Record() �Զ���ʼ¼�� ����״̬Recording ����Ϊtrue
// ����stop()ֹͣ¼�� �Զ��� Recording ����Ϊfalse ��ʱ���������� Record()����¼��
// ����Close �رղ�����,�ͷ�ָ�룬��ʾ���ʵ�����ٽ���¼����

#ifndef RECOEDAUDIO_H
#define RECOEDAUDIO_H




#pragma once
#include <Windows.h>
#include <iostream>
#include <functional>
#include <vector>

#ifndef EXTERNAL_LINK_WINMMLIB
#pragma comment(lib, "winmm.lib") 
#endif // !EXTERNAL_LINK_WINMMLIB


class RecordAudio {
public:
	enum InitError : int;
public:
	/// <summary>
	/// �ɹ���¼���豸ʱ����
	/// </summary>
	using OpenRecordDeviceEvent = std::function<void()>;
	/// <summary>
	/// ��¼�Ƶ����ݳ����˻�����ʱ����,������ǰ���û��Ͻ���¼�Ƶ���������,��������¼�ƽ����ǵ��ⲿ�ֵ�����
	/// </summary>
	using HasBufferStreamEvent = std::function<void(LPSTR, DWORD)>;
	/// <summary>
	/// ���豸�¼�������ֹͣ¼����Ϣ,һ�����û��ֶ�����Stop()��Close() �󴥷�
	/// </summary>
	using CloseRecordDeviceEvent = std::function<void()>;
public:
	RecordAudio();
	~RecordAudio();
	/// <summary>
	/// ��ʼ�� �ڳ�ʼ��ǰ ���ú�¼�Ʋ��� �ͻص��¼�
	/// </summary>
	/// <returns>��ʼ��ʱ�Ƿ�������,�Լ���������</returns>
	InitError Init();
	// ��ʼ¼��
	void Record();
	// ֹͣ¼�� ��ͣ¼��
	void Stop();
	// �ر�¼���豸,ȡ����ʼ��,���Ҫ����ʹ�ø���¼��,�����³�ʼ��
	void Close();
	
	// �Ƿ�����¼����
	bool IsRecording();
	// ���û�������С, ��¼�Ƶ����ݵ��������Сʱ,���Զ����ûص�����
	void setBuffsize(int);
	// ����¼����Ƶ�ĸ�ʽ Ĭ��:WAVE_FORMAT_PCM
	void setFormatTag(WORD);
	// ���ò����� Ĭ��:8000
	void setSamplesPerSec(DWORD);
	// ����¼����Ƶ�ľ��� Ĭ��:16
	void setBitsPerSample(WORD);

	// ��ȡ������˷��豸
	static std::vector<WAVEINCAPS> GetAllDevs();
	// ��ȡ�豸����Ƶ�����豸����
	static int GetDevsNum();
	// ��ȡһ��ָ����ŵ���˷����
	static WAVEINCAPS GetDevsFromId(UINT);

	//���õ�ǰʹ�õ���˷��豸 from WAVEINCAPS
	//void setDevive(WAVEINCAPS);
	//���õ�ǰʹ�õ���˷��豸 from Num
	void setDevive(UINT);
	//��ȡ��ǰʹ�õ���˷��豸
	WAVEINCAPS getCurrentDevice();
	//��ȡ��ǰʹ�õ���˷��豸�ı��
	UINT getCurrentDeviceNum();

private:
	static void CALLBACK callback(
		HWAVEIN   hwi,							   // �豸���
		UINT      uMsg,							   // ��Ϣ
		DWORD_PTR dwInstance,					   // ����
		DWORD_PTR dwParam1,						   // ����1
		DWORD_PTR dwParam2);					   // ����2
	void WaveInProcess(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

private:
	HWAVEIN hWaveIn{};				// �򿪵�һ����Ƶ�����豸
	WAVEFORMATEX waveform{};		// ¼�Ʋ�������� waveform.nChannels ��������
	WAVEHDR wHdr1{}, wHdr2{};
	BYTE* pBuffer1 = nullptr;		// ¼�Ƶ���Ƶ�ֽڻ��洦 ����1
	BYTE* pBuffer2 = nullptr;		// ¼�Ƶ���Ƶ�ֽڻ��洦 ����2
	DWORD bufsize = 1024 * 120;		// Ĭ�ϻ����С
	bool Recording = false;			// �Ƿ�����¼����
	bool isInit = false;			// �Ƿ��ʼ������
	UINT currentDeviceNum = WAVE_MAPPER;		// ʹ����˷��豸�ı��
private:
	/// <summary>
	/// ���ɹ���¼���豸ʱ����
	/// </summary>
	OpenRecordDeviceEvent OpenRecordDevice = NULL;
	/// <summary>
	/// ��¼�Ƶ����ݳ����˻�����ʱ����,������ǰ���û��Ͻ���¼�Ƶ���������,��������¼�ƽ����ǵ��ⲿ�ֵ�����
	/// </summary>
	HasBufferStreamEvent HasBufferStream = NULL;
	/// <summary>
	/// ���豸�¼�������ֹͣ¼����Ϣ,һ�����û��ֶ�����Stop()��Close() �󴥷�
	/// </summary>
	CloseRecordDeviceEvent CloseRecordDevice = NULL;

public:
	/// <summary>
	/// ���ɹ���¼���豸ʱ����
	/// </summary>
	/// <param name=""></param>
	void onOpenRecordDeviceEvent(OpenRecordDeviceEvent);
	/// <summary>
	/// ��¼�Ƶ����ݳ����˻�����ʱ����,������ǰ���û��Ͻ���¼�Ƶ���������,��������¼�ƽ����ǵ��ⲿ�ֵ�����
	/// </summary>
	/// <param name=""></param>
	void onHasBufferStreamEvent(HasBufferStreamEvent);
	/// <summary>
	/// ���豸�¼�������ֹͣ¼����Ϣ,һ�����û��ֶ�����Stop()��Close() �󴥷�
	/// </summary>
	/// <param name=""></param>
	void onCloseRecordDeviceEvent(CloseRecordDeviceEvent);

};


#endif // !RECOEDAUDIO_H