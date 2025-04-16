# Makefile

# 定义变量
CFLAG = -g
OUT_DIR = .
OUT1 = $(OUT_DIR)/client
OUT2 = $(OUT_DIR)/server

# 默认目标
all: $(OUT1) $(OUT2)

# 生成可执行文件
$(OUT1): client.c
	@mkdir -p $(OUT_DIR)  # 创建输出目录（如果不存在）
	gcc $(CFLAG) client.c -o $(OUT1)
$(OUT2): server.c
	@mkdir -p $(OUT_DIR)  # 创建输出目录（如果不存在）
	gcc $(CFLAG) server.c -o $(OUT2)

# 清理生成的文件
clean:
	rm -rf $(OUT1) $(OUT2)
