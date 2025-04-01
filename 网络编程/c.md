- [ ] const使用
- [ ] 字符串操作函数
- [ ] 文件IO
- [ ] 网络编程





## 标准输入输出/标准错误输出

在 C 和 C++ 中，`stdin`、`stdout` 和 `stderr` 是三个标准的输入输出流，它们在程序的输入和输出操作中扮演着重要角色。

### 1. `stdin`（标准输入）

- **定义**：`stdin` 是标准输入流，通常与终端的键盘相对应。它用于接收用户输入。
- **使用**：可以通过 `scanf`、`fgets` 等函数从 `stdin` 读取数据。
- **文件描述符**：在 C 语言中，`stdin` 的文件描述符为 0。

### 2. `stdout`（标准输出）

- **定义**：`stdout` 是标准输出流，通常与终端的屏幕相对应。它用于输出程序的正常结果。
- **使用**：可以通过 `printf`、`puts`、`fputs` 等函数向 `stdout` 输出数据。
- **缓冲机制**：`stdout` 默认是行缓冲的，这意味着输出会先存储在缓冲区中，直到遇到换行符或缓冲区满时才会被实际输出到屏幕上。
- **文件描述符**：在 C 语言中，`stdout` 的文件描述符为 1。

### 3. `stderr`（标准错误输出）

- **定义**：`stderr` 是标准错误输出流，通常也与终端的屏幕相对应。它用于输出错误信息和警告。
- **使用**：可以通过 `fprintf(stderr, ...)` 或 `perror` 等函数向 `stderr` 输出错误信息。
- **无缓冲输出**：`stderr` 是无缓冲的，这意味着输出到 `stderr` 的信息会立即显示，适合用于错误信息的及时反馈。
- **文件描述符**：在 C 语言中，`stderr` 的文件描述符为 2。
- **和perror类似都是标准错误输出**

### 4. 重定向

- 这三个流都可以被重定向到文件或其他设备。例如，可以将 `stdout` 重定向到一个文件，以便将程序的输出保存到文件中，而 `stderr` 可以单独重定向，以便将错误信息输出到另一个文件。
- 示例命令：
  - `./program > output.txt` 将 `stdout` 重定向到 `output.txt`。
  - `./program 2> error.txt` 将 `stderr` 重定向到 `error.txt`。
  - `./program > output.txt 2>&1` 将 `stdout` 和 `stderr` 都重定向到同一个文件。

## fprintf和fscanf

`fprintf` 和 `fscanf` 是 C 语言中用于格式化输出和输入的函数，主要用于文件操作。以下是这两个函数的详细使用方法：

### 1. `fprintf` 函数(printf)
- **功能**：将格式化的数据写入指定的文件流。
- **函数原型**：
  ```c
  int fprintf(FILE *fp, const char *format, ...);
  ```
- **参数**：
  - `fp`：文件指针，指向要写入的文件。
  - `format`：格式控制字符串，指定输出的格式。
  - 后续参数：要写入的数据。

- **示例**：
  ```c
  FILE *fp = fopen("output.txt", "w");
  if (fp != NULL) {
      int i = 10;
      float f = 3.14;
      fprintf(fp, "Integer: %d, Float: %.2f\n", i, f);
      fclose(fp);
  }
  ```
  在这个例子中，整数和浮点数将被写入到 `output.txt` 文件中。

### 2. `fscanf` 函数(scanf)
- **功能**：从指定的文件流中读取格式化的数据。
- **函数原型**：
  ```c
  int fscanf(FILE *fp, const char *format, ...);
  ```
- **参数**：
  
  - `fp`：文件指针，指向要读取的文件。
  - `format`：格式控制字符串，指定输入的格式。
- 后续参数：用于存储读取数据的变量的地址。
  
- **示例**：
  ```c
  FILE *fp = fopen("input.txt", "r");
  if (fp != NULL) {
      int i;
      float f;
      fscanf(fp, "Integer: %d, Float: %f", &i, &f);
      printf("Read Integer: %d, Read Float: %.2f\n", i, f);
      fclose(fp);
  }
  ```
  在这个例子中，程序从 `input.txt` 文件中读取一个整数和一个浮点数，并将其打印到控制台。

### 注意事项
- 使用 `fprintf` 时，确保文件已成功打开，并在写入后关闭文件。
- 使用 `fscanf` 时，确保格式字符串与文件中的数据格式匹配，以避免读取错误。

这两个函数在处理文件输入输出时非常有用，能够以格式化的方式读写数据，便于数据的存储和处理

## sprintf和sscanf【字符串处理】

`sprintf` 和 `sscanf` 是 C 语言中用于字符串处理的函数，分别用于格式化输出和格式化输入。以下是这两个函数的详细使用方法和示例。

### 1. `sprintf` 函数
- **功能**：将格式化的数据写入字符串。
- **函数原型**：
  ```c
  int sprintf(char *str, const char *format, ...);
  ```
- **参数**：
  - `str`：指向存储结果的字符数组。
  - `format`：格式控制字符串，指定输出的格式。
  - 后续参数：要写入的数据。

- **示例**：
  ```c
  char buffer;
  int num = 42;
  sprintf(buffer, "The answer is %d", num);
  printf("%s\n", buffer);  // 输出: The answer is 42
  ```
  在这个例子中，整数 `num` 被格式化为字符串并存储在 `buffer` 中。

