/*****************************************************************************
 *
 * Filename:
 * ---------
 *    charging_pmic.c
 *
 * Project:
 * --------
 *   ALPS_Software
 *
 * Description:
 * ------------
 *   This file implements the interface between BMT and ADC scheduler.
 *
 * Author:
 * -------
 *  Oscar Liu
 *
 *============================================================================
  * $Revision:   1.0  $
 * $Modtime:   11 Aug 2005 10:28:16  $
 * $Log:   //mtkvs01/vmdata/Maui_sw/archives/mcu/hal/peripheral/inc/bmt_chr_setting.h-arc  $
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <mach/charging.h>
#include "bq24297.h"
#include <drivers/misc/mediatek/power/mt8135/upmu_common.h>
#include <mach/mt_gpio_def.h>
#include <mach/upmu_hw.h>
#include <linux/xlog.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/mt_sleep.h>
#include <mach/mt_boot.h>
#include <mach/system.h>
#include <mach/battery_custom_data.h>

#if 1
#include <mach/mt_clkmgr.h>
#include <mach/mt_pm_ldo.h>
#endif
 /* ============================================================ // */
 /* define */
 /* ============================================================ // */
#define STATUS_OK	0
#define STATUS_UNSUPPORTED	-1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))

static mt_battery_charging_custom_data *p_bat_charging_data;

kal_bool charging_type_det_done = KAL_TRUE;

const kal_uint32 VBAT_CV_VTH[] = {
	3504000, 3520000, 3536000, 3552000,
	3568000, 3584000, 3600000, 3616000,
	3632000, 3648000, 3664000, 3680000,
	3696000, 3712000, 3728000, 3744000,
	3760000, 3776000, 3792000, 3808000,
	3824000, 3840000, 3856000, 3872000,
	3888000, 3904000, 3920000, 3936000,
	3952000, 3968000, 3984000, 4000000,
	4016000, 4032000, 4048000, 4064000,
	4080000, 4096000, 4112000, 4128000,
	4144000, 4160000, 4176000, 4192000,
	4208000, 4224000, 4240000, 4256000
};

const kal_uint32 CS_VTH[] = {
	51200, 57600, 64000, 70400,
	76800, 83200, 89600, 96000,
	102400, 108800, 115200, 121600,
	128000, 134400, 140800, 147200,
	153600, 160000, 166400, 172800,
	179200, 185600, 192000, 198400,
	204800, 211200, 217600, 224000
};

const kal_uint32 INPUT_CS_VTH[] = {
	CHARGE_CURRENT_100_00_MA, CHARGE_CURRENT_150_00_MA, CHARGE_CURRENT_500_00_MA,
	    CHARGE_CURRENT_900_00_MA,
	CHARGE_CURRENT_1000_00_MA, CHARGE_CURRENT_1500_00_MA, CHARGE_CURRENT_1800_00_MA,
	    CHARGE_CURRENT_1800_00_MA
};

const kal_uint32 VCDT_HV_VTH[] = {
	BATTERY_VOLT_04_200000_V, BATTERY_VOLT_04_250000_V, BATTERY_VOLT_04_300000_V,
	    BATTERY_VOLT_04_350000_V,
	BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_450000_V, BATTERY_VOLT_04_500000_V,
	    BATTERY_VOLT_04_550000_V,
	BATTERY_VOLT_04_600000_V, BATTERY_VOLT_06_000000_V, BATTERY_VOLT_06_500000_V,
	    BATTERY_VOLT_07_000000_V,
	BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V, BATTERY_VOLT_09_500000_V,
	    BATTERY_VOLT_10_500000_V
};

 /* ============================================================ // */
 /* function prototype */
 /* ============================================================ // */


 /* ============================================================ // */
 /* extern variable */
 /* ============================================================ // */

 /* ============================================================ // */
 /* extern function */
 /* ============================================================ // */
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern bool mt_usb_is_device(void);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);

 /* ============================================================ // */
kal_uint32 charging_value_to_parameter(const kal_uint32 *parameter, const kal_uint32 array_size,
				       const kal_uint32 val)
{
	if (val < array_size) {
		return parameter[val];
	} else {
		battery_xlog_printk(BAT_LOG_CRTI, "Can't find the parameter \r\n");
		return parameter[0];
	}
}


