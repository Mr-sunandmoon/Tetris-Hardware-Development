CC = arm-none-linux-gnueabi-gcc

OBJS = *.c
LDFLAGS = -lpthread
# 新增 -std=gnu99 支持循环内定义变量
CFLAGS = -std=gnu99

Tetris4:$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	mv Tetris4 /source/rootfs/

.PHONY:clean
clean:
	rm -rf *.o Tetris4