### 2. `sscanf` 函数
- **功能**：从字符串中读取格式化的数据。
- **函数原型**：
  ```c
  int sscanf(const char *str, const char *format, ...);
  ```
- **参数**：
  - `str`：指向要读取的字符串。
  - `format`：格式控制字符串，指定输入的格式。
  - 后续参数：用于存储读取数据的变量的地址。

- **示例**：
  ```c
  const char *input = "42 3.14 Hello";
  int num;
  float f;
  char str;
  sscanf(input, "%d %f %s", &num, &f, str);
  printf("Number: %d, Float: %.2f, String: %s\n", num, f, str);
  // 输出: Number: 42, Float: 3.14, String: Hello
  ```
  在这个例子中，`sscanf` 从字符串 `input` 中读取一个整数、一个浮点数和一个字符串。

### 注意事项
- 使用 `sprintf` 时要确保目标字符串有足够的空间来存储格式化后的结果，以避免缓冲区溢出。可以使用 `snprintf` 来限制写入的字符数，从而提高安全性。
- 使用 `sscanf` 时，确保格式字符串与输入字符串的格式匹配，以避免读取错误。

这两个函数在处理字符串和数据转换时非常有用，能够简化数据的格式化和解析过程 

## fgets和fpus

`fgets` 和 `fputs` 是 C 语言中用于处理字符串输入和输出的函数。它们的主要功能和用法如下：

### 1. `fgets` 函数(gets)
- **功能**：从指定的文件流中读取一行字符串。
- **函数原型**：
  
  ```c
  char *fgets(char *str, int n, FILE *stream);
  ```
- **参数**：
  
  - `str`：指向字符数组的指针，用于存储读取的字符串。
  - `n`：要读取的最大字符数（包括结束的空字符）。
- `stream`：指向 `FILE` 对象的指针，标识要从中读取字符的流（例如 `stdin` 表示标准输入）。
  
- **返回值**：成功时返回 `str` 的指针，失败或到达文件末尾时返回 `NULL`。

- **示例**：
  ```c
  #include <stdio.h>

  int main() {
      char buffer;
      printf("请输入一行文本：");
      if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
          printf("您输入的文本是：%s", buffer);
      }
      return 0;
  }
  ```
  在这个例子中，`fgets` 从标准输入读取一行文本并存储在 `buffer` 中。

### 2. `fputs` 函数(puts)
- **功能**：将字符串写入指定的文件流。
- **函数原型**：
  ```c
  int fputs(const char *str, FILE *stream);
  ```
- **参数**：
  - `str`：要写入的字符串。
  - `stream`：指向 `FILE` 对象的指针，标识要写入的流（例如 `stdout` 表示标准输出）。

- **返回值**：成功时返回非负值，失败时返回 `EOF`。

- **示例**：
  ```c
  #include <stdio.h>

  int main() {
      const char *text = "Hello, World!";
      fputs(text, stdout);  // 将字符串输出到标准输出
      return 0;
  }
  ```
  在这个例子中，`fputs` 将字符串 `"Hello, World!"` 输出到标准输出。

### 注意事项
- `fgets` 会包括换行符在内，如果读取的行长度小于 `n-1`，则换行符会被存储在 `str` 中。
- `fputs` 不会自动添加换行符，因此如果需要换行，必须手动添加。

这两个函数在处理字符串输入输出时非常有用，能够以格式化的方式读写数据，便于数据的存储和处理。



## string函数库

### strcat函数

`strcat` 函数是 C 语言中的一个标准库函数，用于将一个字符串追加到另一个字符串的末尾。它的基本用法和注意事项如下：

### 函数原型
```c
char *strcat(char *dest, const char *src);
```

### 参数
- `dest`：指向目标字符串的指针，该字符串必须有足够的空间来容纳追加后的结果。
- `src`：指向要追加的源字符串的指针。

### 返回值
- `strcat` 返回一个指向目标字符串 `dest` 的指针。

### 使用示例
以下是一个简单的示例，演示如何使用 `strcat` 函数：

```c
#include <stdio.h>
#include <string.h>

int main() {
    char dest = "Hello, ";
    char src[] = "World!";
    
    strcat(dest, src);  // 将 src 追加到 dest 的末尾
    printf("结果字符串: %s\n", dest);  // 输出: Hello, World!
    
    return 0;
}
```

在这个示例中，`strcat` 将字符串 `"World!"` 追加到 `"Hello, "` 的末尾，最终结果为 `"Hello, World!"` [[1]](https://www.runoob.com/cprogramming/c-function-strcat.html)[[2]](https://www.ibm.com/docs/zh/i/7.5?topic=functions-strcat-concatenate-strings).

### 注意事项
- **缓冲区溢出**：确保目标字符串 `dest` 有足够的空间来存储追加后的字符串，否则可能会导致缓冲区溢出。
- **字符串结束符**：`strcat` 会在目标字符串的末尾添加一个空字符 `\0`，以确保字符串的正确结束。

通过合理使用 `strcat`，可以方便地连接多个字符串，但务必注意内存管理和字符串的有效性。

---
Learn more:
1. [C 库函数 - strcat() | 菜鸟教程](https://www.runoob.com/cprogramming/c-function-strcat.html)
2. [strcat ()-并置字符串](https://www.ibm.com/docs/zh/i/7.5?topic=functions-strcat-concatenate-strings)
3. [c语言如何使用strcat函数 | PingCode智库](https://docs.pingcode.com/baike/1531715)





1. 字符串库函数注意是否会自动末尾添加“\0”