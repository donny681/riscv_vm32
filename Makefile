CC = gcc -g
CFLAGS = -Wall -o 

ROOTPATH=./EasyLogger
INCLUDES += -I$(ROOTPATH)/demo/os/linux/easylogger/inc -I$(ROOTPATH)/easylogger/inc -I$(ROOTPATH)/easylogger/plugins/file
# SRCS += EasyLogger/easylogger
SRCS += $(shell find ./EasyLogger/easylogger/src/ -name '*.c')
SRCS +=  $(shell find ./EasyLogger/easylogger/plugins/file/ -name 'elog_file.c')
SRCS +=  $(shell find ./EasyLogger/demo/os/linux/easylogger/port/ -name '*.c')
LIB=-lpthread

SRCS += $(shell find ./src/ -name '*.c')
SRCS += main.c
INCLUDES += -I./include

all: 
	$(shell mkdir build)
	$(CC) $(SRCS) $(CFLAGS) build/main  $(INCLUDES) $(LIB)

clean:
	rm -rf build

run:
	./build/main


run1:
	./build/main ./file/c_env.bin

# test:
#     riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -o build/test.c -o test
# 	riscv64-unknown-elf-objcopy -O binary test test.bin
