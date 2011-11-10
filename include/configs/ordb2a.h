/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * BOARD/CPU
 */

#define CONFIG_SYS_CLK_FREQ		50000000
#define CONFIG_SYS_RESET_ADDR		0x00000100

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		0x02000000

#define CONFIG_SYS_UART_BASE		0x90000000
#define CONFIG_SYS_UART_FREQ		CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_UART_BAUD		115200

#define CONFIG_BOARD_NAME		"ordb2a" /* custom board name */

#define CONFIG_SYS_NO_FLASH

/*
 * SERIAL
 */
# define CONFIG_SYS_NS16550
# define CONFIG_SYS_NS16550_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	1
# define CONFIG_CONS_INDEX		1
# define CONFIG_SYS_NS16550_COM1	(0x90000000)
# define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_CLK_FREQ

#define CONFIG_BAUDRATE			CONFIG_SYS_UART_BAUD
#define CONFIG_SYS_BAUDRATE_TABLE	{CONFIG_BAUDRATE}
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* Suppress console info */
#define CONSOLE_ARG			"console=console=ttyS0,115200\0"

/*
 * Ethernet
 */
#define CONFIG_ETHOC
#define CONFIG_SYS_ETHOC_BASE		0x92000000


/*
 * TIMER
 */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_OPENRISC_TMR_HZ	100

/*
 * SPI flash
 */
#define CONFIG_SF
#define CONFIG_OC_SIMPLE_SPI
#define CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
/* third parameter seems left over from oc_tiny_spi driver */
#define CONFIG_SYS_SIMPLE_SPI_LIST {{0xb0000000, CONFIG_SYS_CLK_FREQ, 0}}
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_FSL_SF
#define CONFIG_SF_DEFAULT_MODE SPI_MODE_0
#define CONFIG_SF_DEFAULT_SPEED 25000000
#define CONFIG_SPI_FLASH_CS 0
#define CONFIG_CMD_SPI
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_SF

/*
 * Partitioning SPI flash
 */
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
/*
#define CONFIG_RBTREE
#define CONFIG_SYS_USE_UBI
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS*/
#define CONFIG_CMD_MTDPARTS

/*
 * environment in spi flash
 */
#define CONFIG_USE_SPIFLASH
#ifdef CONFIG_USE_SPIFLASH
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE (16<<10)
#define CONFIG_ENV_OFFSET (720<<10)
#define CONFIG_ENV_SECT_SIZE 4096
#define CONFIG_ENV_SPI_BUS 0
#define CONFIG_ENV_SPI_CS 0
#define CONFIG_ENV_SPI_MODE SPI_MODE_0
#define CONFIG_ENV_SPI_MAX_HZ 50000000
#endif


/*
 * SD
 */

/*
 * Memory organisation:
 *
 * RAM start ---------------------------
 *           | ...                     |
 *           ---------------------------
 *           | Stack                   |
 *           ---------------------------
 *           | Global data             |
 *           ---------------------------
 *           | Environment             |
 *           ---------------------------
 *           | Monitor                 |
 * RAM end   ---------------------------
 */
/* We're running in RAM */
#define CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_SYS_MONITOR_LEN	0x40000	/* Reserve 256k */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_SDRAM_BASE + \
				CONFIG_SYS_SDRAM_SIZE - \
				CONFIG_SYS_MONITOR_LEN)

#ifndef CONFIG_USE_SPIFLASH
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE		0x20000 /* Total Size of Environment, 128KB */
#endif
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

/*
 * Global data object and stack pointer
 */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_ENV_ADDR \
					- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_GBL_DATA_ADDR	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_STACK_LENGTH		0x10000 /* 64KB */
#define CONFIG_SYS_MALLOC_LEN		0x400000 /* 4MB */
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_INIT_SP_OFFSET \
					- CONFIG_SYS_STACK_LENGTH \
					- CONFIG_SYS_MALLOC_LEN)
/*
 * MISC
 */
#define CONFIG_SYS_LONGHELP		/* Provide extended help */
#define CONFIG_SYS_PROMPT		"==> "	/* Command prompt	*/
#define CONFIG_SYS_CBSIZE		256	/* Console I/O buf size */
#define CONFIG_SYS_MAXARGS		16	/* Max command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Bootarg buf size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + \
					16)	/* Print buf size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x2000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP_ADDR - 0x20000)
#define CONFIG_CMDLINE_EDITING

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#ifdef CONFIG_CMD_NET
# define CONFIG_NET_MULTI
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#endif

#endif /* __CONFIG_H */
