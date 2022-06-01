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
 * Serial to UDP
 * Radar Sensor から取得したシリアルデータをUDPにてHostへ転送する
 */


#define SERIAL_PORT "/dev/ttyUSB1"

#define debug 0



// 文字列 分割処理
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


// Serial Tx Data Task
void Stx(int fd , std::string dat){
    for (int zz=0; zz<=dat.length(); zz++){
        std::string hoge = dat.substr(zz,1);
        write (fd, hoge.c_str() , 1);
        usleep(25000);
    }
}



// メイン処理
int main(int argc, char *argv[])
{
    std::cout << "Hello Word." << std::endl;

    if (argc < 1){
        /*
        std::cout << "引数 Error : ./a.out HostIPaddress PortNo \r\n";
        std::cout << "ex) ./a.out 192.168.30.10 4001" << std::endl;
        */
        std::cout << "引数 Error : ./a.out HostIPaddress \r\n";
        std::cout << "ex) ./a.out 192.168.30.10" << std::endl;
        return -1;
    }

    std::stringstream ss;
    std::string s2 , s3 , st;
    std::string shoge;
    int IFast , ILast , TotalLen;

    // Debug
    int hoge;
    int count = 0;

    unsigned char msg[] = "serial port open...\n";
    unsigned char buf[60000];           // バッファ
    int fd;                             // ファイルディスクリプタ
    struct termios tio;                 // シリアル通信設定
    int baudRate = B921600;             // ボーレート設定
    int i;
    int len;
    int ret;
    int size;

    fd = open(SERIAL_PORT, O_RDWR);     // デバイスをオープンする
    if (fd < 0) {
        printf("open error\n");
        return -1;
    }

    // UDP Port bind
    std::string Hostip = argv[1];
    //int PortNo = atoi(argv[2]);
    //simple_udp udp0(Hostip , PortNo);
    simple_udp udp0(Hostip ,4001);

    tio.c_cflag += CREAD;               // 受信有効
    tio.c_cflag += CLOCAL;              // ローカルライン（モデム制御なし）
    tio.c_cflag += CS8;                 // データビット:8bit
    tio.c_cflag += 0;                   // ストップビット:1bit
    tio.c_cflag += 0;                   // パリティ:None

     //non canonical, non echo back
    tio.c_iflag &= ~(ECHO | ICANON);

     //non blocking
     tio.c_cc[VMIN] = 0;
     tio.c_cc[VTIME] = 0;


    cfsetispeed( &tio, baudRate );
    cfsetospeed( &tio, baudRate );

    cfmakeraw(&tio);                    // RAWモード

    tcsetattr( fd, TCSANOW, &tio );     // デバイスに設定を行う

    ioctl(fd, TCSETS, &tio);            // ポートの設定を有効にする

    s2 = "";
    s3 = "";
    st = "";
    len = 0;

    Stx(fd , "reset\n");



    // 送受信処理ループ
    while(1) {
        len = read(fd, buf, sizeof(buf));
        if (0 < len) {
            ss.str("");                                 // 一時バッファ初期化
            // 受信データ取得
            for(i = 0; i < len; i++) {
                ss << std::hex << buf[i];
            }
            s2 += ss.str();                             // s2 : 受信データ退避

            // 改行コード検出
            IFast = s2.find_first_of("\n");             // 改行コード検索

            if (IFast != -1){                           // 改行コードあり
                TotalLen = s2.length();                 // 全データ数
                ILast = s2.find_last_of("\n");          // 最終改行コード位置

                s3 = s2.substr(0,ILast);                // 最終改行コードまで取得
                shoge = s2.substr(ILast+1);             // 最終改行コード以降を取得 (次回使用)
                s2 = shoge;

                #if debug
                std::cout << "Fast:" << IFast << " Last:" << ILast << " Len:" << TotalLen << std::endl;
                std::cout << "S3: " << s3 << std::endl;
                std::cout << "S2: " << shoge << std::endl;
                #else
                vector<string> ary = split(s3,"\n");    // 改行コードにてデータ分割
                for (int z=0; z<ary.size(); z++){
                    st = ary[z];
                    st += "\n";                         // 改行コードを付加しておく
                    std::cout << st;                    // コンソール出力
                    udp0.udp_send(st);                  // UDP にてデータ送信
                }
                #endif
            }
        }
        // エコーバック
        //write(fd, buf, len);
    }

    close(fd);                              // デバイスのクローズ
    return 0;
}
