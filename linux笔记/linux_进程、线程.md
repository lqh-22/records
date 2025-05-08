TODO:linux高级程序设计、LINUX网络编程书籍



#  进程操作命令

<img src="./../imgs/image-20250428101411810.png" alt="image-20250428101411810" style="zoom:80%;" />

# 进程控制

![image-20250428101518129](./../imgs/image-20250428101518129.png)

**进程创建**

- fork()
  - vfork()确保子进程一定优先于父进程开始执行

- execve
  - <img src="./../imgs/image-20250428102109985.png" alt="image-20250428102109985" style="zoom:50%;" />

- system
  - 底层调用execve，不同的是execve会覆盖原程序，而system会新开一个进程执行


**进程终止**

- _exit()和exit()
- <img src="./../imgs/image-20250428102337981.png" alt="image-20250428102337981" style="zoom:50%;" />
- exit会清理I/O缓存后输出

**僵尸进程**

- 子进程结束后，父进程没有调用waitpid清理子进程，该子进程即为僵尸进程，若父进程结束，僵尸子进程会被init进程收养，由init进程调用waitpid清理僵尸进程

- **wait的使用**

  - | 所需头文件 | `#include<sys/types.h>` `#include<sys/wait.h>`               |
    | ---------- | ------------------------------------------------------------ |
    | 函数功能   | 等待子进程中断或结束                                         |
    | 函数原型   | `pid_t wait(int *status);`                                   |
    | 函数传入值 | `status`：用于获取子进程状态的指针，通过该指针可获取子进程退出信息（如正常退出状态码或终止信号等） |
    | 函数返回值 | 执行成功则返回终止子进程的进程号（PID）；若有错误发生则返回 -1，失败原因存于 `errno` 中 |
    | 备注       | `wait()` 会暂停目前进程的执行，直到其子进程终止或有信号到达并被处理，该函数用于父进程等待子进程结束，回收子进程资源，避免僵尸进程产生。 |

  - | 宏                     | 功能描述                                                     |
    | ---------------------- | ------------------------------------------------------------ |
    | `WIFEXITED(status)`    | 若子进程正常退出，返回非零值；反之，返回 0。正常退出意味着子进程调用了 `exit` 或者 `_exit` 函数，或者从 `main` 函数中返回。 |
    | `WEXITSTATUS(status)`  | 当 `WIFEXITED(status)` 为真时，此宏可用于获取子进程的退出状态码。子进程通常通过 `exit` 函数或者从 `main` 函数返回一个整数值来设定退出状态码，其取值范围是 0 - 255。 |
    | `WIFSIGNALED(status)`  | 若子进程因接收到信号而终止，返回非零值；否则，返回 0。这表明子进程是被某个信号（如 `SIGKILL` 或 `SIGTERM`）终止的。 |
    | `WTERMSIG(status)`     | 当 `WIFSIGNALED(status)` 为真时，此宏用于获取导致子进程终止的信号编号。例如，`SIGKILL` 的编号是 9，`SIGTERM` 的编号是 15。 |
    | `WIFSTOPPED(status)`   | 若子进程被暂停执行，返回非零值；否则，返回 0。子进程可能因为接收到 `SIGSTOP`、`SIGTSTP` 等信号而暂停。 |
    | `WSTOPSIG(status)`     | 当 `WIFSTOPPED(status)` 为真时，此宏用于获取使子进程暂停的信号编号。 |
    | `WIFCONTINUED(status)` | 若子进程因接收到 `SIGCONT` 信号而继续执行，返回非零值；否则，返回 0。 |

