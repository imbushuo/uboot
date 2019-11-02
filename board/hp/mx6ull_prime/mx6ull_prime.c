/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <common.h>
#include <dm/uclass.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <linux/sizes.h>
#include <linux/fb.h>
#include <miiphy.h>
#include <mmc.h>
#include <mxsfb.h>
#include <netdev.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <asm/mach-imx/video.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL_WP (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define I2C_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE)

#define LCD_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP | PAD_CTL_PUE | \
	PAD_CTL_PKE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm)

#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

#define WEIM_NOR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
		PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
		PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define OTG_ID_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#ifdef CONFIG_SYS_I2C
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1 */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_UART4_TX_DATA__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_UART4_TX_DATA__GPIO1_IO28 | PC,
		.gp = IMX_GPIO_NR(1, 28),
	},
	.sda = {
		.i2c_mode = MX6_PAD_UART4_RX_DATA__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_UART4_RX_DATA__GPIO1_IO29 | PC,
		.gp = IMX_GPIO_NR(1, 29),
	},
};

/* I2C2 */
struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_UART5_TX_DATA__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_UART5_TX_DATA__GPIO1_IO_30 | PC,
		.gp = IMX_GPIO_NR(1, 30),
	},
	.sda = {
		.i2c_mode = MX6_PAD_UART5_RX_DATA__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_UART5_RX_DATA__GPIO1_IO_31 | PC,
		.gp = IMX_GPIO_NR(1, 31),
	},
};
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#ifdef CONFIG_CMD_NAND
static iomux_v3_cfg_t const nand_pads[] = {
	MX6_PAD_NAND_DATA00__RAWNAND_DATA00 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA01__RAWNAND_DATA01 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA02__RAWNAND_DATA02 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA03__RAWNAND_DATA03 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA04__RAWNAND_DATA04 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA05__RAWNAND_DATA05 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA06__RAWNAND_DATA06 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA07__RAWNAND_DATA07 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_CLE__RAWNAND_CLE | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_ALE__RAWNAND_ALE | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_CE0_B__RAWNAND_CE0_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_CE1_B__RAWNAND_CE1_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_CSI_MCLK__RAWNAND_CE2_B   | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_CSI_PIXCLK__RAWNAND_CE3_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_RE_B__RAWNAND_RE_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_WE_B__RAWNAND_WE_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_WP_B__RAWNAND_WP_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_READY_B__RAWNAND_READY_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DQS__RAWNAND_DQS | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	SETUP_IOMUX_PADS(nand_pads);

	setup_gpmi_io_clk((3 << MXC_CCM_CSCDR1_BCH_PODF_OFFSET) |
			  (3 << MXC_CCM_CSCDR1_GPMI_PODF_OFFSET));

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

#ifdef CONFIG_MXC_SPI
#error "SPI NOR is not supported on this board"
#endif


static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

#ifdef CONFIG_FSL_QSPI
#error "QSPI is not supported on this board"
#endif

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 4},
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(1, 19)
#define USDHC1_PWR_GPIO	IMX_GPIO_NR(1, 9)
#define USDHC1_VSELECT IMX_GPIO_NR(1, 5)
#define USDHC2_PWR_GPIO	IMX_GPIO_NR(4, 10)

int board_mmc_get_env_dev(int devno)
{
	if (devno == 1 && mx6_esdhc_fused(USDHC1_BASE_ADDR))
		devno = 0;

	return devno;
}

int mmc_map_to_kernel_blk(int devno)
{
	if (devno == 0 && mx6_esdhc_fused(USDHC1_BASE_ADDR))
		devno = 1;

	return devno;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
#ifdef CONFIG_MX6ULL_DDR3_ARM2_EMMC_REWORK
		ret = 1;
#else
		ret = !gpio_get_value(USDHC1_CD_GPIO);
#endif
		break;
#if !defined(CONFIG_CMD_NAND) && !defined(CONFIG_MX6ULL_DDR3_ARM2_QSPIB_REWORK)
	case USDHC2_BASE_ADDR:
		ret = 1;
		break;
#endif
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int i;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		/*case 0:
			SETUP_IOMUX_PADS(usdhc1_pads);
			gpio_request(USDHC1_CD_GPIO, "usdhc1 cd");
			gpio_direction_input(USDHC1_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			// 3.3V
			gpio_request(USDHC1_VSELECT, "usdhc1 vsel");
			gpio_request(USDHC1_PWR_GPIO, "usdhc1 pwr");
			gpio_direction_output(USDHC1_VSELECT, 0);
			gpio_direction_output(USDHC1_PWR_GPIO, 1);
			break;*/
		default:
			printf("Warning: you configured more USDHC controllers (%d) than supported by the board\n", i + 1);
			return 0;
			}

			if (fsl_esdhc_initialize(bis, &usdhc_cfg[i]))
				printf("Warning: failed to initialize mmc dev %d\n", i);
	}

	return 0;
}
#endif

