/*
 * Copyright (C) 2005 Texas Instruments.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/mem.h>

extern int misc_init_r (void);
extern u32 get_mem_type(void);
extern int do_load_serial_bin(ulong , int);

#ifdef CFG_PRINTF
int print_info(void)
{
	printf("\n\nTexas Instruments X-Loader 1.46 ("
			__DATE__ " - " __TIME__ ")\n");
	return 0;
}
#endif

typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
#ifdef CFG_PRINTF
 	serial_init,		/* serial communications setup */
	print_info,
#endif
  	nand_init,		/* board specific nand init */
  	NULL,
};

void start_armboot (void)
{
  	init_fnc_t **init_fnc_ptr;
	uchar *buf;

   	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

#ifdef START_LOADB_DOWNLOAD
	do_load_serial_bin(CFG_LOADADDR, 115200);
#else
	misc_init_r();
	buf =  (uchar*) CFG_LOADADDR;

	int i, size;

	/* first try mmc */
	if (mmc_init(1)) {
		size = file_fat_read("u-boot.bin", buf, 0);
		if (size > 0) {
			printf("Loading u-boot.bin from mmc\n");
			buf += size;
		}
		else printf("No u-boot.bin on mmc\n");
	}

	if (buf == (uchar *)CFG_LOADADDR) {
		printf("Trying to load u-boot.bin from nand\n");
		for (i = NAND_UBOOT_START; i < NAND_UBOOT_END; i+= NAND_BLOCK_SIZE){
			if (!nand_read_block(buf, i))
				buf += NAND_BLOCK_SIZE; /* advance buf ptr */
		}
	}
#endif
	if (buf == (uchar *)CFG_LOADADDR)
		hang();

	/* go run U-Boot and never return */
  	printf("Starting OS Bootloader...\n");
 	((init_fnc_t *)CFG_LOADADDR)();

	/* should never come here */
}

void hang (void)
{
	/* call board specific hang function */
	board_hang();

	/* if board_hang() returns, hange here */
	printf("X-Loader hangs\n");
	for (;;);
}
