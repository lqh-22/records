# 文件系统创建

1. 查看系统分区情况
   - fdisk -l
2. 建立分区
   - fdisk /dev/sdb
3. 格式化分区
   - mkfs.ext3 /dev/sdb1
4. 挂载分区
   - mount /dev/sdb1 /test

## 挂载文件系统

```
mount -t type mountpoint device -o options
	很多嵌入式根文件系统是不可写，通过mount -o rw,remount /可重新挂载
	type:nfs可挂载网络文件
```





# 文件系统通用操作

## 不带缓存的文件I/O操作

**1、文件描述符**

- 在linux系统中有三个已经分配的文件描述符，即标准输入(0)、标注输出(1)和标准错误(2)，分别在/dev/stdin、/dev/stdout和/dev/stderr
- 文件描述符范围为0~OPEN_MAX，不同进程的相同的文件描述符可能不是同一个设备

**2、打开创建文件open()、create()函数**

```
头文件：
	sys/types.hM sys/stat.h和fcntl.h
函数定义：
    int open(const char *pathname, int flags); 
    int open(const char *pathname, int flags, mode_t mode);
	返回值：正常fd/出错-1
	pathname:最大1024字节
	flags:O_RDONLY（0）\O_WRONLY（1）\O_RDWR（2）
		参数flags除了上述三个选项之外，还有一些可选的参数。
        O_APPEND选项：使每次对文件进行写操作都追加到文件的尾端。
        O_CREAT:如果文件不存在则创建它，当使用此选择项时，第三个参数mode需要同时设定，用来说明新文件的权限。
        O_EXCL:查看文件是否存在。如果同时指定了O CREAT,而文件己经存在，会返回错误。用这种方法可以安全地打开一个文件。
        O_TRUNC:将文件长度截断为0。如果此文件存在，并且文件成功打开，则会将其长度截短为0。
    mode:在flags为0_CREAT时，结合使用配置文件权限问题
```

**3、关闭文件close()函数**

```
头文件：
	unistd.h 
函数定义：
	int close(int fd);
	返回值：成功（0）/出错（-1）
```

**4、读取文件read()函数**

```
头文件：
	unistd.h 
函数定义：
	ssize_t read(int fd, void *buf, size_t count);
	返回值：返回读取的字节数/如果到达文件尾（0）/出错（-1）
	ssize_t：具体实现可能为int(4字节)或者long long（8字节）
```

**5、写文件write()函数**

```
头文件：
	unistd.h 
函数定义：
	ssize_t write(int fd, const void *buf, size_t count);
	返回值：返回写入的字节数/出错（-1）
```

**6、文件偏移lseek()函数**

```
头文件：
	unistd.h 
函数定义：
	off_t lseek(int fildes, off_t offset, int whence);
	返回值：返回文件当前偏移量的值/出错（-1）
	这个函数对文件描述符 fildes所代表的文件，按照操作模式 whence 和偏移的大小offset，重新设定文件的偏移量。
    如果 lseek(函数操作成功，则返回新的文件偏移量的值；如果失败返回-1。由于文件的偏移量可以为负值，判断lseek(是否操作成功时，不要使用小于的判断，要使用是否等于-1来判断函数失败。
    参数 whence 和 offset 结合使用。whence 表示操作的模式，offset 是偏移的值，offset的值可以为负值。offset值的含义如下：
    如果 whence 为 SEEK SET，则 offset 为相对文件开始处的值，即将该文件偏移量设为距文件开始处offset个字节。
    如果 whence 为SEEK CUR，则offset 为相对当前位置的值，即将该文件的偏移量设置为其当前值加offset。
    如果 whence 为 SEEK END，则offset 为相对文件结尾的值，即将该文件的偏移量设置为文件长度加 offset。
```

**7、文件空间映射mmap()函数**

```
比较复杂，作用是把文件映射到内存中，对内存的操作会同步到文件中
```

**8、文件输入输出控制ioctl()函数**

```
作用：对设备进行操作
头文件：
	sys/ioctl.h
函数定义：
	int ioctl(int d, int request, ...);
	d：已经打开的设备描述符
	request：具体看驱动
```



## 带缓存的流文件I/O操作

TODO：

fopen
