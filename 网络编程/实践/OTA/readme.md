client在win下，server在linux下

服务器端指定本地的ip和port在后台执行
客户端指定服务器端的ip和port，指定服务器端的文件下载，支持断点续传


gcc -o client.exe client.c -lwsock32

在client中添加了差分升级和解压缩：
gcc -o client.exe ./bspatch/bspatch.c ./bzip2/blocksort.c ./bzip2/huffman.c ./bzip2/crctable.c ./bzip2/randtable.c ./bzip2/compress.c ./bzip2/decompress.c ./bzip2/bzlib.c -lwsock32


bsdiff：形成补丁包
bspatch：应用补丁包
versionTable：对flash指定区域维护固件的版本表
app：
    patch：存放临时的补丁包(filename_v1_v2_patch)
    test1: 固件名test1
        v1.0
            test1.bin
        v1.1
        ...
        v2.0
    test2:
    .
    .
    .
    test3:


otaFlashInfo.txt
    断点续传使用
versionTable.txt
    存放固件名和版本号，差分升级使用