kal_uint32 charging_parameter_to_value(const kal_uint32 *parameter, const kal_uint32 array_size,
				       const kal_uint32 val)
{
	kal_uint32 i;

	for (i = 0; i < array_size; i++) {
		if (val == *(parameter + i)) {
			return i;
		}
	}

	battery_xlog_printk(BAT_LOG_CRTI, "NO register value match. val=%d\r\n", val);
	/* TODO: ASSERT(0);      // not find the value */
	return 0;
}


static kal_uint32 bmt_find_closest_level(const kal_uint32 *pList, kal_uint32 number,
					 kal_uint32 level)
{
	kal_uint32 i;
	kal_uint32 max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = KAL_TRUE;
	else
		max_value_in_last_element = KAL_FALSE;

	if (max_value_in_last_element == KAL_TRUE) {
		for (i = (number - 1); i != 0; i--)	/* max value in the last element */
		{
			if (pList[i] <= level) {
				return pList[i];
			}
		}

		battery_xlog_printk(BAT_LOG_CRTI,
				    "Can't find closest level, small value first \r\n");
		return pList[0];
		/* return CHARGE_CURRENT_0_00_MA; */
	} else {
		for (i = 0; i < number; i++)	/* max value in the first element */
		{
			if (pList[i] <= level) {
				return pList[i];
			}
		}

		battery_xlog_printk(BAT_LOG_CRTI,
				    "Can't find closest level, large value first \r\n");
		return pList[number - 1];
		/* return CHARGE_CURRENT_0_00_MA; */
	}
}

static kal_uint32 charging_hw_init(void *data)
{
	kal_uint32 status = STATUS_OK;
	p_bat_charging_data = (mt_battery_charging_custom_data *)data;

	upmu_set_rg_bc11_bb_ctrl(1);	/* BC11_BB_CTRL */
	upmu_set_rg_bc11_rst(1);	/* BC11_RST */

#if 0				/* no use */
	/* pull PSEL low */
	mt_pin_set_mode_gpio(GPIO_CHR_PSEL_PIN);
	gpio_direction_output(GPIO_CHR_PSEL_PIN, 0);
#endif

	if (p_bat_charging_data->charger_enable_pin > 0) {
		/* pull CE low */
		mt_pin_set_mode_gpio(p_bat_charging_data->charger_enable_pin);
		gpio_direction_output(p_bat_charging_data->charger_enable_pin, 0);
	}

	/* battery_xlog_printk(BAT_LOG_FULL, "gpio_number=0x%x,gpio_on_mode=%d,gpio_off_mode=%d\n", gpio_number, gpio_on_mode, gpio_off_mode); */

	bq24297_set_en_hiz(0x0);
	/* bq24297_set_vindpm(0xA); //VIN DPM check 4.68V */
	bq24297_set_vindpm(0x7);	/* VIN DPM check 4.44V */
	bq24297_set_reg_rst(0x0);
	bq24297_set_wdt_rst(0x1);	/* Kick watchdog */
	bq24297_set_sys_min(0x5);	/* Minimum system voltage 3.5V */
#if defined(CONFIG_MTK_JEITA_STANDARD_SUPPORT)
	bq24297_set_iprechg(0x1);	/* Precharge current 256mA */
#else
	bq24297_set_iprechg(0x3);	/* Precharge current 512mA */
#endif
	bq24297_set_iterm(0x0);	/* Termination current 128mA */

#if !defined(CONFIG_MTK_JEITA_STANDARD_SUPPORT)
	bq24297_set_vreg(0x2C);	/* VREG 4.208V */
#endif
	bq24297_set_batlowv(0x1);	/* BATLOWV 3.0V */
	bq24297_set_vrechg(0x0);	/* VRECHG 0.1V (4.108V) */
	bq24297_set_en_term(0x1);	/* Enable termination */
	bq24297_set_term_stat(0x0);	/* Match ITERM */
	bq24297_set_watchdog(0x1);	/* WDT 40s */
#if !defined(CONFIG_MTK_JEITA_STANDARD_SUPPORT)
	bq24297_set_en_timer(0x0);	/* Disable charge timer */
#endif
	bq24297_set_int_mask(0x0);	/* Disable fault interrupt */

	return status;
}


