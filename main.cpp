// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#define RECORD
#define PLAY

#include <iostream>
#ifdef RECORD
#include "Recordlib/RecordAudio.h"
#endif // RECORD

#ifdef PLAY
#include "Recordlib/PlayAudio.h"
#endif // PLAY
void m2();
#include <fstream>
std::ofstream file1;

int main()
{
	//RecordAudio R1 = RecordAudio();
	//PlayAudio P1 = PlayAudio();
	//while (true)
	//{
	//	BYTE* sound = R1.Record();
	//	P1.Play((LPSTR)sound, R1.RecordSize());
	//}
	//m2();
#ifdef RECORD
	std::cout << "录制" << std::endl;
	RecordAudio R = RecordAudio();
	std::cout << "初始化 RecordAudio" << std::endl;
	//R.InitFile("MyAudio.Audio");
	std::cout << "初始化 MyAudio.Audio" << std::endl;
	std::cout << "开始录制" << std::endl;
	R.onOpenRecordDeviceEvent([]() {
		file1.open("MyAudio.Audio", std::ios::binary);
		std::cout << "回调: 打开设备" << std::endl;
		});
	R.onHasBufferStreamEvent([](LPSTR Stream, DWORD size) {
		file1.write(Stream, size);
		std::cout << "回调: 录制中... 存储了 " << size << " 字节" << std::endl;
		});

	R.onStopRecordingEvent([]() {
		file1.close();
		std::cout << "回调: 关闭设备" << std::endl;
		});
	R.Init();
	R.Record();
	Sleep(1000 * 10);	// 录制10s 后暂时一下
	R.Stop();
	Sleep(1000 * 1);	// 停止录制1s 后接着录
	R.Record();
	Sleep(1000 * 10);	// 接着录制10s 后结束
	R.Stop();

	R.Close();
#endif // RECORD

#ifdef PLAY
	std::cout << "播放" << std::endl;
	std::ifstream file2;
	PlayAudio P = PlayAudio();
	//P.InitFile("MyAudio.Audio"/*, 1024 * 1024 * 4*/);
	//while (P.HaveLoop()) {
	//	P.Play(P.ReadFile());
	//	P.ReadFileEnd();
	//}
	file2.open("MyAudio.Audio", std::ios::binary);
	if (!file2.is_open()) {
		std::cout << "准备播放的文件不存在." << std::endl;
		return 0;
	}
	

	P.onNeedWriteData([&file2,&P](LPSTR data,int* size) {
		std::cout << "onNeedWriteData." << std::endl;
		//char* _data = new char[P.GetBuffSize()];
		memset(data, 0, P.GetBuffSize());
		file2.read(data, P.GetBuffSize());
		size_t readcount = file2.gcount();
		std::cout << "读取了数据大小:" << readcount << std::endl;
		*size = readcount;
		if (readcount > 0) {
			//memcpy(data, _data, readcount);
			//*size = readcount;
			return true;
		}
		else {
			return false;
		}
		//delete[] _data;
	});
	P.Init();
	P.Play();
	auto _ = getchar();
	P.Close();
	file2.close();
	//P.CloseFile();
	P.~PlayAudio();
#endif // PLAY


}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件


#include <windows.h>
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib") 

static BYTE* file = (BYTE*)malloc(sizeof(BYTE) * 512);
static DWORD    hasRecorded = 0;
static BOOL     recurr = TRUE;
class USER
{
public:
	int a = 0;
	char b = 'a';
};

void CALLBACK callback(HWAVEIN   hwi,          // 设备句柄
	UINT      uMsg,							   // 消息
	DWORD_PTR dwInstance,					   // 对象
	DWORD_PTR dwParam1,						   // 参数1
	DWORD_PTR dwParam2);					   // 参数2

