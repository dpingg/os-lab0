/* start.S的主要功能是切换在实模式工作的处理器到32位保护模式。为此需要设置正确的
 * GDT、段寄存器和CR0寄存器。C语言代码的主要工作是将磁盘上的内容装载到内存中去。
 * 磁盘镜像的结构如下：
	 +-----------+------------------.        .-----------------+
	 |   引导块   |  游戏二进制代码       ...        (ELF格式)     |
	 +-----------+------------------`        '-----------------+
 * C代码将游戏文件整个加载到物理内存0x100000的位置，然后跳转到游戏的入口执行。 */

#include "boot.h"

#define SECTSIZE 512

void readseg(unsigned char *, int, int);

void bootmain(void) {
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	unsigned char* pa, *i;

	/* 因为引导扇区只有512字节，我们设置了堆栈从0x8000向下生长。
	 * 我们需要一块连续的空间来容纳ELF文件头，因此选定了0x8000。 */
	elf = (struct ELFHeader*)0x8000;

	/* 读入ELF文件头 */
	readseg((unsigned char*)elf, 4096, 0);

	/* 把每个program segement依次读入内存 */
	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph ++) {
		/* ************************************* */
		pa = (unsigned char*)ph->paddr; /* 获取物理地址 */
		readseg(pa, ph->filesz, ph->off); /* 读入数据 */
		for (i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
	}

	((void(*)(void))elf->entry)();
}


/*
#define HD_PORT_DATA            0x1f0
#define HD_PORT_ERROR           0x1f1
#define HD_PORT_SECT_COUNT      0x1f2
#define HD_PORT_SECT_NUM        0x1f3
#define HD_PORT_CYL_LOW         0x1f4
#define HD_PORT_CYL_HIGH        0x1f5
#define HD_PORT_DRV_HEAD        0x1f6
#define HD_PORT_STATUS          0x1f7
#define HD_PORT_COMMAND         0x1f7
看起来很恐怖，但是它被组织的很清晰，我们从0x1F0读取数据，
如果有任何错误，我们从0x1F1中获取错误状态，通过0x1F2和0x1F3设置扇区数量，
通过0x1F4和0x1F5设置柱面数，剩下的0x1F6设置磁头数， 
我们通过端口0x1F7读取磁盘状态，我们通过端口0x1F7发送读写命令。

#define HD_READ         0x20
#define HD_WRITE        0x30
*/


void waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0x40); /* 等待磁盘完毕 --等待硬盘状态，直到可以写或读为止，*/
}

/* 读磁盘的一个扇区 */
void readsect(void *dst, int offset) {
	int i;
	waitdisk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x20);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}

/* 将位于磁盘offset位置的count字节数据读入物理地址pa */
void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