static kal_uint32 charging_dump_register(void *data)
{
	kal_uint32 status = STATUS_OK;

	battery_xlog_printk(BAT_LOG_CRTI, "charging_dump_register\r\n");

	bq24297_dump_register();

	return status;
}


static kal_uint32 charging_enable(void *data)
{
	kal_uint32 status = STATUS_OK;
	kal_uint32 enable = *(kal_uint32 *) (data);

	if (KAL_TRUE == enable) {
		bq24297_set_en_hiz(0x0);
		bq24297_set_chg_config(0x1);	/* charger enable */
	} else {
#if defined(CONFIG_USB_MTK_HDRC_HCD)
		if (mt_usb_is_device())
#endif
		{
			bq24297_set_chg_config(0x0);

		}
	}

	return status;
}


static kal_uint32 charging_set_cv_voltage(void *data)
{
	kal_uint32 status = STATUS_OK;
	kal_uint16 register_value;
	kal_uint32 cv_value = *(kal_uint32 *) (data);

	if (cv_value == BATTERY_VOLT_04_200000_V) {
		/* use nearest value */
		cv_value = 4208000;
	}
	register_value =
	    charging_parameter_to_value(VBAT_CV_VTH, GETARRAYNUM(VBAT_CV_VTH), cv_value);
	bq24297_set_vreg(register_value);

	return status;
}


static kal_uint32 charging_get_current(void *data)
{
	kal_uint32 status = STATUS_OK;
	/* kal_uint32 array_size; */
	/* kal_uint8 reg_value; */

	kal_uint8 ret_val = 0;
	kal_uint8 ret_force_20pct = 0;

	/* Get current level */
	bq24297_read_interface(bq24297_CON2, &ret_val, CON2_ICHG_MASK, CON2_ICHG_SHIFT);

	/* Get Force 20% option */
	bq24297_read_interface(bq24297_CON2, &ret_force_20pct, CON2_FORCE_20PCT_MASK,
			       CON2_FORCE_20PCT_SHIFT);

	/* Parsing */
	ret_val = (ret_val * 64) + 512;

	if (ret_force_20pct == 0) {
		/* Get current level */
		/* array_size = GETARRAYNUM(CS_VTH); */
		/* *(kal_uint32 *)data = charging_value_to_parameter(CS_VTH,array_size,reg_value); */
		*(kal_uint32 *) data = ret_val;
	} else {
		/* Get current level */
		/* array_size = GETARRAYNUM(CS_VTH_20PCT); */
		/* *(kal_uint32 *)data = charging_value_to_parameter(CS_VTH,array_size,reg_value); */
		/* return (int)(ret_val<<1)/10; */
		*(kal_uint32 *) data = (int)(ret_val << 1) / 10;
	}

	return status;
}



static kal_uint32 charging_set_current(void *data)
{
	kal_uint32 status = STATUS_OK;

#if 1
	kal_uint32 set_chr_current;
	kal_uint32 array_size;
	kal_uint32 register_value;
	kal_uint32 current_value = *(kal_uint32 *) data;

	if (current_value == 25600) {
		bq24297_set_force_20pct(0x1);
		bq24297_set_ichg(0xC);
		return status;
	}
	bq24297_set_force_20pct(0x0);

	array_size = GETARRAYNUM(CS_VTH);
	set_chr_current = bmt_find_closest_level(CS_VTH, array_size, current_value);
	register_value = charging_parameter_to_value(CS_VTH, array_size, set_chr_current);
	bq24297_set_ichg(register_value);
#else
	battery_xlog_printk(BAT_LOG_FULL, "skip charging_set_current()\r\n");
#endif

	return status;
}


static kal_uint32 charging_set_input_current(void *data)
{
	kal_uint32 status = STATUS_OK;

#if 1
	kal_uint32 set_chr_current;
	kal_uint32 array_size;
	kal_uint32 register_value;

	array_size = GETARRAYNUM(INPUT_CS_VTH);
	set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size, *(kal_uint32 *) data);
	register_value = charging_parameter_to_value(INPUT_CS_VTH, array_size, set_chr_current);

	bq24297_set_iinlim(register_value);