void m2()
{
	HWAVEIN         hWaveIn;		        //输入设备
	HWAVEOUT        hWaveOut;		        //输出设备
	WAVEFORMATEX    waveform{};	            //定义音频流格式
	BYTE* pBuffer1, * pBuffer2;				//输入音频缓冲区（左右声道）
	WAVEHDR         whdr_i1{}, whdr_i2{};       //输入音频头
	WAVEHDR         whdr_o{};                //输出音频头
	USER* user = new USER();		//定义用户

	// 设备数量
	int count = waveInGetNumDevs();
	printf("\n音频输入数量：%d\n", count);

	// 设备名称
	WAVEINCAPS waveIncaps;
	MMRESULT mmResult = waveInGetDevCaps(0, &waveIncaps, sizeof(WAVEINCAPS));//2
	std::cout << "设备名称：";
	std::wcout << waveIncaps.szPname << std::endl;

	// 设置音频流格式
	waveform.nSamplesPerSec = 44100;												// 采样率
	waveform.wBitsPerSample = 16;												// 采样精度
	waveform.nChannels = 2;                                                     // 声道个数
	waveform.cbSize = 0;														// 额外空间	
	waveform.wFormatTag = WAVE_FORMAT_PCM;										// 音频格式
	waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) / 8;  // 块对齐
	waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;  // 传输速率

	//分配内存
	pBuffer1 = new BYTE[1024 * 10000];
	pBuffer2 = new BYTE[1024 * 10000];
	memset(pBuffer1, 0, static_cast<size_t>(1024 * 10000));   // 内存置0
	memset(pBuffer2, 0, static_cast<size_t>(1024 * 10000));   // 内存置0

	// 设置音频头
	whdr_i1.lpData = (LPSTR)pBuffer1; // 指向buffer
	whdr_i1.dwBufferLength = 1024 * 10000;     // buffer大小
	whdr_i1.dwBytesRecorded = 0;      // buffer存放大小
	whdr_i1.dwUser = 0;
	whdr_i1.dwFlags = 0;
	whdr_i1.dwLoops = 1;
	whdr_i2.lpData = (LPSTR)pBuffer1; // 指向buffer
	whdr_i2.dwBufferLength = 1024 * 10000;     // buffer大小
	whdr_i2.dwBytesRecorded = 0;      // buffer存放大小
	whdr_i2.dwUser = 0;
	whdr_i2.dwFlags = 0;
	whdr_i2.dwLoops = 1;

	// 开启录音
	MMRESULT mRet = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)callback, (DWORD_PTR)user, CALLBACK_FUNCTION);
	waveInPrepareHeader(hWaveIn, &whdr_i1, sizeof(WAVEHDR));//准备buffer
	waveInPrepareHeader(hWaveIn, &whdr_i2, sizeof(WAVEHDR));//准备buffer
	waveInAddBuffer(hWaveIn, &whdr_i1, sizeof(WAVEHDR));    //添加buffer
	waveInAddBuffer(hWaveIn, &whdr_i2, sizeof(WAVEHDR));    //添加buffer

	waveInStart(hWaveIn);
	auto _ = getchar();
	recurr = FALSE;
	//waveInStop(hWaveIn);
	waveInReset(hWaveIn);
	waveInClose(hWaveIn);

	HANDLE wait = CreateEvent(NULL, 0, 0, NULL);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);

	// 播放录音
	whdr_o.lpData = (LPSTR)file;			// 指向buffer
	whdr_o.dwBufferLength = hasRecorded;    // buffer大小
	whdr_o.dwBytesRecorded = hasRecorded;
	whdr_o.dwFlags = 0;
	whdr_o.dwLoops = 1;

	if (wait == NULL)
	{
		return;
	}
	ResetEvent(wait);
	waveOutPrepareHeader(hWaveOut, &whdr_o, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &whdr_o, sizeof(WAVEHDR));
	/*Sleep(5000);*/
	DWORD dw = WaitForSingleObject(wait, INFINITE);
	if (dw == WAIT_OBJECT_0)
	{
		std::cout << "jieshu" << std::endl;
	}
}



void CALLBACK callback(HWAVEIN   hwi,                              // 设备句柄
	UINT      uMsg,							   // 消息
	DWORD_PTR dwInstance,					   // 对象
	DWORD_PTR dwParam1,						   // 参数1
	DWORD_PTR dwParam2)						   // 参数2
{
	// 获取对象
	USER* user2 = (USER*)dwInstance;
	// 获取音频头
	PWAVEHDR  pwhdr = (PWAVEHDR)dwParam1;

	// 处理消息
	switch (uMsg)
	{
	case WIM_OPEN:                                 // 打开录音设备

		printf("成功打开设备..\n");
		break;

	case WIM_DATA:                                 // 缓冲区已满
	{
		printf("缓冲池已满..\n");
		printf("a:%d , b:%c \n", user2->a, user2->b);
		// 缓冲池信息
		DWORD buflen = pwhdr->dwBufferLength;
		DWORD bytrecd = pwhdr->dwBytesRecorded;
		hasRecorded += bytrecd;

		// 缓冲扩增
		auto _file = (BYTE*)realloc(file, hasRecorded * sizeof(BYTE));
		if (_file != nullptr) {
			file = _file;
		}
		// 存储新内容
		if (file)
		{
			memcpy(&file[hasRecorded - bytrecd], pwhdr->lpData, bytrecd);
			printf("已存储:%d byte\n", hasRecorded);
		}
		// 循环 	
		if (recurr)
		{
			// 加入缓存
			waveInAddBuffer(hwi, pwhdr, sizeof(WAVEHDR));
		}
	}
	break;

	case WIM_CLOSE:                               // 关闭录音设备
	{
		printf("停止录音..\n");

	}
	break;

	default:
		break;
	}
}