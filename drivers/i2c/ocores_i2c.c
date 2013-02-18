/*
 * (C) Copyright 2012
 * Henrik Nordstrom, OrSOC, henrik.nordstrom@orsoc.se
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
 *
 * This has been changed substantially by Gerald Van Baren, Custom IDEAS,
 * vanbaren@cideas.com.  It was heavily influenced by LiMon, written by
 * Neil Russell.
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>


/* Register info copied from orpsoc i2c_master_slave.h */

/* Register offsets */
#define I2C_MASTER_SLAVE_PRERlo 0x0	// Clock prescaler register
#define I2C_MASTER_SLAVE_PRERhi 0x1	// Clock prescaler register
#define I2C_MASTER_SLAVE_CTR    0x2	// Control register
#define I2C_MASTER_SLAVE_TXR    0x3	// Transmit register
#define I2C_MASTER_SLAVE_RXR    0x3	// Recive register
#define I2C_MASTER_SLAVE_CR     0x4	// Control register
#define I2C_MASTER_SLAVE_SR     0x4	// Status register
#define I2C_MASTER_SLAVE_SLADR  0x7	// Slave address register

/* CTR bits */
#define I2C_MASTER_SLAVE_CTR_CORE_ENABLE 0x80
#define I2C_MASTER_SLAVE_CTR_INTR_ENABLE 0x40
#define I2C_MASTER_SLAVE_CTR_SLAVE_ENABLE 0x20

/* CR bits */
#define I2C_MASTER_SLAVE_CR_START        0x80
#define I2C_MASTER_SLAVE_CR_STOP         0x40
#define I2C_MASTER_SLAVE_CR_READ         0x20
#define I2C_MASTER_SLAVE_CR_WRITE        0x10
#define I2C_MASTER_SLAVE_CR_ACK          0x08
#define I2C_MASTER_SLAVE_CR_SL_CONT      0x02
#define I2C_MASTER_SLAVE_CR_IACK         0x01

/* SR bits */
#define I2C_MASTER_SLAVE_SR_RXACK            0x80
#define I2C_MASTER_SLAVE_SR_BUSY             0x40
#define I2C_MASTER_SLAVE_SR_ARB_LOST         0x20
#define I2C_MASTER_SLAVE_SR_SLAVE_MODE       0x10
#define I2C_MASTER_SLAVE_SR_SLAVE_DATA_AVAIL 0x08
#define I2C_MASTER_SLAVE_SR_SLAVE_DATA_REQ   0x04
#define I2C_MASTER_SLAVE_SR_TRANSFER_IN_PRG  0x02
#define I2C_MASTER_SLAVE_SR_IRQ_FLAG         0x01

static u32 i2c_base_addrs[] = {
	0xa0000000,
	0xa1000000,
};

static u32 i2c_base = 0;

static int i2c_read_reg(int reg)
{
	u8 value = ((u8 *)i2c_base)[reg];
	debug("I2C read %x = %x\n", reg, value);
	return value;
}

static void i2c_write_reg(int reg, u8 value)
{
	debug("I2C write %x = %x\n", reg, value);
	((u8 *)i2c_base)[reg] = value;
}

void i2c_send_byte(uchar byte, int start, int stop)
{
	i2c_write_reg(I2C_MASTER_SLAVE_TXR, byte);
	i2c_write_reg(I2C_MASTER_SLAVE_CR, (start ? I2C_MASTER_SLAVE_CR_START : 0) | (stop ? I2C_MASTER_SLAVE_CR_STOP : 0) | I2C_MASTER_SLAVE_CR_WRITE | I2C_MASTER_SLAVE_CR_IACK);
}

void i2c_send_stop(void)
{
	/* Do this work? */
	i2c_write_reg(I2C_MASTER_SLAVE_CR, I2C_MASTER_SLAVE_CR_STOP|I2C_MASTER_SLAVE_CR_IACK);
}

static int i2c_wait_while_busy(void)
{
	int timeout = 1000000;
	while (timeout--) {
		if (!(i2c_read_reg(I2C_MASTER_SLAVE_SR) & I2C_MASTER_SLAVE_SR_BUSY))
			return 0;
	
	}
	return -1;
}

/* Waits for transfer to complete, and optionally check received ACK value
 * check_ack = 0	Do not care about ACK value
 * check_ack > 0	Require ACK
 * eheck_ack < 0	Require NACK
 */
