硬件：GD32F103C8T6

参考文献：

1. [嵌入式OTA升级实现原理_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1em411C7xy/?spm_id_from=333.1387.favlist.content.click&vd_source=3b2bba9cd9eee7c88a08ea7d3ea6261a)

---



1、OTA升级是什么：OTA升级，说简单点，就是对我们单片机Flash里面的程序进行更新，就和我们用烧录工具去更新程序一样，只是烧录方式，变成了我们先把程序(Bin文件)先上传到服务器，然后由服务器给每个设备下发程序更新指令和数据。

2、OTA固件升级的两种方式：

![25e57f47790ab8452409b1b7ade72fd](./../imgs/25e57f47790ab8452409b1b7ade72fd.jpg)

![936f12ceb5bdf00d4cab50b3b248a60](./../imgs/936f12ceb5bdf00d4cab50b3b248a60.jpg)

3、相关工作：

```
1、服务器与开发板的通信流程、通信协议
2、开发板上BootLoader设计
3、固件应用设计
```

4、目标

**需求1：**

有线OTA：开发板通过以太网网线连接路由器，服务器通过通过网线连接路由器（wifi？），服务器发送bin固件包，开发板替换内部bin固件包

环境：spaceOS

流程：单片机通过外设接口（如 UART 、 IIC 、 SPI 、 CAN 、 USB 等接口），连接具备联网能力的模块、器件、设备（以下统称上位机）。上位机从服务器上拉取固件包，再将固件包以约定的通讯协议，经由通讯接口发送至 **MCU** ，由 **MCU** 负责固件的解析、解密、存储、更新等操作，以完成设备固件更新的功能.



**需求2：**

服务器通过网线（HTTP协议）传递文件到开发板指定目录下



专家咨询费、填表（12个）





目前先熟悉OTA的整体流程 

[【手把手教程 4G通信物联网 OTA远程升级 BootLoader程序设计】GD32F103C8T6单片机【上篇章】_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1SatHeBEVG/?spm_id_from=333.1387.favlist.content.click&vd_source=3b2bba9cd9eee7c88a08ea7d3ea6261a)

需求：

![image-20250415215148017](./../imgs/image-20250415215148017.png)



需要bootloader去跳转程序区A或者更新程序区A(OTA升级方式第一种)

​	单片机执行程序，从起始位置（低地址）开始运行，一般起始位置放的是bootloader程序段

**必须实现的功能：**

- 开发板某一个掉电不易失寄存器存放ota_flag标志，1表示需要bootloader去擦除程序区从外部flash读取进行更新，0表示跳转到程序区
- 开发板需要一个外部的flash存放从服务器下载下来的固件文件

**方便使用的额外功能**

- 通过串口IAP实现更新程序区固件
- OTA版本号上报
- 外部flash存放多个程序文件，通过命令下发更新指定文件