- **waitpid()**

  - `waitpid` 是 Linux 系统下用于等待子进程状态改变（如终止或暂停）的系统调用，常用于父进程回收子进程资源，避免僵尸进程。以下是详细解释：

    ### 1. 函数原型与头文件

    ```c
    #include <sys/types.h>
    #include <sys/wait.h>
    
    pid_t waitpid(pid_t pid, int *status, int options);
    ```

    ### 2. 参数说明

    - `pid`：指定等待的子进程标识，取值及含义如下：
      - `pid < -1`：等待进程组 ID 为 `pid` 绝对值的任意子进程。
      - `pid = -1`：等待任意子进程（等价于 `wait()`）。
      - `pid = 0`：等待进程组 ID 与当前进程相同的子进程。
      - `pid > 0`：等待进程 ID 等于 `pid` 的子进程。
    - `status`：用于获取子进程状态的指针，常通过以下宏解析：
      - `WIFEXITED(status)`：子进程正常退出时返回非零值，可通过 `WEXITSTATUS(status)` 获取退出状态码。
      - `WIFSIGNALED(status)`：子进程因未捕获信号终止时返回非零值，`WTERMSIG(status)` 可获取终止信号编号。
      - `WIFSTOPPED(status)`：子进程被暂停时返回非零值，`WSTOPSIG(status)` 用于获取暂停信号编号。
    - `options`：控制waitpid行为，常用选项：
      - `WNOHANG`：若子进程未结束，立即返回 `0`（非阻塞）；若结束，返回子进程 PID。
      - `WUNTRACED`：子进程进入暂停状态时立即返回（默认只关注终止子进程）。

    ### 3. 返回值

    - 成功：返回终止或暂停子进程的 PID（`options` 为 `WNOHANG` 且子进程未结束时返回 `0`）。
    - 失败：返回 `-1`，如无可用子进程（`errno` 设为 `ECHILD`）或调用被信号中断（`errno` 设为 `EINTR`）。

    ### 4. 示例代码

    ```c
    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
    
    int main() {
        pid_t pid, childpid;
        int status;
    
        pid = fork(); // 创建子进程
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) { // 子进程
            sleep(2);
            exit(42); // 正常退出，状态码 42
        } else { // 父进程
            do {
                // 非阻塞等待，避免父进程一直阻塞
                childpid = waitpid(pid, &status, WNOHANG);
                if (childpid == 0) {
                    printf("子进程未结束，继续执行其他任务...\n");
                    sleep(1);
                }
            } while (childpid == 0);
    
            if (WIFEXITED(status)) {
                printf("子进程正常退出，退出状态码: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("子进程被信号 %d 终止\n", WTERMSIG(status));
            }
        }
        return 0;
    }
    ```

    ### 5. 注意事项

    - **避免僵尸进程**：父进程应及时调用 `waitpid` 回收子进程，否则子进程会成为僵尸进程，浪费系统资源。
    - **非阻塞模式**：结合 `WNOHANG` 可让父进程在等待子进程时不被阻塞，继续执行其他任务。
    - **信号处理**：子进程终止时，父进程会收到 `SIGCHLD` 信号，可在信号处理函数中调用 `waitpid` 回收子进程（需注意信号处理函数的可重入性）。

    通过合理使用 `waitpid`，可有效管理子进程生命周期，确保系统资源合理利用。


**孤儿进程**

- 父进程结束了，子进程还在运行

| **特征**       | **孤儿进程**                       | **僵尸进程**                             |
| -------------- | ---------------------------------- | ---------------------------------------- |
| **父进程状态** | 已终止，子进程被 `init` 收养       | 仍在运行，但未回收子进程状态             |
| **子进程状态** | 正常运行（非终止），PPID=1         | 已终止，状态为 `Z`（僵尸状态）           |
| **资源回收**   | 由 `init` 自动回收（子进程终止时） | 需父进程调用 `wait()` 回收，否则长期残留 |
| **影响**       | 无负面影响                         | 占用进程表资源，可能导致进程创建失败     |
| **处理方式**   | 无需处理                           | 父进程回收或杀死父进程                   |



# 守护进程

## 流程

在 Linux 系统中，守护进程（Daemon Process）是在后台长期运行、不受终端控制的进程，通常用于执行系统服务任务。以下是创建守护进程的标准流程及要点：

### 一、创建流程

1. **创建子进程，终止父进程**

   - 使用 `fork()` 系统调用创建子进程，随后父进程调用 `exit()` 退出。这样做使子进程成为 “孤儿进程”，被 init 进程收养，形式上脱离终端控制（shell 会认为命令已执行完毕）。

   - 代码示例：

     ```c
     pid_t pid = fork();  
     if (pid < 0) {  
         // 错误处理  
     } else if (pid > 0) {  
         exit(EXIT_SUCCESS); // 父进程退出  
     }  
     // 子进程继续执行  
     ```