#else
	battery_xlog_printk(BAT_LOG_CRTI, "skip charging_set_input_current()\r\n");
#endif

	return status;
}


static kal_uint32 charging_get_charging_status(void *data)
{
	kal_uint32 status = STATUS_OK;
	kal_uint32 ret_val;

	ret_val = bq24297_get_chrg_stat();

	if (ret_val == 0x3)
		*(kal_uint32 *) data = KAL_TRUE;
	else
		*(kal_uint32 *) data = KAL_FALSE;

	return status;
}


static kal_uint32 charging_reset_watch_dog_timer(void *data)
{
	kal_uint32 status = STATUS_OK;

	battery_xlog_printk(BAT_LOG_FULL, "charging_reset_watch_dog_timer\r\n");

	bq24297_set_wdt_rst(0x1);	/* Kick watchdog */

	return status;
}


static kal_uint32 charging_set_hv_threshold(void *data)
{
	kal_uint32 status = STATUS_OK;

	kal_uint32 set_hv_voltage;
	kal_uint32 array_size;
	kal_uint16 register_value;
	kal_uint32 voltage = *(kal_uint32 *) (data);

	array_size = GETARRAYNUM(VCDT_HV_VTH);
	set_hv_voltage = bmt_find_closest_level(VCDT_HV_VTH, array_size, voltage);
	register_value = charging_parameter_to_value(VCDT_HV_VTH, array_size, set_hv_voltage);
	upmu_set_rg_vcdt_hv_vth(register_value);

	return status;
}


static kal_uint32 charging_get_hv_status(void *data)
{
	kal_uint32 status = STATUS_OK;

	*(kal_bool *) (data) = upmu_get_rgs_vcdt_hv_det();

	return status;
}


static kal_uint32 charging_get_battery_status(void *data)
{
	kal_uint32 status = STATUS_OK;

	/* upmu_set_baton_tdet_en(1); */
	upmu_set_rg_baton_en(1);
	*(kal_bool *) (data) = upmu_get_rgs_baton_undet();

	return status;
}


static kal_uint32 charging_get_charger_det_status(void *data)
{
	kal_uint32 status = STATUS_OK;

	*(kal_bool *) (data) = upmu_get_rgs_chrdet();

	return status;
}


kal_bool charging_type_detection_done(void)
{
	return charging_type_det_done;
}

extern CHARGER_TYPE hw_charger_type_detection(void);

static kal_uint32 charging_get_charger_type(void *data)
{
	kal_uint32 status = STATUS_OK;
	CHARGER_TYPE charger_type = CHARGER_UNKNOWN;
	kal_uint32 ret_val;
	kal_uint8 reg_val = 0;
	kal_uint32 count = 0;
	unsigned int USB_U2PHYACR6_2 = 0xF122081A;

#if defined(CONFIG_POWER_EXT)
	*(CHARGER_TYPE *) (data) = STANDARD_HOST;
#else

	charging_type_det_done = KAL_FALSE;

#if 0
	charger_type = hw_charger_type_detection();
#else
	battery_xlog_printk(BAT_LOG_CRTI, "use BQ24297 charger detection\r\n");

	enable_pll(UNIVPLL, "USB_PLL");
	hwPowerOn(MT65XX_POWER_LDO_VUSB, VOL_DEFAULT, "VUSB_LDO");
	SETREG16(USB_U2PHYACR6_2, 0x80);	/* bit 7 = 1 : switch to PMIC */

	ret_val = bq24297_get_vbus_stat();

	if (ret_val == 0x0) {
		bq24297_set_dpdm_en(1);
		while (bq24297_get_dpdm_status() == 1) {
			count++;
			mdelay(1);
			battery_xlog_printk(BAT_LOG_CRTI, "polling BQ24297 charger detection\r\n");
			if (count > 1000)
				break;
		}
	}

	ret_val = bq24297_get_vbus_stat();
	switch (ret_val) {
	case 0x1:
		charger_type = STANDARD_HOST;
		/* charger_type = hw_charger_type_detection(); */
		break;
	case 0x2:
		charger_type = STANDARD_CHARGER;
		break;
	default:
		charger_type = NONSTANDARD_CHARGER;	/* CHARGER_UNKNOWN; */
		break;
	}

	if (charger_type == STANDARD_CHARGER) {
		bq24297_read_interface(bq24297_CON0, &reg_val, CON0_IINLIM_MASK, CON0_IINLIM_SHIFT);
		if (reg_val < 0x4) {
			battery_xlog_printk(BAT_LOG_CRTI,
					    "Set to Non-standard charger due to 1A input limit.\r\n");
			charger_type = NONSTANDARD_CHARGER;
		} else if (reg_val == 0x4) { /* APPLE_1_0A_CHARGER - 1A apple charger */
			battery_xlog_printk(BAT_LOG_CRTI, "Set to APPLE_1_0A_CHARGER.\r\n");
			charger_type = APPLE_1_0A_CHARGER;
		} else if (reg_val == 0x6) { /* APPLE_2_1A_CHARGER,  2.1A apple charger */
			battery_xlog_printk(BAT_LOG_CRTI, "Set to APPLE_2_1A_CHARGER.\r\n");
			charger_type = APPLE_2_1A_CHARGER;
		}
	}

	CLRREG16(USB_U2PHYACR6_2, 0x80);	/* bit 7 = 0 : switch to USB */
	hwPowerDown(MT65XX_POWER_LDO_VUSB, "VUSB_LDO");
	disable_pll(UNIVPLL, "USB_PLL");
#endif
	battery_xlog_printk(BAT_LOG_CRTI, "charging_get_charger_type = %d\r\n", charger_type);

	*(CHARGER_TYPE *) (data) = charger_type;

	charging_type_det_done = KAL_TRUE;
#endif
	return status;
}