static int i2c_transfer_complete(int check_ack)
{
	int timeout = 10000000;
	while (timeout--) {
		int status = i2c_read_reg(I2C_MASTER_SLAVE_SR);
		/* Arbitration lost? */
		if (status & I2C_MASTER_SLAVE_SR_ARB_LOST)
			return -1;
		/* Transfer still in progress? */
		if (status & I2C_MASTER_SLAVE_SR_TRANSFER_IN_PRG)
			continue;
		if (check_ack) {
		       	if (!(status & I2C_MASTER_SLAVE_SR_RXACK))
				return check_ack > 0 ? 0 : -1;
			else {
				if (check_ack > 0) {
					i2c_send_stop();
					return -1;
				} else {
					return 0; /* NACK expected */
				}
			}
		}
		return 0;
	}
	return -1;
}

static int i2c_reset(void)
{
	i2c_write_reg(I2C_MASTER_SLAVE_CR, I2C_MASTER_SLAVE_CR_READ|I2C_MASTER_SLAVE_CR_IACK);
	if (i2c_transfer_complete(0))
		return -1;
	i2c_send_stop();
	if (i2c_wait_while_busy())
		return -1;
	return 0;
}


int i2c_send_start(uchar chip, int read)
{
	/* Send chip address. Only 7 bit addresses supported by u-boot */
	i2c_send_byte(chip << 1 | ( read != 0), 1, 0);
	if (i2c_transfer_complete(1))
		return -1;
	return 0;
}

static int i2c_address_chip(uchar chip, uint addr, int alen, int read)
{
	int shift;
	if (i2c_send_start(chip, 0))
		return -1;
	while (alen-- > 0) {
		shift = alen * 8;
		i2c_send_byte(addr >> shift, 0, 0);
		if (i2c_transfer_complete(1))
			return -1;
	}
	if (read && i2c_send_start(chip, 1))
		return -1;
	return 0;
}

static int i2c_read_byte(int stop)
{
	i2c_write_reg(I2C_MASTER_SLAVE_CR, I2C_MASTER_SLAVE_CR_READ | (stop ? I2C_MASTER_SLAVE_CR_STOP | I2C_MASTER_SLAVE_CR_ACK : 0));
	if (i2c_transfer_complete(0))
		return -1;
	return i2c_read_reg(I2C_MASTER_SLAVE_RXR);
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	if (i2c_wait_while_busy())
		return -1;
	if (i2c_address_chip(chip, addr, alen, 1))
		return -1;
	while (len-- > 0) {
		int byte = i2c_read_byte(len == 0);
		if (byte < 0)
			return -1;
		*buffer++ = byte;
	}
	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	if (i2c_wait_while_busy())
		return -1;
	if (i2c_address_chip(chip, addr, alen, 0))
		return -1;
	while (len-- > 0) {
		i2c_send_byte(*buffer++, 0, len == 0);
		if (i2c_transfer_complete(0))
			return -1;
	}
	return 0;
}

unsigned int i2c_get_bus_num(void)
{
	unsigned int i;
	for (i = 0; i <= 1; i++) {
		if (i2c_base == i2c_base_addrs[i])
			return i;
	}
	return -1;
}

int i2c_set_bus_num(unsigned int bus)
{
	if (bus > 1)
		return -1;
	i2c_base = i2c_base_addrs[bus];
	return 0;
}

int i2c_probe(uchar chip)
{
	u32 tmp;

	/*
	 * Try to read the first location of the chip.
	 */
	return i2c_read(chip, 0, 1, (uchar *)&tmp, 1);
}

void i2c_init(int speed, int slaveaddr)
{
	int prescale;

	if (!i2c_base)
		i2c_base = i2c_base_addrs[0];

	/* Disable controller and ack any pending IRQ */
	i2c_write_reg(I2C_MASTER_SLAVE_CTR, 0 | I2C_MASTER_SLAVE_CR_IACK);

	/* Set clock speed */
	prescale = 50000000 / (5 * speed ) -1;
	i2c_write_reg(I2C_MASTER_SLAVE_PRERlo, prescale & 0xff);
	i2c_write_reg(I2C_MASTER_SLAVE_PRERhi, (prescale >> 8) & 0xff);

	/* Enable controller */
	i2c_write_reg(I2C_MASTER_SLAVE_CTR, I2C_MASTER_SLAVE_CTR_CORE_ENABLE);

	i2c_reset();
}

