



# Makefile基础

---

参考博客：

[C语言-Makefile经典教程(掌握这些足够) - John_ABC - 博客园](https://www.cnblogs.com/JohnABC/p/4568459.html)

[【C语言】- Makefile | 白菜君の技术库](https://whiteccinn.github.io/2019/07/08/C语言/Makefile/)

---

## 书写规则

```makefile
demo: demo.c
	gcc demo.c -o demo
clean:
	rm demo
---------------------------------------
编译目标:依赖文件
	编译命令
---------------------------------------
1、当执行make命令时，会自动寻找第一个目标执行
2、若第一个目标其依赖文件不存在或有更新，则执行目标依赖文件的命令，依次递归
3、在命令之前添加@表示执行时不显示该命令，只显示执行结果
	在命令前添加-表示若该命令执行出错，忽略继续执行之后的命令
```

## Makefile中的变量

```makefile
cflag=-c
demo: demo.o
	gcc demo.o -o demo
demo.o:demo.c
	gcc cflag demo.c -o demo.o
clean:
	rm demo
---------------------------------------
1、变量理解为c语言中的宏定义
	如果要展开通配符中的文件，可用wildcard函数
	name=$(wildcard *.c)
2、 :=表示简单赋值和=类似
	+=表示追加赋值
	?=表示判断赋值（判断左边是否已经定义过，如果是则忽略本次赋值）
```

## 伪目标

```makefile
正像我们前面例子中的“clean”一样，即然我们生成了许多文件编译文件，我们也应该提供一个清除它们的“目标”以备完整地重编译而用。 （以“make clean”来使用该目标）
因为，我们并不生成“clean”这个文件。“伪目标”并 不是一个文件，只是一个标签，由于“伪目标”不是文件，所以make无法生成它的依赖关系和决定它是否要执行。我们只有通过显示地指明这个“目标”才能让 其生效。当然，“伪目标”的取名不能和文件名重名，不然其就失去了“伪目标”的意义了。

用.PHONY:clean表示伪目标，用空格隔开多个伪目标
```

## 通配符

- *、?、[string]、%

- 重点考虑%

  目前有Makefile文件如下：

  ```makefile
  cflags=-c
   
  main: demo.o myprint.o
  	gcc demo.o myprint.o -o main
   
  %.o: %.c 
  	gcc $(cflags) $<
  clean:
  	rm myprint.o demo.o main
  .PHONY: clean
  ```

  其等价与如下Makefile文件(有几个文件就将产生对应数目的编译目标，而不是将他们都放在一起)：

  ```makefile
  cflags=-c
   
  main: demo.o myprint.o
  	gcc demo.o myprint.o -o main
   
  demo.o:demo.c
  	gcc $(cflags) demo.c 
   
  myprint.o:myprint.c
  	gcc $(cflags) myprint.c 
   
  clean:
  	rm myprint.o demo.o main
  .PHONY: clean
  ```

  并不等价如下：

  ```makefile
  cflags=-c
   
  main: demo.o myprint.o
  	gcc demo.o myprint.o -o main
   
  demo.o myprint.o:demo.c myprint.c
  	gcc $(cflags) demo.c 
   
  clean:
  	rm myprint.o demo.o main
  .PHONY: clean
  ```

  

## VPATH

1. **定义 VPATH**：
    你可以通过以下方式定义 `VPATH`：

   ```
   VPATH = dir1:dir2:dir3
   ```

   这里，`dir1`、`dir2` 和 `dir3` 是你希望 Make 查找文件的目录，多个目录之间用冒号 `:` 分隔。
   
2. **示例**：
    假设你有以下目录结构：

   ```
   src/
       main.c
       utils.c
   include/
       utils.h
   ```
   
   你可以在 Makefile 中这样设置 `VPATH`：

   ```
   VPATH = src:include
   
   all: main.o utils.o
   
   %.o: %.c
       gcc -c $< -o $@
   ```
   
   在这个例子中，Make 会在 `src` 和 `include` 目录中查找 `.c` 文件和 `.h` 文件。
   
3. **清除 VPATH**：
    如果需要清除某个模式的搜索路径，可以使用：

   ```
   vpath %.c
   ```

   这将清除所有与 `.c` 文件相关的搜索路径。
   
4. **注意事项**

   若想要VPATH生效，在命令中使用$<等命令，不能直接执行文件名

## ifeq和ifneq

在 Makefile 中，`ifeq` 和 `ifneq` 是用于条件判断的指令，允许根据变量的值执行不同的操作。以下是它们的用法和示例：

### 1. `ifeq` 的用法
`ifeq` 用于判断两个参数是否相等。其基本语法如下：
```makefile
ifeq (arg1, arg2)
    # 如果 arg1 和 arg2 相等，执行的命令
else
    # 如果 arg1 和 arg2 不相等，执行的命令
endif
```
- **示例**：
```makefile
CC = gcc

ifeq ($(CC), gcc)
    LIBS = -lm
else
    LIBS = 
endif

all:
    $(CC) main.c -o main $(LIBS)
```
在这个示例中，如果 `CC` 的值是 `gcc`，则会链接数学库 `-lm`。

### 2. `ifneq` 的用法
`ifneq` 用于判断两个参数是否不相等。其基本语法如下：
```makefile
ifneq (arg1, arg2)
    # 如果 arg1 和 arg2 不相等，执行的命令
else
    # 如果 arg1 和 arg2 相等，执行的命令
endif
```
- **示例**：
```makefile
VERSION = 1.0

ifneq ($(VERSION), 1.0)
    MESSAGE = "Version is not 1.0"
else
    MESSAGE = "Version is 1.0"
endif

all:
    @echo $(MESSAGE)
```
在这个示例中，如果 `VERSION` 的值不是 `1.0`，则会输出 "Version is not 1.0"。

### 注意事项
- `ifeq` 和 `ifneq` 的条件判断是在 Makefile 解析时进行的，因此不能使用自动化变量（如 `$@`、`$<` 等）作为条件判断的参数，因为这些变量在规则执行时才会被赋值。
- 这些条件语句可以嵌套使用，以实现更复杂的逻辑判断。

通过使用 `ifeq` 和 `ifneq`，你可以根据不同的条件灵活地控制 Makefile 的行为，从而提高构建过程的灵活性和可维护性。

---
Learn more:
1. [makefile中的条件判断ifeq、ifneq、ifdef\_makefile ifeq-CSDN博客](https://blog.csdn.net/nyist327/article/details/42552743)
2. [浅显易懂 Makefile 入门 （04）- 条件判断 （ifeq、ifneq、ifdef 和 ifndef）、定义命令包 define 和伪目标-CSDN博客](https://blog.csdn.net/wohu1104/article/details/111022292)
3. [Makefile 中 ifeq ifneq 等用法 - Malphite - 博客园](https://www.cnblogs.com/Malphite/p/10302375.html)

## ifdef和ifndef

在 Makefile 中，`ifdef` 和 `ifndef` 是用于条件判断的指令，主要用于检查变量是否已定义或是否为空。以下是它们的用法和示例：

### 1. `ifdef` 的用法
`ifdef` 用于判断一个变量是否已定义。如果变量存在（即使它的值为空），条件为真。

- **基本语法**：
  ```makefile
  ifdef VARIABLE_NAME
      # 如果 VARIABLE_NAME 已定义，执行的命令
  else
      # 如果 VARIABLE_NAME 未定义，执行的命令
  endif
  ```

- **示例**：
  ```makefile
  MY_VAR = 
  
  ifdef MY_VAR
      MESSAGE = "MY_VAR is defined"
  else
      MESSAGE = "MY_VAR is not defined"
  endif
  
  all:
      @echo $(MESSAGE)
  ```
  在这个示例中，`MY_VAR` 被定义但为空，因此输出将是 "MY_VAR is defined" [[1]](https://blog.csdn.net/weixin_39662684/article/details/107538194).

### 2. `ifndef` 的用法
`ifndef` 用于判断一个变量是否未定义。如果变量不存在，条件为真。

- **基本语法**：
  ```makefile
  ifndef VARIABLE_NAME
      # 如果 VARIABLE_NAME 未定义，执行的命令
  else
      # 如果 VARIABLE_NAME 已定义，执行的命令
  endif
  ```

- **示例**：
  ```makefile
  MY_VAR = 
  
  ifndef MY_VAR
      MESSAGE = "MY_VAR is not defined"
  else
      MESSAGE = "MY_VAR is defined"
  endif
  
  all:
      @echo $(MESSAGE)
  ```
  在这个示例中，由于 `MY_VAR` 被定义（尽管为空），因此输出将是 "MY_VAR is defined" [[2]](https://bbs.huaweicloud.com/blogs/307734).

### 注意事项
- `ifdef` 和 `ifndef` 只能用于检查变量是否存在，不能用于检查变量的具体值。
- 这些条件判断在 Makefile 解析时进行，因此不能使用自动化变量（如 `$@`、`$<` 等）作为条件判断的参数。

通过使用 `ifdef` 和 `ifndef`，你可以根据变量的定义状态灵活地控制 Makefile 的行为，从而提高构建过程的灵活性和可维护性。

---
Learn more:
1. [makefile ifdef用法-CSDN博客](https://blog.csdn.net/weixin_39662684/article/details/107538194)
2. [10.Makefile ifeq、ifneq、ifdef和ifndef-云社区-华为云](https://bbs.huaweicloud.com/blogs/307734)
3. [(9)Makefile的条件判断 - Mr\_chuan - 博客园](https://www.cnblogs.com/shaochuanhe/articles/14491828.html)

## 函数

---

只会记录用到过的函数

---







## 嵌套makefile

在一些大的工程中，我们会把我们不同模块或是不同功能的源文件放在不同的目录中，我们可以在每个目录中都书写一个该目录的 Makefile，这有利于让我们的 Makefile 变得更加地简洁，而不至于把所有的东西全部写在一个 Makefile 中，这样会很难维护我们的 Makefile，这个技术对于我们模块编译和分段编译有着非常大的好处。

例如，我们有一个子目录叫 subdir，这个目录下有个 Makefile 文件，来指明了这个目录下文件的编译规则。那么我们`总控的 Makefile` 可以这样书写：

```
subsystem:
  cd subdir && $(MAKE)
```

其等价于：

```
subsystem:在Makefile中，wildcard函数用于匹配文件名模式，并返回符合该模式的文件列表。它的语法如下：
$(wildcard pattern)

其中，pattern是一个文件名模式，可以包含通配符*（匹配任意数量的字符）和?（匹配单个字符）。wildcard函数会返回当前目录下所有符合该模式的文件名列表。
示例
假设当前目录下有如下文件：
main.c
utils.c
utils.h
README.md

你可以使用wildcard函数来获取所有.c文件：
SRC_FILES := $(wildcard *.c)

执行后，SRC_FILES变量的值将是：
main.c utils.c

常见用法

获取所有源文件：
SRC_FILES := $(wildcard *.c)


获取所有头文件：
HEADER_FILES := $(wildcard *.h)


结合多个模式：
ALL_FILES := $(wildcard *.c *.h)


在多个目录中查找文件：
SRC_FILES := $(wildcard src/*.c include/*.h)



注意事项

wildcard函数只在Makefile解析时展开，因此它不会动态地检测文件系统的变化。
如果没有任何文件匹配指定的模式，wildcard函数将返回空字符串。

示例Makefile
SRC_FILES := $(wildcard *.c)
OBJ_FILES := $(SRC_FILES:.c=.o)

all: program

program: $(OBJ_FILES)
    gcc -o $@ $^

%.o: %.c
    gcc -c $< -o $@

clean:
    rm -f $(OBJ_FILES) program

在这个示例中，wildcard函数用于获取所有.c文件，然后通过模式替换生成对应的.o文件列表，最终编译并链接生成可执行文件program。
  $(MAKE) -C subdir
```

定义$(MAKE)宏变量的意思是，也许我们的 make 需要一些参数，所以定义成一个变量比较利于维护。这两个例子的意思都是先进入“subdir”目录，然后执行 make 命令。

我们把这个 Makefile 叫做“总控 Makefile”，`总控 Makefile 的变量可以传递到下级的 Makefile 中（如果你显示的声明），但是不会覆盖下层的 Makefile 中所定义的变量，除非指定了“-e”参数。`

如果你要传递变量到下级 Makefile 中，那么你可以使用这样的声明：

```
export<variable ...>
```

如果你不想让某些变量传递到下级 Makefile 中，那么你可以这样声明：

```
unexport<variable ...>
```

**示例一：**

```
export variable = value
```

其等价于：

```
variable = value

export variable
```

其等价于：

```
export variable := value
```

其等价于：

```
variable := value

export variable
```

**示例二：**

```
export variable += value
```

其等价于：

```
variable += value

export variable
```

`如果你要传递所有的变量，那么，只要一个export就行了`。后面什么也不用跟，表示传递所有的变量。

需要注意的是，有两个变量，一个是 `SHELL`，一个是`MAKEFLAGS`，这两个变量不管你是否 export，其总是要传递到下层 Makefile 中，`特别是 MAKEFILES 变量，其中包含了 make 的参数信息`，如果我们执行“总控 Makefile”时有 make 参数或是在上层 Makefile 中定义了这个变量，那么 MAKEFILES 变量将会是这些参数，并会传递到下层 Makefile 中，这是一个系统级的环境变量。

但是 make 命令中的有几个参数并不往下传递，它们是“-C”,“-f”,“-h”“-o”和“-W”（有关 Makefile 参数的细节将在后面说明），`如果你不想往下层传递参数`，那么，你可以这样来：

```
subsystem:
  cd subdir && $(MAKE) MAKEFLAGS=
```

如果你定义了环境变量 MAKEFLAGS，那么你得确信其中的选项是大家都会用到的，如果其中有“-t”,“-n”,和“-q”参数，那么将会有让你意想不到的结果，或许会让你异常地恐慌。

还有一个在“嵌套执行”中比较有用的参数，`“-w”或是“--print-directory”会在 make 的过程中输出一些信息`，让你看到目前的工作目录。比如，如果我们的下级 make 目录是“/home/ccinn/gnu/make”，如果我们使用“make -w”来执行，那么当进入该目录时，我们会看到：

```
make: Entering directory `/home/ccinn/gnu/make'.
```

而在完成下层 make 后离开目录时，我们会看到：

```
make: Leaving directory `/home/ccinn/gnu/make'
```

当你使用“-C”参数来指定 make 下层 Makefile 时，“-w”会被自动打开的。

## 自动变量

```makefile
1、$<	第一个依赖文件名称
2、$@	目标文件的完整名称
3、$^	所有不重复的依赖文件，空格隔开
```



# GDB调试

## 命令行调试

在学习之前通过make编译得到可执行文件后，使用gdb调式可执行文件

```
1、使用格式
	gdb [选项] [可执行程序]
	常用选项（不重要）：
		-c\-h\-n\-q\-s
2、进入gdb后命令
	l	list	列出调试源码
	b	break	打断点，b 19表示在第19行打断点
	r	run		运行
	n	next	下一过程，不进入函数
	s	step	下一步，进入函数
	p	print	显示变量值，p 变量名
	info break	显示断点信息
	info local	显示全部的局部变量
	q	quit	退出
```

## VSCODE调试

使用命令行gdb调试麻烦，通过vscode调试方便

在 Linux 中使用 Visual Studio Code (VSCode) 调试可执行文件的步骤如下：

### 1. 编译可执行文件
确保你的可执行文件包含调试信息。使用 `gcc` 编译时，添加 `-g` 选项。例如：
```bash
gcc -g -o my_program my_program.c
```
这将生成一个名为 `my_program` 的可执行文件，并包含调试信息。

### 2. 安装 C/C++ 扩展
在 VSCode 中，确保安装了 C/C++ 扩展。可以通过以下步骤进行安装：
- 打开 VSCode。
- 按下 `Ctrl + Shift + X` 打开扩展视图。
- 搜索 "C/C++" 并安装由 Microsoft 提供的扩展。

### 3. 创建调试配置
- 打开你的项目文件夹。
- 按下 `Ctrl + Shift + D` 打开运行和调试视图。
- 点击 "create a launch.json file"。
- 选择 "C++ (GDB/LLDB)" 作为环境。

### 4. 配置 `launch.json`
在自动生成的 `launch.json` 文件中，找到 `program` 字段并设置为你的可执行文件路径。例如：
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/my_program",
            "args": [], // 参数传递用
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```
确保 `program` 字段指向你的可执行文件。

### 5. 开始调试
- 在调试视图中，选择刚刚创建的配置（如 "(gdb) Launch"）。
- 点击绿色的开始按钮，VSCode 将启动调试会话。

### 6. 设置断点
在代码中点击行号可以设置断点，调试时程序会在这些断点处暂停，方便你检查变量和程序状态。

### 7. 清理和运行
使用 `clean` 目标清理生成的文件，确保在每次调试前都使用最新的可执行文件。

通过以上步骤，你可以在 Linux 中使用 VSCode 调试可执行文件，享受图形化调试的便利，而不必依赖命令行工具如 GDB。

---
Learn more:
1. [在linux 下通过vscode调试elf可执行文件 - Jay's some notes](https://notes.leconiot.com/debug_elf_with_vscode.html)
2. [\[Linux\]用VScode 调试Linux命令行带参数可执行程序\_vscode命令行调试-CSDN博客](https://blog.csdn.net/wangyijieonline/article/details/84943093)
3. [vscode调试cmake生成的可执行文件\_vscode运行可执行文件-CSDN博客](https://blog.csdn.net/ergevv/article/details/139751251)

## .vscode中的文件说明

- launch.json
  - 调试运行用
- c_cpp_properties.json
  - 头文件用
- settings.json
  - 键值对存储形式，用来存数据库信息等，可用被解析