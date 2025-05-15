client在win下，server在linux下

服务器端指定本地的ip和port在后台执行
客户端指定服务器端的ip和port，指定服务器端的文件下载，支持断点续传


gcc -o client.exe client.c -lwsock32