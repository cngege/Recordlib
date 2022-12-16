#include "RecordAudio.h"


enum RecordAudio::InitError : int {
	// �ظ���ʼ��
	ReInit = -1,
	// �������� û�д���
	NoError = 0,
	// �豸id����
	BadDeviceId = 2,
	// ָ������Դ�ѱ�����
	AllocAted = 4,
	// û�а�װ��������
	NoDriver = 6,
	// ���ܷ���������ڴ�
	NoMem = 7,
	// �豸��֧������Ĳ��θ�ʽ
	BadFormat = 32
};

RecordAudio::RecordAudio()
{
	// ������Ƶ����ʽ
	waveform.wFormatTag = WAVE_FORMAT_PCM;				// ¼����Ƶ�ĸ�ʽ
	waveform.nSamplesPerSec = 8000;						// ������,���������ʺ�¼�ƺ����ݵĴ�С
	waveform.wBitsPerSample = 16;						// ¼����Ƶ���ֽ�,���,����
	waveform.nChannels = 1;								// �������� 1��2
	waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // �����
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // ��������
	waveform.cbSize = 0;								// ����ռ�	

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
	InitError ret = (InitError)waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)(&callback), (DWORD_PTR)this, CALLBACK_FUNCTION);		// WAVE_MAPPER:¼�Ƶ���˷�id -1ΪĬ����˷�
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

	waveInStart(hWaveIn);		//��ʾ��ʼ¼����
	
}
//
//BYTE* RecordAudio::Record(int RecordTime_ms, DWORD NewSize)
//{
//	BYTE* pBuffer1 = new BYTE[NewSize];
//	wHdr1.lpData = (LPSTR)pBuffer1;
//	wHdr1.dwBufferLength = NewSize;			// buff ��С
//	wHdr1.dwBytesRecorded = 0;				// buff ��Ŵ�С?
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



void CALLBACK RecordAudio::callback(HWAVEIN   hwi,                              // �豸���
	UINT      uMsg,							   // ��Ϣ
	DWORD_PTR dwInstance,					   // ���� this
	DWORD_PTR dwParam1,						   // ����1
	DWORD_PTR dwParam2)						   // ����2
{
	((RecordAudio*)dwInstance)->WaveInProcess(hwi, uMsg, dwInstance, dwParam1, dwParam2);
}
void RecordAudio::WaveInProcess(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	// ��ǰ����
	RecordAudio* _this = (RecordAudio*)dwInstance;
	// ��ȡ��Ƶͷ
	WAVEHDR& pwhdr = _this->wHdr1;
	// ������Ϣ
	switch (uMsg)
	{
	case WIM_OPEN:      // ��¼���豸
	{
		//printf("�ɹ����豸..\n");		// �������һ���Զ����¼�
		if (_this->OpenRecordDevice) _this->OpenRecordDevice();
		break;
	}
	case WIM_DATA:      // ���������� ��ʾ���Ѿ�¼����ָ���Ĵ�С����Ƶ��
	{
		//�������仺����
		DWORD bytrecd = pwhdr.dwBytesRecorded;	// �����ϴ����������������¼�Ƶ���Ƶ�Ĵ�С

		// �����ٵ���һ���Զ��庯�� ��¼�Ƶ���Ƶ���ݴ��ݳ�ȥ
		// ��Ƶ����:pwhdr->lpData�� ��Ƶ�Ĵ�С: bytrecd
		if(_this->HasBufferStream) _this->HasBufferStream(pwhdr.lpData, bytrecd);

		if (_this->Recording) {
			waveInAddBuffer(hwi, &pwhdr, sizeof(WAVEHDR));
		}
		break;
	}
	case WIM_CLOSE:     // �ر�¼���豸
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