2. **子进程创建新会话（关键步骤）**

   - 调用 `setsid()` 函数，使子进程成为新会话的首进程，摆脱原会话、进程组和控制终端的关联。
   - 代码：`setsid();`
   - 作用：确保进程完全独立，不再受原有终端的信号（如 `SIGINT`）影响。

3. **改变工作目录**

   - 使用 `chdir("/")` 将工作目录改为根目录（也可改为其他固定目录如 `/tmp`）。避免因守护进程长期运行导致其工作目录所在文件系统无法卸载。
   - 代码：`chdir("/");`

4. **重设文件创建掩码**

   - 调用 `umask(0)` 清除文件创建掩码，确保守护进程创建文件 / 目录时拥有所需权限（避免继承父进程的掩码限制）。

5. **关闭文件描述符**

   - 关闭从父进程继承的已打开文件描述符，释放资源。可通过循环关闭：

     ```c
     #include <sys/resource.h>  
     int maxfd = sysconf(_SC_OPEN_MAX);  
     for (int i = 0; i < maxfd; i++) {  
         close(i);  
     }  
     ```

   - 进一步将标准输入、输出、错误（0、1、2）重定向到/dev/null，防止守护进程进行无效 I/O 操作：

     ```c
     int fd = open("/dev/null", O_RDWR);  
     dup2(fd, STDIN_FILENO);  
     dup2(fd, STDOUT_FILENO);  
     dup2(fd, STDERR_FILENO);  
     if (fd > 2) close(fd);  
     ```

### 二、核心要点

- **脱离终端控制**：通过 `fork()` 后父进程退出、`setsid()` 新建会话，确保守护进程不依赖任何终端。
- **资源清理与独立性**：关闭不必要文件描述符、重设工作目录和文件掩码，避免资源浪费或依赖外部环境（如可卸载的文件系统）。
- **代码健壮性**：添加错误处理（如 `fork()`、`setsid()` 失败时的处理），确保守护进程稳定启动。

### 三、示例代码框架

```c
#include <stdio.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>  

int main() {  
    pid_t pid = fork();  
    if (pid < 0) {  
        perror("fork");  
        return 1;  
    } else if (pid > 0) {  
        exit(EXIT_SUCCESS);  
    }  

    if (setsid() == (pid_t)-1) {  
        perror("setsid");  
        return 1;  
    }  

    if (chdir("/") == -1) {  
        perror("chdir");  
        return 1;  
    }  

    umask(0);  

    int fd = open("/dev/null", O_RDWR);  
    if (fd < 0) {  
        perror("open /dev/null");  
        return 1;  
    }  
    dup2(fd, STDIN_FILENO);  
    dup2(fd, STDOUT_FILENO);  
    dup2(fd, STDERR_FILENO);  
    if (fd > 2) close(fd);  

    // 守护进程核心业务逻辑（如循环执行任务）  
    while (1) {  
        // 具体任务代码  
        sleep(1);  
    }  

    return 0;  
}  
```

通过以上步骤，可将普通进程改造为守护进程，使其在后台稳定运行，执行如日志记录、定时任务、网络监听等系统服务功能。

## 日志输出

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

// 记录日志到文件
void log_to_file(const char *message) {
    FILE *log_file = fopen("/var/log/my_daemon.log", "a");
    if (log_file == NULL) {
        return;
    }

    // 获取当前时间
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time_str[80];
    strftime(time_str, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    // 写入日志
    fprintf(log_file, "[%s] %s\n", time_str, message);
    fclose(log_file);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() == (pid_t)-1) {
        perror("setsid");
        return 1;
    }

    if (chdir("/") == -1) {
        perror("chdir");
        return 1;
    }

    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
        perror("open /dev/null");
        return 1;
    }
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > 2) close(fd);

    // 记录日志
    log_to_file("Daemon started");

    // 守护进程核心业务逻辑（如循环执行任务）
    while (1) {
        log_to_file("Daemon is running");
        sleep(10);
    }

    return 0;
}
```



## 守护进程实践

- 守护进程（类似top）
  - signal的使用
- 守护进程（监视文件修改）
  - stat的使用











TODO：进程通信、线程以及锁机制 	 	
