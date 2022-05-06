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

#include "simple_udp.h"


/*
 * TEST Code
 * g++ 001.cc にてBuild
 */


#define SERIAL_PORT "/dev/ttyUSB1"

// 送信先IP & Port No 設定
simple_udp udp0("192.168.30.155",4001);

int main(int argc, char *argv[])
{
    std::cout << "Hello Word." << std::endl;

    std::stringstream ss;
    std::string s2 , s3;
    int hoge;

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

    // 送受信処理ループ
    while(1) {
        len = read(fd, buf, sizeof(buf));
        if (0 < len) {
            for(i = 0; i < len; i++) {
                //std::cout << buf[i];

                ss << std::hex << buf[i];
                s2 = s2 + ss.str();
                //std::cout << s2;

                hoge = s2.find_first_of('\n');
                if ( hoge != -1){
                    s3 = s2.substr(0,hoge+1);
                    s2 = s2.substr(hoge+1);
                    std::cout << s3;
                    udp0.udp_send(s3);
                }
            }
/*
            if (buf[i] == '\n'){
                break;
                std::cout << s2;
                udp0.udp_send(s2);
                printf("\n");
                s2 = "";
            }
*/
        }
        // エコーバック
        //write(fd, buf, len);
    }

    close(fd);                              // デバイスのクローズ
    return 0;
}
