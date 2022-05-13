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

using namespace std;

/*
 * 
 */


#define SERIAL_PORT "/dev/ttyUSB1"





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
    std::string ww;

    if (argc < 1){
        std::cout << "引数 Error : ./a.out command \r\n";
        return -1;
    }

    if (argc == 1){
        ww = argv[1];
    }
    else {
        for (int ho = 1; ho< argc; ho++){
            ww += argv[ho];
            if (ho != (argc-1)){
                ww += " ";
            }
        }
    }
    ww += "\n";

    std::stringstream ss;

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

    std::cout << ww.c_str() << std::endl;

    Stx(fd , ww);

#if 1
    while(1) {
        len = read(fd, buf, sizeof(buf));
        if (0 < len) {
            for(i = 0; i < len; i++) {
                ss << std::hex << buf[i];
            }
            std::cout << ss.str() << std::endl;
        }
    }
#endif

    close(fd);                              // デバイスのクローズ
    return 0;
}
