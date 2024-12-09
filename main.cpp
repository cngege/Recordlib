// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "Meet/include/meet"

#include "Recordlib/RecordAudio.h"
#include "Recordlib/PlayAudio.h"
#include <fstream>

#include <comdef.h> //_bstr_t

// MIC 将使用的录制设备的编号
UINT RECORD_DeviceNum = 0;
// 将使用的播放设备的编号
UINT PLAY_DeviceNum = 0;


void RecordToWavFile();
void RecordToFile() {
    std::cout << "录制" << std::endl;
    std::ofstream file1;
    RecordAudio R = RecordAudio();
    std::cout << "初始化 RecordAudio" << std::endl;
    //R.InitFile("MyAudio.Audio");
    std::cout << "初始化 MyAudio.Audio" << std::endl;
    std::cout << "开始录制 回车键结束录制" << std::endl;
    R.onOpenRecordDeviceEvent([&file1]() {
        file1.open("MyAudio.Audio", std::ios::binary);
    std::cout << "回调: 打开设备" << std::endl;
        });
    R.onHasBufferStreamEvent([&file1](const char* Stream, ULONG size) {
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
    return;
}

void PlayFormFile() {
    std::cout << "播放" << std::endl;
    std::ifstream file_R;
    PlayAudio P = PlayAudio();
    P.setDevive(PLAY_DeviceNum);
    file_R.open("MyAudio.Audio", std::ios::binary);
    if (!file_R.is_open()) {
        std::cout << "准备播放的文件不存在." << std::endl;
        return;
    }
    std::cout << "开始播放 回车键停止播放." << std::endl;
    P.onOpenPlayDevice([]() {
        std::cout << "打开了播放设备." << std::endl;
        });

    P.onClosePlayDevice([]() {
        std::cout << "关闭了播放设备." << std::endl;
        });

    P.onNeedWriteData([&file_R, &P](LPSTR data, int* size) {
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
    return;
}

void StartTcpServer() {
    // TCP服务
    meet::TCPServer s;
    // Record
    RecordAudio R = RecordAudio();
    R.setDevive(RECORD_DeviceNum);
    {
        // 显示出现在正在使用的麦克风录制设备
        std::cout << "当前使用的麦克风设备: ";
        std::cout << (char*)_bstr_t(R.getCurrentDevice().szPname) << std::endl;
    }
    // Play
    PlayAudio P = PlayAudio();

    USHORT port = 1234;
    int maxconn = 1;
    std::cout << "将监听 0.0.0.0:1234" << std::endl;
    s.onClientDisConnect([&R, &P](meet::TCPServer::MeetClient meetClient) {
        R.Stop();
        P.Stop();
        printf("\n[%s -:- %d][连接] 断开连接\n", meetClient.addr.toString().c_str(), meetClient.port);
    });
    s.onNewClientConnect([&R,&P](meet::TCPServer::MeetClient meetClient /*meet::IP ip, USHORT port, SOCKET socket*/) {
        R.Record();
        P.Play();
        printf("\n[%s -:- %d][连接] 连接成功\n", meetClient.addr.toString().c_str(), meetClient.port);
    });
    s.onRecvData([&P](meet::TCPServer::MeetClient meetClient /*meet::IP ip, USHORT port, SOCKET socket*/, ULONG64 len, const char* data) {
        //std::cout << "接受到：" << std::to_string(len) << " 字节" << std::endl;
        P.WriteAudioData(const_cast<char*>(data), (int)len);
    });
    meet::Error listen_err = s.Listen(meet::IP("0.0.0.0"), port, maxconn);
    if (listen_err != meet::Error::noError) {
        std::cout << "监听错误:" << meet::getString(listen_err) << std::endl;
        return;
    }

    R.onHasBufferStreamEvent([&s](const char* Stream, ULONG size) {
        if (size == 0) return;
        for (auto& c : s.GetALLClient()) {
            s.sendData(c.clientSocket, const_cast<char*>(Stream), size);
        }
    });

    //初始化
    R.Init();
    P.Init();

    auto _ = getchar();
    // 首先断开所有远程连接的客户端
    auto clientList = s.GetALLClient();
    for (auto& c : clientList) {
        s.disClientConnect(c.addr, c.port);
    }
    // 然后关闭TCPServer
    // s.stop() 我发现了一件大事，Server好像还没有停止的接口

    // 关闭麦克风 和 输出设备
    R.Close();
    P.Close();

}

void StartTcpClient() {
    meet::TCPClient c;

    // Record
    RecordAudio R = RecordAudio();
    R.setDevive(RECORD_DeviceNum);
    {
        // 显示出现在正在使用的麦克风录制设备
        std::cout << "当前使用的麦克风设备: ";
        std::cout << (char*)_bstr_t(R.getCurrentDevice().szPname) << std::endl;
    }
    // Play
    PlayAudio P = PlayAudio();

    c.onDisConnect([&P,&R]() {
        R.Stop();
        P.Stop();
        std::cout << "服务端断开了连接" << std::endl;
    });

    c.onRecvData([&P](ULONG64 len, const char* data) {
        // 接受到了音频数据 进行播放
        P.WriteAudioData(const_cast<char*>(data), (int)len);
    });

    meet::Error connect_error;
    R.Init();
    P.Init();

    std::cout << "输入你要连接的主机地址：";
    std::string input;
    std::getline(std::cin, input);
    if ((connect_error = c.connect(meet::IP(input), 1234)) != meet::Error::noError) {
        std::cout << "连接错误:" << meet::getString(connect_error) << std::endl;
        return;
    }

    R.onHasBufferStreamEvent([&c](const char* Stream, ULONG size) {
        if (size == 0) return;
        if(!c.Connected) return;
        c.sendData((char*)Stream, size);
    });

    R.Record();
    P.Play();
    
    auto _ = getchar();
    c.disConnect();
    // 关闭麦克风 和 输出设备
    R.Close();
    P.Close();
}


void ShowAndSelectMic() {

}

int main()
{
    for (;;) {

        //system("cls");
        std::cout << "0. 退出" << std::endl;
        std::cout << "1. 录制并存储" << std::endl;
        std::cout << "2. 读文件并播放" << std::endl;
        std::cout << "3. 监听TCP网络服务,开启语音" << std::endl;
        std::cout << "4. 连接网络,开启语音" << std::endl;
        std::cout << "5. 录制并保存到WAV文件" << std::endl;
        std::cout << "6. 麦克风设备["<< RecordAudio::GetDevsNum() << "] 当前:";
        {
            // wchar 最大长度32，这是mmeapi 头文件限制死的
            std::cout << (char*)_bstr_t(RecordAudio::GetDevsFromId(RECORD_DeviceNum).szPname) << std::endl;
        }
        std::cout << "6. 播放设备[" << PlayAudio::GetDevsNum() << "] 当前:";
        {
            // wchar 最大长度32，这是mmeapi 头文件限制死的
            std::cout << (char*)_bstr_t(PlayAudio::GetDevsFromId(PLAY_DeviceNum).szPname) << std::endl;
        }
        std::cout << "你准备:";
        std::string input;
        std::getline(std::cin, input);
        if (input == "0") {
            break;
        }
        else if (input == "1") {
            RecordToFile();
        }
        else if (input == "2") {
            PlayFormFile();
        }
        else if (input == "3") {
            // TODO: 监听网络
            StartTcpServer();
        }
        else if (input == "4") {
            // TODO: 连接网路
            StartTcpClient();
        }
        else if(input == "5") {
            RecordToWavFile();
        }
        else if (input == "6") {
            // TODO: 显示并允许选择一个麦克风设备
            auto dev = RecordAudio::GetAllDevs();

        }
    }
}

#include <mmreg.h>
void RecordToWavFile() {
    size_t pcmSize = 0;
    std::cout << "录制" << std::endl;
    std::ofstream file1;
    RecordAudio R = RecordAudio();
    std::cout << "初始化 RecordAudio" << std::endl;
    std::cout << "初始化 MyAudio.wav" << std::endl;
    std::cout << "开始录制 回车键结束录制" << std::endl;
    R.onOpenRecordDeviceEvent([&file1]() {
        file1.open("MyAudio.wav", std::ios::binary);
        file1.seekp(44);
        std::cout << "回调: 打开设备" << std::endl;
    });
    R.onHasBufferStreamEvent([&file1,&pcmSize](const char* Stream, ULONG size) {
        file1.write(Stream, size);
        pcmSize += size;
        std::cout << "回调: 录制中... 存储了 " << size << " 字节" << std::endl;
    });

    R.onCloseRecordDeviceEvent([&R, &file1, &pcmSize]() {
        // 写入头：
        byte header[44];
        std::cout << pcmSize << std::endl;
        //RIFF 块标识和大小（0 - 11 字节）：
        {
            //前 4 个字节是 “RIFF” 字符串
            memcpy_s(header, 4, std::string("RIFF").data(), 4);
            // 紧接着的 4 个字节表示整个文件的大小减去 8 字节
            *reinterpret_cast<UINT*>(header + 4) = static_cast<UINT>(pcmSize + 44 - 8);
            //最后 4 个字节是 “WAVE” 字符串
            memcpy_s(header + 8, 4, std::string("WAVE").data(), 4);
        }
        //fmt 块标识和音频格式信息（12 - 35 字节）：
        {
            //第 12 - 15 字节是 “fmt”（注意 fmt 后面有一个空格），这是格式块（Format chunk）的标志。
            memcpy_s(header + 12, 4, std::string("fmt ").data(), 4);
            //16 - 19 字节表示 fmt 块的大小，对于常见的 PCM 音频格式，这个值通常是 16。
            *reinterpret_cast<UINT*>(header + 16) = 16;
            // 20 - 21 字节表示音频格式，对于未压缩的 PCM 音频，这个值通常是 1。其他的值可能代表不同的音频编码格式。
            *reinterpret_cast<USHORT*>(header + 20) = R.getFormatTag();
            //22 - 23 字节表示声道数，例如，1 代表单声道，2 代表立体声。
            *reinterpret_cast<USHORT*>(header + 22) = R.getChannels();
            // 24 - 27 字节表示采样率，例如，44100 表示每秒采样 44100 次。
            *reinterpret_cast<UINT*>(header + 24) = R.getSamplesPerSec();
            //28 - 31 字节表示每秒的数据字节数，它是通过采样率、声道数和每个样本的字节数计算得到的。
            *reinterpret_cast<UINT*>(header + 28) = R.getAvgBytesPerSec();
            //32 - 33 字节表示每个样本的字节数，对于 16 位的 PCM 音频，这个值是 2。 每个样本的字节数
            *reinterpret_cast<UINT*>(header + 32) = R.getBlockAlign();
            //34 - 35 字节表示每个样本的位数，对于 16 位的 PCM 音频，这个值是 16。 每个样本的位数
            *reinterpret_cast<UINT*>(header + 34) = R.getBitsPerSample();
        }
        //data 块标识和音频数据大小（36 - 43 字节）
        {
            //36 - 39 字节是 “data” 字符串，这是数据块（Data chunk）的标志，表明接下来是音频数据部分。
            memcpy_s(header + 36, 4, std::string("data").data(), 4);
            //40 - 43 字节表示音频数据的大小，单位是字节，这个大小不包括文件头部分。
            *reinterpret_cast<UINT*>(header + 40) = static_cast<UINT>(pcmSize);
        }

        file1.seekp(0);
        file1.write((char*)header, 44);
        file1.close();
        std::cout << "回调: 关闭设备" << std::endl;
    });
    R.setFormatTag(WAVE_FORMAT_PCM);
    R.Init();
    R.Record();
    auto _ = getchar();
    R.Close();
    return;
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