#ifdef CONFIG_VIDEO_MXS
static iomux_v3_cfg_t const lcd_pads[] = {
	MX6_PAD_LCD_CLK__LCDIF_CLK | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_ENABLE__LCDIF_ENABLE | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_HSYNC__LCDIF_HSYNC | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_VSYNC__LCDIF_VSYNC | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA00__LCDIF_DATA00 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA01__LCDIF_DATA01 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA02__LCDIF_DATA02 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA03__LCDIF_DATA03 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA04__LCDIF_DATA04 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA05__LCDIF_DATA05 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA06__LCDIF_DATA06 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA07__LCDIF_DATA07 | MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_CSI_VSYNC__GPIO4_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_CSI_DATA00__GPIO4_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_CSI_DATA01__GPIO4_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_CSI_DATA02__GPIO4_IO23 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_LCD_RESET__GPIO3_IO04 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD1_DATA3__GPIO2_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

struct lcd_panel_info_t {
	unsigned int lcdif_base_addr;
	int depth;
	void (*enable)(struct lcd_panel_info_t const *dev);
	struct fb_videomode mode;
};

#define LCD_CS  IMX_GPIO_NR(4, 22)
#define LCD_SCK IMX_GPIO_NR(4, 21)
#define LCD_SDA IMX_GPIO_NR(4, 23)

void lcd_reset(void);
void lcd_exit_standby(void);

void lcd_send_byte(unsigned char b) {
	gpio_set_value(LCD_CS, 0);
	for (int i = 0; i < 8; i++) {
		gpio_set_value(LCD_SCK, 0);
		gpio_set_value(LCD_SDA, (b & 0x80) ? 1 : 0);
		b <<= 1;
		gpio_set_value(LCD_SCK, 1);
	}
	gpio_set_value(LCD_CS, 1);
}

void lcd_reset()
{
	gpio_request(IMX_GPIO_NR(3, 4), "lcd reset");

	/* Reset the screen */
	gpio_direction_output(IMX_GPIO_NR(3, 4) , 1);
	udelay(12000);
	gpio_direction_output(IMX_GPIO_NR(3, 4) , 0);
	udelay(12000);
	gpio_direction_output(IMX_GPIO_NR(3, 4) , 1);

	/* The controller needs 120ms to recover from reset */
	mdelay(120);
}

void lcd_exit_standby()
{
	/* ili9322_power_on */
	lcd_send_byte(0x05);
	lcd_send_byte(0xef);

	/* The controller needs 120ms to recover from reset */
	mdelay(120);

	lcd_send_byte(0x30);
	lcd_send_byte(0x0d);
}

void do_enable_parallel_lcd(struct display_info_t const *dev)
{
	int ret = 0, busno = 0, chip_addr = 0x08, data_addr = 0x4c;
	uint8_t write_byte = 0x1f;
	struct udevice *i2cbus;
	struct udevice *i2cdev;

	enable_lcdif_clock(dev->bus, 1);

	SETUP_IOMUX_PADS(lcd_pads);

	/* Set LDO1 voltage to 3.3V: LDO1_VOLT at 0x4c, value 0x1f */
	ret = uclass_get_device_by_seq(UCLASS_I2C, busno, &i2cbus);
	if (ret) {
		debug("%s: No I2C bus %d foune\n", __func__, busno);
		return;
	}

	ret = i2c_get_chip(i2cbus, chip_addr, 1, &i2cdev);
	if (ret) {
		debug("%s: Failed to get chip at %d\n", __func__, chip_addr);
		return;
	}

	ret = dm_i2c_write(i2cdev, data_addr, &write_byte, 1);
	if (ret) {
		debug("%s: Failed to write I2C \n", __func__);
		return;
	}

	/* Power up the LCD */
	gpio_request(IMX_GPIO_NR(4, 19), "lcd backlight");
	gpio_direction_output(IMX_GPIO_NR(4, 19) , 1);

	gpio_request(IMX_GPIO_NR(2, 21), "lcd backlight battery");
	gpio_direction_output(IMX_GPIO_NR(2, 21) , 1);

	gpio_request(LCD_CS, "lcd cs");
	gpio_direction_output(LCD_CS, 1);

	gpio_request(LCD_SCK, "lcd sck");
	gpio_direction_output(LCD_SCK, 1);

	gpio_request(LCD_SDA, "lcd sda");
	gpio_direction_output(LCD_SDA, 1);

	lcd_reset();

	/* ili9322_seq_vcom */
	lcd_send_byte(0x01);
	lcd_send_byte(0x14);
	lcd_send_byte(0x02);
	lcd_send_byte(0x3a);

	/* ili9322_seq_gamma */
	lcd_send_byte(0x10);
	lcd_send_byte(0xa7);
	lcd_send_byte(0x11);
	lcd_send_byte(0x55);
	lcd_send_byte(0x12);
	lcd_send_byte(0x71);
	lcd_send_byte(0x13);
	lcd_send_byte(0x71);
	lcd_send_byte(0x14);
	lcd_send_byte(0x73);
	lcd_send_byte(0x15);
	lcd_send_byte(0x55);
	lcd_send_byte(0x16);
	lcd_send_byte(0x18);
	lcd_send_byte(0x17);
	lcd_send_byte(0x62);

	/* ili9322_exit_standby */
	lcd_exit_standby();
}

struct display_info_t const displays[] = {{
	.bus = MX6ULL_LCDIF1_BASE_ADDR,
	.addr = 0,
	.pixfmt = 24,
	.detect = NULL,
 	.enable	= do_enable_parallel_lcd,
	.mode	= {
		.name		= "PRIMELCD",
		.xres           = 320,
		.yres           = 240,
		.pixclock       = 55555,
		.left_margin    = 71,
		.right_margin   = 100,
		.upper_margin   = 17,
		.lower_margin   = 6,
		.hsync_len      = 1,
		.vsync_len      = 1,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED,
		.serial_rgb		= 1
} } };
size_t display_count = ARRAY_SIZE(displays);
#endif

#ifdef CONFIG_MXC_EPDC
#error "EPDC is not supported on this board."
#endif

#ifdef CONFIG_FEC_MXC
#error "Ethernet is not supported on this board."
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
#endif

#ifdef CONFIG_CMD_NAND
	setup_gpmi_nand();
#endif

	return 0;
}
#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd1", MAKE_CFGVAL(0x42, 0x20, 0x00, 0x00)},
	// 64 pages in block
	{"nand", MAKE_CFGVAL(0x93, 0x00, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

u32 get_board_rev(void)
{
	return get_cpu_rev();
}

int checkboard(void)
{
	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
#ifndef CONFIG_DM_USB

#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)
iomux_v3_cfg_t const usb_otg1_pads[] = {
	MX6_PAD_GPIO1_IO04__USB_OTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO1_IO00__ANATOP_OTG1_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL),
};

/*
 * Leave it here, but default configuration only supports 1 port now,
 * because we need sd1 and i2c1
 */
iomux_v3_cfg_t const usb_otg2_pads[] = {
	/* conflict with i2c1_scl */
	MX6_PAD_GPIO1_IO02__USB_OTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* conflict with sd1_vselect */
	MX6_PAD_GPIO1_IO05__ANATOP_OTG2_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL),
};

int board_usb_phy_mode(int port)
{
	return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	switch (port) {
	case 0:
		SETUP_IOMUX_PADS(usb_otg1_pads);
		break;
	case 1:
		SETUP_IOMUX_PADS(usb_otg2_pads);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return 1;
	}

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	/* Set Power polarity */
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}
#endif
#endif
