# Makefile

# 定义变量
CFLAG = -g
OUT_DIR = ./out
OUT1 = $(OUT_DIR)/send
OUT2 = $(OUT_DIR)/receive

# 默认目标
all: $(OUT1) $(OUT2)

# 生成可执行文件
$(OUT1): send.c
	@mkdir -p $(OUT_DIR)  # 创建输出目录（如果不存在）
	gcc $(CFLAG) send.c -o $(OUT1)
$(OUT2): receive.c
	@mkdir -p $(OUT_DIR)  # 创建输出目录（如果不存在）
	gcc $(CFLAG) receive.c -o $(OUT2)

# 清理生成的文件
clean:
	rm -rf $(OUT1) $(OUT2)
