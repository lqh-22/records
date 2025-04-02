CFLAG = -g
OUT_DIR = ./out
OUT1 = $(OUT_DIR)/server
OUT2 = $(OUT_DIR)/client

all:$(OUT1) $(OUT2)

$(OUT1):server.c
	@mkdir -p $(OUT_DIR)  # 创建输出目录（如果不存在）
	gcc $(CFLAG) server.c -o $(OUT1)
$(OUT2):client.c
	@mkdir -p $(OUT_DIR)  # 创建输出目录（如果不存在）
	gcc $(CFLAG) client.c -o $(OUT2)

clean:
	rm -rf $(OUT1) $(OUT2)