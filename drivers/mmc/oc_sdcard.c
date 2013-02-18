/*
 * Driver for Opencores SD card mass storage controller by Adam Edvardsson
 * written by Yann Vernier for ORSoC AB
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <part.h>

#include <asm/errno.h>

#ifndef CONFIG_OC_SD_BASE
#define CONFIG_OC_SD_BASE 0x9e000000
#endif

struct oc_sd {
  /* Only 32-bit accesses are valid. No endianness compensation on slave wb. */
  /* However, many registers have only 8 or 16 actual bits of storage. */
  /* 0x00 */
  volatile u32 argument, command, status, response;
  /* 0x10 */
  volatile u32 reserved3[3], control;
  /* 0x20 */
  volatile u32 blocksize, power, soft_reset, timeout;
  /* 0x30 */
  volatile u32 normal_int_status, error_int_status,
    normal_int_enable, error_int_enable;
  /* 0x40 */
  volatile u32 reserved12[2], capability, clock_divider;
  /* 0x50 */
  volatile u32 bd_status, bd_int_status, bd_int_enable, reserved17;
  /* 0x60 */
  volatile u32 bd_rx, reserved18[7];
  /* 0x80 */
  volatile u32 bd_tx;
};

static volatile struct oc_sd * const regs=(void*)CONFIG_OC_SD_BASE;

static void ocsd_set_ios(struct mmc *mmc)
{
  printf("ocsd_set_ios: Adjusting clock to %d\n", mmc->clock);
  regs->clock_divider = mmc->clock ? mmc->f_max/mmc->clock-1 : 0;
  if (mmc->bus_width == 4)
    regs->control |= 2;
  else
    regs->control &= ~2;
}

static int ocsd_send_cmd(struct mmc *mmc, 
			 struct mmc_cmd *cmd, struct mmc_data *data)
{
  u32 temp;

  /* command setup */
  printf("SD command: idx %d, flags %#x\n",
	 cmd->cmdidx, cmd->resp_type);
  do {
    temp = regs->status;
  } while (temp & 1);  /* busy */
  temp = (cmd->cmdidx<<8);
  if (cmd->resp_type & MMC_RSP_PRESENT) {
    if (cmd->resp_type & MMC_RSP_136)
      temp |= 0x01;
    else
      temp |= 0x02;
  }
  if (cmd->resp_type & MMC_RSP_CRC)
    temp |= 1<<3;    /* perform CRC check on response */
  if (cmd->resp_type & MMC_RSP_OPCODE)
    temp |= 1<<4;   /* perform response index check */
  printf("Setting cmd=%x arg=%x\n", temp, cmd->cmdarg);

  if (data) {
    /* data transfer */
    regs->command = 0xc000 | temp;
    regs->blocksize=data->blocksize;
    if (data->blocks!=1) {
      printf("Attempted multiblock access, unsupported\n");
      return UNUSABLE_ERR;
    }
    printf("Data transfer command %d\n", cmd->cmdidx);
    //      return COMM_ERR;    /* failed */
    if (data->flags & MMC_DATA_WRITE) {
      regs->bd_tx = (u32)data->src;
      regs->bd_tx = cmd->cmdarg;
    } else if (data->flags & MMC_DATA_READ) {
      regs->bd_rx = (u32)data->dest;
      regs->bd_rx = cmd->cmdarg;
    } else {
      printf("Data transfer neither write nor read?\n");
      return UNUSABLE_ERR;
    }
    do {
      temp = regs->bd_int_status;
    } while (!temp);
    regs->bd_int_status = 0;   /* reset the interrupt status register */
    if (temp==1)
      return 0;    /* did transfer the block */
    else
      return COMM_ERR;    /* failed */
  } else {
    regs->command = temp;
    regs->argument = cmd->cmdarg;
    //printf("Set cmd=%x arg=%x\n", regs->command, regs->argument);
  }
  
  if (cmd->resp_type == MMC_RSP_NONE)
    return 0;

  while (1) {
    /* Await command completion */
    temp = regs->normal_int_status;
    if (temp & 1) {
      printf("Response received, status %x\n", temp);
      break;
    }
    temp = regs->error_int_status;
    if (temp)
      printf("Error status %x\n", temp);
    if (temp & ((1<<3)|(1<<1)))   /* command index or crc wrong */
      return COMM_ERR;
    else if (temp & 1)  /* timeout */
      return TIMEOUT;
  }
  /* Unfortunately, OC SD controller only saves one word of response */
  if (cmd->resp_type & MMC_RSP_PRESENT) {
    cmd->response[0] = regs->response;
    printf("Response 0x%08x\n", cmd->response[0]);
  } else
    printf("No response requested.\n");
  return 0;
}

static int ocsd_init(struct mmc *mmc)
{
  regs->soft_reset=1;
  regs->normal_int_enable = 0;
  regs->bd_int_enable = 0;
  regs->timeout=0xffff;    /* longest possible timeout - ~1.3ms */
  printf("Initializing OC SD to clock %d\n", mmc->clock);
  regs->clock_divider = mmc->clock ? mmc->f_max/mmc->clock-1 : 1;
  printf("Divider %d\n", regs->clock_divider);
  regs->soft_reset=0;
  return 0;
}

int oc_sd_init(bd_t *bis)
{
  struct mmc *mmc=NULL;
  /*int temp, read_bds, write_bds;*/

  mmc = malloc(sizeof(struct mmc));
  if (!mmc)
    return -ENOMEM;
  strcpy(mmc->name, "Opencores SD");
  mmc->send_cmd=ocsd_send_cmd;
  mmc->set_ios=ocsd_set_ios;
  mmc->init=ocsd_init;
  mmc->host_caps=MMC_MODE_4BIT;

  mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
  mmc->f_max = CONFIG_SYS_CLK_FREQ/2;
  mmc->f_min = mmc->f_max/256*2;
  printf("OC SD frequencies: min %d, max %d\n", mmc->f_min, mmc->f_max);
  mmc->block_dev.part_type = PART_TYPE_DOS;

  /* Reset hardware and figure out how much we can write at once */
  /*
  regs->soft_reset=1;
  regs->soft_reset=0;
  temp = regs->bd_status;
  read_bds = (temp>>8)&0xff;
  write_bds = temp&0xff;
  */
  mmc->b_max = 1;  /* we accept only single block commands, due to command generation */

  mmc_register(mmc);

  return 0;
}
