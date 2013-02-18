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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <net.h>
#include <i2c.h>

int board_early_init_f(void)
{
#ifdef CONFIG_OC_SIMPLE_SPI
        extern void spi_init(void);
        spi_init();
#endif
	return 0;
}

int checkboard(void)
{
	printf("BOARD: %s\n", CONFIG_BOARD_NAME);
	return 0;
}

phys_size_t initdram(int board_type)
{
	return 0;
}

#ifdef CONFIG_CMD_NET
static int board_get_mac(int index) {
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	uchar enetaddr[6];
	char enetvar[32];

	sprintf(enetvar, index ? "%s%daddr" : "%saddr", "eth", index);

	/* Already set? */
	if(eth_getenv_enetaddr(enetvar, enetaddr))
		return -1;

	/* Read ethaddr from EEPROM */
	/* TODO: Support more than 2 interfaces */
	I2C_SET_BUS(index != 0);
	i2c_init(CONFIG_SYS_I2C_SPEED, 0);

	if (i2c_read(0x50, 0xfa, 1, enetaddr, 6) == 0 && is_valid_ether_addr(enetaddr)) {
		eth_setenv_enetaddr(enetvar, enetaddr);
	}
#endif /* defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C) */
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
	int eth_number = 0;

#ifdef CONFIG_ETHOC
	board_get_mac(eth_number++);
	rc += ethoc_initialize(0, CONFIG_SYS_ETHOC_BASE);
#ifdef CONFIG_SYS_ETHOC_BASE1
	board_get_mac(eth_number++);
	rc += ethoc_initialize(1, CONFIG_SYS_ETHOC_BASE1);
#endif
#ifdef CONFIG_SYS_ETHOC_BASE2
	board_get_mac(eth_number++);
	rc += ethoc_initialize(2, CONFIG_SYS_ETHOC_BASE2);
#endif
#ifdef CONFIG_SYS_ETHOC_BASE3
	board_get_mac(eth_number++);
	rc += ethoc_initialize(3, CONFIG_SYS_ETHOC_BASE3);
#endif
#endif
	return rc;
}
#endif

#ifdef CONFIG_OC_SD
extern int oc_sd_init(bd_t *bis);
int board_mmc_init(bd_t *bis)
{
  return oc_sd_init(bis);
}
#endif
