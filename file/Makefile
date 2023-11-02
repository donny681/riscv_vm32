.PHONY:all clean
CROSS_COMPILE?=riscv32-unknown-elf-
PRJ_NAME  ?=c_env

AS 		:= $(CROSS_COMPILE)gcc -x assembler-with-cpp
CC 		:= $(CROSS_COMPILE)gcc
OBJDUMP := $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy
LD		:= $(CROSS_COMPILE)ld
SZ      := $(CROSS_COMPILE)size

DEVICE := -march=rv32i -mabi=ilp32 -mcmodel=medany
# -fno-builtin
CFLAGS  	:=  -static \
				-ffunction-sections \
				-fdata-sections \
				-fno-builtin -fno-builtin-printf  -fno-builtin-malloc -Os

CFLAGS		+= $(DEVICE) -g

OBJFLASGS 	:= --disassemble-all \
				--disassemble-zeroes \
				--section=.text \
				--section=.text.startup \
				--section=.text.init \
				--section=.data

LDFLAGS		:= $(DEVICE) 
LDFLAGS		+= -nostartfiles
LDFLAGS     += -Tc_env.lds -lm
CFLAGS      += -I printf
CFLAGS      += -L /home/yates/riscv/riscv32-gcc/bin/bin/




SRCS       := test.c
# SRCS       += syscall.c
# SRCS       += printf/printf.c

ASMS       := start.S
OBJS       := $(SRCS:.c=.o)
OBJ        := $(ASMS:.S=.o)

LINK_OBJS += $(OBJS) $(OBJ)

all: $(PRJ_NAME).bin $(PRJ_NAME)_dump.s $(PRJ_NAME)_dump.map.s $(PRJ_NAME).hex

$(OBJ): %.o : %.S
	@echo [AS] $<
	@$(AS) $(CFLAGS) -c $< -o $@

$(OBJS): %.o:%.c
	@echo [CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(PRJ_NAME).elf:$(OBJ) $(OBJS) 
	@echo [LD] Linking $@
	$(CC) $(LINK_OBJS) $(LDFLAGS) -o $@

$(PRJ_NAME).bin:$(PRJ_NAME).elf
	$(SZ) $<
	@echo [LD] Linking $@
	@$(OBJCOPY) -O binary $< $@

$(PRJ_NAME)_dump.s:$(PRJ_NAME).elf
	@echo [OD] Objdump $@ $<
	@$(OBJDUMP) -dSh $(OBJFLASGS) $< > $@

$(PRJ_NAME)_dump.map.s:$(PRJ_NAME).elf
	@echo [OD] Objdump -x $@ $<
	@$(OBJDUMP) -x $< > $@

$(PRJ_NAME).hex:$(PRJ_NAME).elf
	$(OBJCOPY) $< -O ihex $@


clean:
	- rm $(OBJS) $(OBJ) $(PRJ_NAME).bin $(PRJ_NAME).elf $(PRJ_NAME).map $(PRJ_NAME)_dump.s