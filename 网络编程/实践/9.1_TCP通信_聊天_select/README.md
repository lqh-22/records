TODO：select补充/fcntl补充

**需求：**
聊天功能

**思路(服务端)：**
（服务端使用select实现非阻塞TCP连接，select实现非阻塞处理recv消息，select实现非阻塞处理send消息）
- 在linsten之后调用select监控网络套接字和stdin
  - select超时：表示没有连接请求/没有数据接受/stdin没有输入
  - select返回大于0：
    - 有连接请求：将新的套接字加入监控集合
    - 有输入接受：输出到stdou上
    - stdin有输入：发送给客户端