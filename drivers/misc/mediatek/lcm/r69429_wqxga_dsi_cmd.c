#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/string.h>
#include <linux/regulator/consumer.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#else
#include <string.h>
#endif

#include "lcm_drv.h"
#include "r69429_wqxga_dsi_cmd.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */

#define FRAME_WIDTH  (2560)
#define FRAME_HEIGHT (1600)

/* GPIO42       DSI_TE*/
#ifdef GPIO_LCM_PWR_EN
#define GPIO_LCD_PWR_EN      GPIO_LCM_PWR_EN
#else
#define GPIO_LCD_PWR_EN      0xFFFFFFFF	/* GPIO42 */
#endif

/* GPIO131       vdd33_lcd */
#ifdef GPIO_LCM_PWR2_EN
#define GPIO_LCD_PWR2_EN      GPIO_LCM_PWR2_EN
#else
#define GPIO_LCD_PWR2_EN      GPIO131
#endif

/* GPIO127       LCM_RST for panel */
#ifdef GPIO_LCM_RST
#define GPIO_LCD_RST_EN		GPIO_LCM_RST
#else
#define GPIO_LCD_RST_EN		GPIO127
#endif

/* GPIO87       DISP_PWM0 for backlight panel */
#ifdef GPIO_LCM_BL_EN
#define GPIO_LCD_BL_EN		GPIO_LCM_BL_EN
#else
#define GPIO_LCD_BL_EN		GPIO87
#endif

/* GPIOEXT24       led_en for panel */
#ifdef GPIO_LCM_LED_EN
#define GPIO_LCD_LED_EN		GPIO_LCM_LED_EN
#else
#define GPIO_LCD_LED_EN		GPIOEXT24
#endif

/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

static LCM_UTIL_FUNCS lcm_util = {
	.set_reset_pin = NULL,
	.udelay = NULL,
	.mdelay = NULL,
};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)	 lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									 lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				 lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)						  lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE									1

/* #define	PUSH_TABLET_USING */
#define REGFLAG_DELAY									0xFFFC
#define REGFLAG_END_OF_TABLE								0xFFFD

#ifdef PUSH_TABLET_USING
struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x01, 0, {} },
	{REGFLAG_DELAY, 10, {} },
	{0x3a, 1, {0x77} },
	{0x2a, 4, {0x00, 0x00, 0x04, 0xff} },
	{0x2b, 4, {0x00, 0x00, 0x06, 0x3f} },
	{0x35, 1, {0x00} },
	{0x44, 2, {0x00, 0x00} },
	{0x51, 1, {0xff} },
	{0x53, 1, {0x24} },
	{0x51, 1, {0xff} },
	{0x53, 1, {0x24} },
	{0x55, 1, {0x00} },
	{0x11, 0, {} },
	{REGFLAG_DELAY, 120, {} },
	{0xB0, 1, {0x04} },
#if 0				/* (LCM_DSI_CMD_MODE) */
	{0x2C, 0, {} },
#else
	{0xB3, 5, {0x14, 0x08, 0x00, 0x22, 0x00} },
	{0xB3, 1, {0x14} },
