// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#define RECORD
//#define PLAY

#ifdef RECORD
#include "Recordlib/RecordAudio.h"
#endif // RECORD

#ifdef PLAY
#include "Recordlib/PlayAudio.h"
#endif // PLAY

#include <fstream>
int main()
{
#ifdef RECORD
	std::cout << "录制" << std::endl;
	std::ofstream file1;
	RecordAudio R = RecordAudio();
	std::cout << "初始化 RecordAudio" << std::endl;
	//R.InitFile("MyAudio.Audio");
	std::cout << "初始化 MyAudio.Audio" << std::endl;
	std::cout << "开始录制" << std::endl;
	R.onOpenRecordDeviceEvent([&file1]() {
		file1.open("MyAudio.Audio", std::ios::binary);
		std::cout << "回调: 打开设备" << std::endl;
		});
	R.onHasBufferStreamEvent([&file1](LPSTR Stream, DWORD size) {
		file1.write(Stream, size);
		std::cout << "回调: 录制中... 存储了 " << size << " 字节" << std::endl;
		});

	R.onCloseRecordDeviceEvent([&file1]() {
		file1.close();
		std::cout << "回调: 关闭设备" << std::endl;
		});
	R.Init();
	R.Record();
	auto _ = getchar();
	R.Close();
#endif // RECORD

#ifdef PLAY
	std::cout << "播放" << std::endl;
	std::ifstream file_R;
	PlayAudio P = PlayAudio();
	file_R.open("MyAudio.Audio", std::ios::binary);
	if (!file_R.is_open()) {
		std::cout << "准备播放的文件不存在." << std::endl;
		return 0;
	}
	
	P.onOpenPlayDevice([]() {
		std::cout << "打开了播放设备." << std::endl;
	});

	P.onClosePlayDevice([]() {
		std::cout << "关闭了播放设备." << std::endl;
	});

	P.onNeedWriteData([&file_R,&P](LPSTR data,int* size) {
		file_R.read(data, P.getBuffSize());
		size_t readcount = file_R.gcount();
		*size = static_cast<int>(readcount);
		std::cout << "读取了数据大小:" << readcount << std::endl;
		if (readcount > 0) {
			return true;
		}
		else {
			return false;
		}
	});
	P.Init();
	P.Play();
	auto _ = getchar();
	P.Close();
	file_R.close();
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
