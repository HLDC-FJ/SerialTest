#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <string.h>
#include <sstream>
#include <string>
#include <regex>
#include <vector>

#include "simple_udp.h"

using namespace std;

/*
 * TEST Code
 * g++ 001.cc にてBuild
 */


#define SERIAL_PORT "/dev/ttyUSB1"

// 送信先IP & Port No 設定
//simple_udp udp0("192.168.30.155",4001);



vector<string> split(string str, string separator) {
    if (separator == "") return {str};
    vector<string> result;
    string tstr = str + separator;
    long l = tstr.length(), sl = separator.length();
    string::size_type pos = 0, prev = 0;

    for (;pos < l && (pos = tstr.find(separator, pos)) != string::npos; prev = (pos += sl)) {
        result.emplace_back(tstr, prev, pos - prev);
    }
    return result;
}




int main(int argc, char *argv[])
{
    std::cout << "Hello Word." << std::endl;

    if (argc < 2){
        std::cout << "引数 Error : ./a.out HostIPaddress PortNo \r\n";
        std::cout << "ex) ./a.out 192.168.30.10 4001" << std::endl;
        return -1;
    }

    std::string Hostip = argv[1];
    int PortNo = atoi(argv[2]);
    simple_udp udp0(Hostip , PortNo);

    std::stringstream ss;
    std::string s2 , s3 , st;
    int IFast , ILast , TotalLen;

    // Debug
    int hoge;
    int count = 0;

    unsigned char msg[] = "serial port open...\n";
    unsigned char buf[255];             // バッファ
    int fd;                             // ファイルディスクリプタ
    struct termios tio;                 // シリアル通信設定
//    int baudRate = B9600;
    int baudRate = B921600;
    int i;
    int len;
    int ret;
    int size;

    fd = open(SERIAL_PORT, O_RDWR);     // デバイスをオープンする
    if (fd < 0) {
        printf("open error\n");
        return -1;
    }

    tio.c_cflag += CREAD;               // 受信有効
    tio.c_cflag += CLOCAL;              // ローカルライン（モデム制御なし）
    tio.c_cflag += CS8;                 // データビット:8bit
    tio.c_cflag += 0;                   // ストップビット:1bit
    tio.c_cflag += 0;                   // パリティ:None

    cfsetispeed( &tio, baudRate );
    cfsetospeed( &tio, baudRate );

    cfmakeraw(&tio);                    // RAWモード

    tcsetattr( fd, TCSANOW, &tio );     // デバイスに設定を行う

    ioctl(fd, TCSETS, &tio);            // ポートの設定を有効にする

    s2 = "";
    s3 = "";
    st = "";

    // 送受信処理ループ
    while(1) {
        len = read(fd, buf, sizeof(buf));
        if (0 < len) {
            for(i = 0; i < len; i++) {
                ss << std::hex << buf[i];
            }
            s2 += ss.str();
            ss.str("");

            // 改行コード検出
            IFast = s2.find_first_of("\n");         // 改行コード検索

            if (IFast != -1){                       // 改行コードあり
                TotalLen = s2.length();             // 全データ数
                ILast = s2.find_last_of("\n");      // 最終改行コード位置

                s3 = s2.substr(0,ILast);            // 最終開業コードまで取得
                if (IFast != ILast){
                    s2 = s2.substr(ILast+1);
                } else {
                    if ((TotalLen-1) == ILast){
                        s2 = "";
                    } else {
                        s2 = s2.substr(ILast+1);
                    }
                }

                vector<string> ary = split(s3,"\n");
                //std::cout << "\r\nSize: " << ary.size() << "\r\n" << std::endl;
                for (int z=0; z<ary.size(); z++){
                    st = ary[z];
                    st += "\n";

                    std::cout << st;
                    udp0.udp_send(st);
                }
            }
        }
        // エコーバック
        //write(fd, buf, len);
    }

    close(fd);                              // デバイスのクローズ
    return 0;
}