#endif
	{0x29, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {
		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}
#endif

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

#ifdef BUILD_LK
	printf("%s, LK\n", __func__);
#else
	pr_debug("%s, KE\n", __func__);
#endif

#ifdef PUSH_TABLET_USING
	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#else

	data_array[0] = 0x00010500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(5);

	data_array[0] = 0x773a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00053902;
	data_array[1] = 0x0400002a;
	data_array[2] = 0xff;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = 0x0600002b;
	data_array[2] = 0x3f;
	dsi_set_cmdq(data_array, 3, 1);

#if 0				/* (LCM_DSI_CMD_MODE) */
	data_array[0] = 0x002C3909;	/* send image  2C-->3C */
	dsi_set_cmdq(data_array, 1, 0);
#endif

	data_array[0] = 0x00351500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00033902;
	data_array[1] = 0x00000044;
	dsi_set_cmdq(data_array, 2, 1);
/*
	data_array[0] = 0xff511500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x24531500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xff511500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x24531500;
	dsi_set_cmdq(data_array, 1, 1);
*/
	data_array[0] = 0x00551500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

#if (LCM_DSI_CMD_MODE)
#else
	data_array[0] = 0x04b02300;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00062902;
	data_array[1] = 0x000814b3;
	data_array[2] = 0x00000022;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x14b32300;
	dsi_set_cmdq(data_array, 1, 1);
#endif

	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
#endif
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

#ifndef BUILD_LK
static void lcm_request_gpio_control(void)
{
	gpio_request(GPIO_LCD_PWR2_EN, "GPIO_LCD_PWR2_EN");
	pr_debug("[KE/LCM] GPIO_LCD_PWR2_EN =  0x%x\n", GPIO_LCD_PWR2_EN);

	gpio_request(GPIO_LCD_RST_EN, "GPIO_LCD_RST_EN");
	pr_debug("[KE/LCM] GPIO_LCD_RST_EN =  0x%x\n", GPIO_LCD_RST_EN);

	gpio_request(GPIO_LCD_LED_EN, "GPIO_LCD_LED_EN");
	pr_debug("[KE/LCM] GPIO_LCD_LED_EN =   0x%x\n", GPIO_LCD_LED_EN);
}
#endif

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
	if (GPIO == 0xFFFFFFFF) {
#ifdef BUILD_LK
		printf("[LK/LCM] GPIO_LCD_PWR_EN =   0x%x\n", GPIO_LCD_PWR_EN);
		printf("[LK/LCM] GPIO_LCD_PWR2_EN =  0x%x\n", GPIO_LCD_PWR2_EN);
		printf("[LK/LCM] GPIO_LCD_RST_EN =  0x%x\n", GPIO_LCD_RST_EN);
		printf("[LK/LCM] GPIO_LCD_BL_EN =   0x%x\n", GPIO_LCD_BL_EN);
		printf("[LK/LCM] GPIO_LCM_LED_EN =  0x%x\n", GPIO_LCM_LED_EN);
#elif (defined BUILD_UBOOT)
#else
#endif

		return;
	}
#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
#else
	gpio_set_value(GPIO, (output > 0) ? GPIO_OUT_ONE : GPIO_OUT_ZERO);
#endif
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->lcm_if = LCM_INTERFACE_DSI_DUAL;
	params->lcm_cmd_if = LCM_INTERFACE_DSI_DUAL;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode = BURST_VDO_MODE;
#endif

	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size = 256;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 4;
	params->dsi.vertical_frontporch = 12;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 20;
	params->dsi.horizontal_backporch = 32;
	params->dsi.horizontal_frontporch = 92;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 450;
	params->dsi.ssc_disable = 1;
	params->dsi.cont_clock = 0;

	params->dsi.ufoe_enable = 1;
	params->dsi.ufoe_params.lr_mode_en = 1;
}

static void init_power(void)
{
#ifdef BUILD_LK
	printf("%s, LK\n", __func__);

	lcm_set_gpio_output(GPIO_LCD_BL_EN, GPIO_OUT_ZERO);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ZERO);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_LED_EN, GPIO_OUT_ZERO);
	MDELAY(10);

	/* 1.initial start */
	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_LED_EN, GPIO_OUT_ONE);
	MDELAY(50);

	lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_BL_EN, GPIO_OUT_ONE);
#else
#endif
}

static void lcm_init(struct platform_device *pdev)
{
#ifdef BUILD_LK
	printf("%s, LK\n", __func__);

	init_lcm_registers();
#else
	lcm_request_gpio_control();
#endif
}

static void lcm_suspend_power(void)
{
#ifdef BUILD_LK
	printf("%s, LK\n", __func__);
#else
	pr_debug("%s, kernel\n", __func__);
#endif

	lcm_set_gpio_output(GPIO_LCD_LED_EN, GPIO_OUT_ZERO);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ZERO);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];

#ifdef BUILD_LK
	printf("%s, LK\n", __func__);
#else
	pr_debug("%s, kernel\n", __func__);
#endif

#ifdef PUSH_TABLET_USING
	push_table(lcm_suspend_setting,
		   sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
#else
	data_array[0] = 0x00280500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0x00100500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(100);
#endif
}

static void lcm_resume_power(void)
{
#ifdef BUILD_LK
	printf("%s, LK\n", __func__);
#else
	pr_debug("%s, kernel\n", __func__);
#endif
	/* 1.initial start */
	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ONE);
	MDELAY(5);

	lcm_set_gpio_output(GPIO_LCD_LED_EN, GPIO_OUT_ONE);
	MDELAY(15);

	lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
}

static void lcm_resume(void)
{
#if (LCM_DSI_CMD_MODE)
	unsigned int data_array[16];
#endif

#ifdef BUILD_LK
	printf("%s, LK\n", __func__);
#else
	pr_debug("%s, kernel\n", __func__);
#endif

	init_lcm_registers();

#if (LCM_DSI_CMD_MODE)
	data_array[0] = 0x00110500;	/* Sleep Out */
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(150);

	data_array[0] = 0x00290500;	/* Display On */
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(40);
#endif
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + (width / 2) - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);

	data_array[3] = 0x00053902;
	data_array[4] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[5] = (y1_LSB);

	data_array[6] = 0x002c3909;
	dsi_set_cmdq(data_array, 7, 0);
}
#endif

LCM_DRIVER r69429_wqxga_dsi_cmd_lcm_drv = {
	.name = "r69429_wqxga_dsi_cmd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init_power = init_power,
	.init = lcm_init,
	.suspend_power = lcm_suspend_power,
	.suspend = lcm_suspend,
	.resume_power = lcm_resume_power,
	.resume = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
#endif
};
