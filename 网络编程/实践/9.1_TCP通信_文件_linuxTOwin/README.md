发送端输入接收端的IP地址，端口号，要发送的文件名，运行在linux环境下
接收端输入运行的端口号，运行在win环境下
    需要win安装gcc环境（MinGW）
    win下运行下面gcc -o recvFile.exe recvFile.c -lwsock32