static kal_uint32 charging_get_is_pcm_timer_trigger(void *data)
{
	kal_uint32 status = STATUS_OK;

	if (slp_get_wake_reason() == WR_PCM_TIMER)
		*(kal_bool *) (data) = KAL_TRUE;
	else
		*(kal_bool *) (data) = KAL_FALSE;

	battery_xlog_printk(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());

	return status;
}

static kal_uint32 charging_set_platform_reset(void *data)
{
	kal_uint32 status = STATUS_OK;

	battery_xlog_printk(BAT_LOG_CRTI, "charging_set_platform_reset\n");

	arch_reset(0, NULL);

	return status;
}

static kal_uint32 charging_get_platfrom_boot_mode(void *data)
{
	kal_uint32 status = STATUS_OK;

	*(kal_uint32 *) (data) = get_boot_mode();

	battery_xlog_printk(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());

	return status;
}

static kal_uint32 charging_enable_powerpath(void *data)
{
	kal_uint32 status = STATUS_OK;
	kal_uint32 enable = *(kal_uint32 *) (data);

	if (KAL_TRUE == enable) {
		bq24297_set_en_hiz(0x0);
	} else {
		bq24297_set_en_hiz(0x1);
	}

	return status;
}

static kal_uint32(*const charging_func[CHARGING_CMD_NUMBER]) (void *data) = {
charging_hw_init, charging_dump_register, charging_enable, charging_set_cv_voltage,
	    charging_get_current, charging_set_current, charging_set_input_current,
	    charging_get_charging_status, charging_reset_watch_dog_timer,
	    charging_set_hv_threshold, charging_get_hv_status, charging_get_battery_status,
	    charging_get_charger_det_status, charging_get_charger_type,
	    charging_get_is_pcm_timer_trigger, charging_set_platform_reset,
	    charging_get_platfrom_boot_mode, charging_enable_powerpath};


 /*
  * FUNCTION
  *             Internal_chr_control_handler
  *
  * DESCRIPTION
  *              This function is called to set the charger hw
  *
  * CALLS
  *
  * PARAMETERS
  *             None
  *
  * RETURNS
  *
  *
  * GLOBALS AFFECTED
  *        None
  */
kal_int32 bq24297_control_interface(CHARGING_CTRL_CMD cmd, void *data)
{
	kal_int32 status;
	if (cmd < CHARGING_CMD_NUMBER)
		status = charging_func[cmd] (data);
	else
		return STATUS_UNSUPPORTED;

	return status;
}
