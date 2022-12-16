// CNGEGE
// �����߼�: ����ʵ������, Ȼ����¼�Ʋ���
// ����Record() �Զ������߳̿�ʼ¼�� ����״̬Recording ����Ϊtrue
// ����stop()ֹͣ¼�� �Զ��� Recording ����Ϊfalse ��ʱ���������� Record()����¼��
// ����Close �رղ�����,�ͷ�ָ�룬��ʾ���ʵ�����ٽ���¼����


#pragma once
#include <Windows.h>
#include <iostream>
#include <thread>
#include <functional>
#pragma comment(lib, "winmm.lib") 

class RecordAudio {
public:
	/// <summary>
	/// �ɹ���¼���豸ʱ����
	/// </summary>
	using OpenRecordDeviceEvent = void(__fastcall*)();
	/// <summary>
	/// ��¼�Ƶ����ݳ����˻�����ʱ����,������ǰ���û��Ͻ���¼�Ƶ���������,��������¼�ƽ����ǵ��ⲿ�ֵ�����
	/// </summary>
	using HasBufferStreamEvent = void(__fastcall*)(LPSTR, DWORD);
	/// <summary>
	/// ���豸�¼�������ֹͣ¼����Ϣ,һ�����û��ֶ�����Stop()��Close() �󴥷�
	/// </summary>
	using StopRecordingEvent = void(__fastcall*)();
public:
	RecordAudio();
	~RecordAudio();
	/// <summary>
	/// ��ʼ�� �ڳ�ʼ��ǰ ���ú�¼�Ʋ��� �ͻص��¼�
	/// </summary>
	void Init();
	void Resize(size_t);
	void Record();					// ÿ�ε���¼��һ����,��¼�Ƶ����ݴ洢�ڻ�����
	//BYTE* Record(int RecordTime_ms, DWORD NewSize = 1024 * 1024);
	//size_t RecordSize();
	void Stop();
	void Close();
	bool IsRecording();
	//void InitFile(const char* Path);
	//void WriteInFile(BYTE* Record);
	//void WriteInFile(BYTE* Record, DWORD size);
	//int CloseFile();

private:
	static void CALLBACK callback(
		HWAVEIN   hwi,							   // �豸���
		UINT      uMsg,							   // ��Ϣ
		DWORD_PTR dwInstance,					   // ����
		DWORD_PTR dwParam1,						   // ����1
		DWORD_PTR dwParam2);					   // ����2
	void WaveInProcess(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

private:
	HWAVEIN hWaveIn;				// �򿪵�һ����Ƶ�����豸
	WAVEFORMATEX waveform;			// ¼�Ʋ�������� waveform.nChannels ��������
	WAVEHDR wHdr1;
	BYTE* pBuffer1;					// ¼�Ƶ���Ƶ�ֽڻ��洦
	//HANDLE wait;					// �����¼�

	DWORD bufsize = 1024 * 15;		// Ĭ�ϻ����С
	//FILE* file;
	bool Recording = false;			// �Ƿ�����¼����
	bool IsInit = false;			// �Ƿ��ʼ������

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
	StopRecordingEvent StopRecording = NULL;

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
	void onStopRecordingEvent(StopRecordingEvent);

};