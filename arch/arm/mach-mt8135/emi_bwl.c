#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/xlog.h>
#include <mach/mt_emi.h>

#include "mach/mt_reg_base.h"
#include "mach/emi_bwl.h"
#include "mach/sync_write.h"

DEFINE_SEMAPHORE(emi_bwl_sem);

static struct platform_driver mem_bw_ctrl = {
	.driver = {
		   .name = "mem_bw_ctrl",
		   .owner = THIS_MODULE,
		   },
};

static struct platform_driver ddr_type = {
	.driver = {
		   .name = "ddr_type",
		   .owner = THIS_MODULE,
		   },
};

static struct platform_driver dramc_high = {
	.driver = {
		   .name = "dramc_high",
		   .owner = THIS_MODULE,
		   },
};

#if 0
static struct platform_driver mem_bw_finetune_md = {
	.driver = {
		   .name = "mem_bw_finetune_md",
		   .owner = THIS_MODULE,
		   },
};

static struct platform_driver mem_bw_finetune_mm = {
	.driver = {
		   .name = "mem_bw_finetune_mm",
		   .owner = THIS_MODULE,
		   },
};
#endif

/* define EMI bandwiwth limiter control table */
static struct emi_bwl_ctrl ctrl_tbl[NR_CON_SCE];

/* current concurrency scenario */
static int cur_con_sce = 0x0FFFFFFF;

/* define concurrency scenario strings */
static const char *con_sce_str[] = {
#define X_CON_SCE(con_sce, arba, arbb, arbe, arbf, arbg) #con_sce,
#include "mach/con_sce_ddr3_32.h"
#undef X_CON_SCE
};

/* define EMI bandwidth allocation tables */
/****************** For DDR3-32bit ******************/

static const unsigned int emi_arba_ddr3_32_val[] = {
#define X_CON_SCE(con_sce, arba, arbb, arbe, arbf, arbg) arba,
#include "mach/con_sce_ddr3_32.h"
#undef X_CON_SCE
};

static const unsigned int emi_arbb_ddr3_32_val[] = {
#define X_CON_SCE(con_sce, arba, arbb, arbe, arbf, arbg) arbb,
#include "mach/con_sce_ddr3_32.h"
#undef X_CON_SCE
};

static const unsigned int emi_arbe_ddr3_32_val[] = {
#define X_CON_SCE(con_sce, arba, arbb, arbe, arbf, arbg) arbe,
#include "mach/con_sce_ddr3_32.h"
#undef X_CON_SCE
};

static const unsigned int emi_arbf_ddr3_32_val[] = {
#define X_CON_SCE(con_sce, arba, arbb, arbe, arbf, arbg) arbf,
#include "mach/con_sce_ddr3_32.h"
#undef X_CON_SCE
};

static const unsigned int emi_arbg_ddr3_32_val[] = {
#define X_CON_SCE(con_sce, arba, arbb, arbe, arbf, arbg) arbg,
#include "mach/con_sce_ddr3_32.h"
#undef X_CON_SCE
};

int get_ddr_type(void)
{
	unsigned int value;

	value = DRAMC_READ(DRAMC_LPDDR2);
	if ((value >> 28) & 0x1)	/* check LPDDR2_EN */
	{
		return LPDDR2;
	}

	value = DRAMC_READ(DRAMC_PADCTL4);
	if ((value >> 7) & 0x1)	/* check DDR3_EN */
	{
		if (DRAMC_READ(DRAMC_CONF1) & 0x1) {
			return DDR3_32;
		} else {
			return DDR3_16;
		}
	}
#if 0				/*to be ECO */
	value = DRAMC_READ(DRAMC_ACTIM1);
	if ((value >> 28) & 0x1)	/* check LPDDR3_EN */
	{
		return LPDDR3;
	}
#endif

	return mDDR;
}

/*
 * mtk_mem_bw_ctrl: set EMI bandwidth limiter for memory bandwidth control
 * @sce: concurrency scenario ID
 * @op: either ENABLE_CON_SCE or DISABLE_CON_SCE
 * Return 0 for success; return negative values for failure.
 */
int mtk_mem_bw_ctrl(int sce, int op)
{
	int i, highest;

	if (sce >= NR_CON_SCE) {
		return -1;
	}

	if (op != ENABLE_CON_SCE && op != DISABLE_CON_SCE) {
		return -1;
	}
	if (in_interrupt()) {
		return -1;
	}

	down(&emi_bwl_sem);

	if (op == ENABLE_CON_SCE) {
		ctrl_tbl[sce].ref_cnt++;
	} else if (op == DISABLE_CON_SCE) {
		if (ctrl_tbl[sce].ref_cnt != 0) {
			ctrl_tbl[sce].ref_cnt--;
		}
	}

	/* find the scenario with the highest priority */
	highest = -1;
	for (i = 0; i < NR_CON_SCE; i++) {
		if (ctrl_tbl[i].ref_cnt != 0) {
			highest = i;
			break;
		}
	}
	if (highest == -1) {
		highest = CON_SCE_NORMAL;
	}

	/* set new EMI bandwidth limiter value */
	if (highest != cur_con_sce) {

		if (get_ddr_type() == DDR3_32) {
			emi_writel(emi_arba_ddr3_32_val[highest], EMI_ARBA);
			emi_writel(emi_arba_ddr3_32_val[highest], EMI_ARBB);
			emi_writel(emi_arbe_ddr3_32_val[highest], EMI_ARBE);
			emi_writel(emi_arbf_ddr3_32_val[highest], EMI_ARBF);
			emi_reg_sync_writel(emi_arbg_ddr3_32_val[highest], EMI_ARBG);
		} else {
			pr_info("this dram type is not support!\n");
		}

		cur_con_sce = highest;
	}

	up(&emi_bwl_sem);

	return 0;
}

/*
 * ddr_type_show: sysfs ddr_type file show function.
 * @driver:
 * @buf: the string of ddr type
 * Return the number of read bytes.
 */
static ssize_t ddr_type_show(struct device_driver *driver, char *buf)
{
	if (get_ddr_type() == LPDDR2) {
		sprintf(buf, "LPDDR2\n");
	} else if (get_ddr_type() == DDR3_16) {
		sprintf(buf, "DDR3_16\n");
	} else if (get_ddr_type() == DDR3_32) {
		sprintf(buf, "DDR3_32\n");
	} else if (get_ddr_type() == LPDDR3) {
		sprintf(buf, "LPDDR3\n");
	} else {
		sprintf(buf, "mDDR\n");
	}

	return strlen(buf);
}

/*
 * ddr_type_store: sysfs ddr_type file store function.
 * @driver:
 * @buf:
 * @count:
 * Return the number of write bytes.
 */
static ssize_t ddr_type_store(struct device_driver *driver, const char *buf, size_t count)
{
	/*do nothing */
	return count;
}

DRIVER_ATTR(ddr_type, 0644, ddr_type_show, ddr_type_store);

/*
 * con_sce_show: sysfs con_sce file show function.
 * @driver:
 * @buf:
 * Return the number of read bytes.
 */
static ssize_t con_sce_show(struct device_driver *driver, char *buf)
{
	if (cur_con_sce >= NR_CON_SCE) {
		sprintf(buf, "none\n");
	} else {
		sprintf(buf, "%s\n", con_sce_str[cur_con_sce]);
	}

	return strlen(buf);
}

/*
 * con_sce_store: sysfs con_sce file store function.
 * @driver:
 * @buf:
 * @count:
 * Return the number of write bytes.
 */
static ssize_t con_sce_store(struct device_driver *driver, const char *buf, size_t count)
{
	int i;

	for (i = 0; i < NR_CON_SCE; i++) {
		if (!strncmp(buf, con_sce_str[i], strlen(con_sce_str[i]))) {
			if (!strncmp
			    (buf + strlen(con_sce_str[i]) + 1, EN_CON_SCE_STR,
			     strlen(EN_CON_SCE_STR))) {
				mtk_mem_bw_ctrl(i, ENABLE_CON_SCE);
				pr_info("concurrency scenario %s ON\n", con_sce_str[i]);
				break;
			} else
			    if (!strncmp
				(buf + strlen(con_sce_str[i]) + 1, DIS_CON_SCE_STR,
				 strlen(DIS_CON_SCE_STR))) {
				mtk_mem_bw_ctrl(i, DISABLE_CON_SCE);
				pr_info("concurrency scenario %s OFF\n", con_sce_str[i]);
				break;
			}
		}
	}

	return count;
}

DRIVER_ATTR(concurrency_scenario, 0644, con_sce_show, con_sce_store);


/*
 * finetune_md_show: sysfs con_sce file show function.
 * @driver:
 * @buf:
 * Return the number of read bytes.
 */
static ssize_t finetune_md_show(struct device_driver *driver, char *buf)
{
	unsigned int dram_type;

	dram_type = get_ddr_type();

	if (dram_type == LPDDR2) {	/*LPDDR2. FIXME */
		switch (cur_con_sce) {
		case CON_SCE_VR_MP4:
			sprintf(buf, "true");
			break;
		default:
			sprintf(buf, "false");
			break;
		}
	} else if (dram_type == DDR3_16) {	/*DDR3-16bit. FIXME */
	 /*TBD*/} else if (dram_type == DDR3_32) {	/*DDR3-32bit. FIXME */
	 /*TBD*/} else if (dram_type == mDDR) {	/*mDDR. FIXME */
	 /*TBD*/} else {
		/*unkown dram type */
		sprintf(buf, "ERROR: unkown dram type!");
	}

	return strlen(buf);
}

/*
 * finetune_md_store: sysfs con_sce file store function.
 * @driver:
 * @buf:
 * @count:
 * Return the number of write bytes.
 */
static ssize_t finetune_md_store(struct device_driver *driver, const char *buf, size_t count)
{
	/*Do nothing */
	return count;
}

DRIVER_ATTR(finetune_md, 0644, finetune_md_show, finetune_md_store);


/*
 * finetune_mm_show: sysfs con_sce file show function.
 * @driver:
 * @buf:
 * Return the number of read bytes.
 */
static ssize_t finetune_mm_show(struct device_driver *driver, char *buf)
{
	unsigned int dram_type;

	dram_type = get_ddr_type();

	if (dram_type == LPDDR2) {	/*LPDDR2. FIXME */
		switch (cur_con_sce) {
		default:
			sprintf(buf, "false");
			break;
		}
	} else if (dram_type == DDR3_16) {	/*DDR3-16bit. FIXME */
	 /*TBD*/} else if (dram_type == DDR3_32) {	/*DDR3-32bit. FIXME */
	 /*TBD*/} else if (dram_type == mDDR) {	/*mDDR. FIXME */
	 /*TBD*/} else {
		/*unkown dram type */
		sprintf(buf, "ERROR: unkown dram type!");
	}

	return strlen(buf);
}

/*
 * finetune_md_store: sysfs con_sce file store function.
 * @driver:
 * @buf:
 * @count:
 * Return the number of write bytes.
 */
static ssize_t finetune_mm_store(struct device_driver *driver, const char *buf, size_t count)
{
	/*Do nothing */
	return count;
}

DRIVER_ATTR(finetune_mm, 0644, finetune_mm_show, finetune_mm_store);


/*
 * dramc_high_show: show the status of DRAMC_HI_EN
 * @driver:
 * @buf:
 * Return the number of read bytes.
 */
static ssize_t dramc_high_show(struct device_driver *driver, char *buf)
{
	unsigned int dramc_hi;

	dramc_hi = (emi_readl(EMI_TESTB) >> 12) & 0x1;
	if (dramc_hi == 1)
		return sprintf(buf, "DRAMC_HI is ON\n");
	else
		return sprintf(buf, "DRAMC_HI is OFF\n");
}

/*
 dramc_hign_store: enable/disable DRAMC untra high. WARNING: ONLY CAN BE ENABLED AT MD_STANDALONE!!!
 * @driver:
 * @buf: need to be "0" or "1"
 * @count:
 * Return the number of write bytes.
*/
static ssize_t dramc_high_store(struct device_driver *driver, const char *buf, size_t count)
{
	unsigned int value;
	unsigned int emi_testb;

	if (sscanf(buf, "%u", &value) != 1)
		return -EINVAL;

	emi_testb = emi_readl(EMI_TESTB);

	if (value == 1) {
		emi_testb |= 0x1000;	/* Enable DRAM_HI */
		emi_writel(emi_testb, EMI_TESTB);
	} else if (value == 0) {
		emi_testb &= ~0x1000;	/* Disable DRAM_HI */
		emi_writel(emi_testb, EMI_TESTB);
	} else
		return -EINVAL;

	return count;
}

DRIVER_ATTR(dramc_high, 0644, dramc_high_show, dramc_high_store);

/*
 * emi_bwl_mod_init: module init function.
 */
static int __init emi_bwl_mod_init(void)
{
	int ret;

	{
		emi_writel(0x10202488, EMI_ARBI);
		emi_writel(0x40105808, EMI_ARBA);
		emi_writel(0x40100000, EMI_ARBB);
		emi_writel(0xa0a05004, EMI_ARBE);
		emi_writel(0xa0a05001, EMI_ARBF);
		emi_writel(0xa0a05028, EMI_ARBG);
		emi_writel(0x84462f2f, EMI_ARBJ);
		emi_writel(0x0f1f2534, EMI_CONB);
		emi_writel(0x11211323, EMI_CONC);
		emi_writel(0x0f1f2534, EMI_COND);
		emi_writel(0x11211323, EMI_CONE);
		emi_writel(0x2f312c2f, EMI_CONG);
		emi_writel(0x2f312c2f, EMI_CONH);
		emi_writel(0x80007200, EMI_SLCT);	/* DE suggest to support ultra sideband */
		emi_writel(0x00720038, EMI_ARBK);
	}

	ret = mtk_mem_bw_ctrl(CON_SCE_NORMAL, ENABLE_CON_SCE);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL", "fail to set EMI bandwidth limiter\n");
	}

	/* Register BW ctrl interface */
	ret = platform_driver_register(&mem_bw_ctrl);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL",
			    "fail to register EMI_BW_LIMITER driver\n");
	}

	ret = driver_create_file(&mem_bw_ctrl.driver, &driver_attr_concurrency_scenario);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL",
			    "fail to create EMI_BW_LIMITER sysfs file\n");
	}

	/* Register DRAM type information interface */
	ret = platform_driver_register(&ddr_type);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL", "fail to register DRAM_TYPE driver\n");
	}

	ret = driver_create_file(&ddr_type.driver, &driver_attr_ddr_type);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL", "fail to create DRAM_TYPE sysfs file\n");
	}

	/* Register DRAMC ultra high interface */
	ret = platform_driver_register(&dramc_high);
	ret = driver_create_file(&dramc_high.driver, &driver_attr_dramc_high);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL", "fail to create DRAMC_HIGH sysfs file\n");
	}
#if 0
	/* Register MD feature fine-tune interface */
	ret = platform_driver_register(&mem_bw_finetune_md);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL",
			    "fail to register EMI_BW_FINETUNE_MD driver\n");
	}

	ret = driver_create_file(&mem_bw_finetune_md.driver, &driver_attr_finetune_md);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL",
			    "fail to create EMI_BW_FINETUNE_MD sysfs file\n");
	}

	/* Register MM feature fine-tune interface */
	ret = platform_driver_register(&mem_bw_finetune_mm);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL",
			    "fail to register EMI_BW_FINETUNE_MM driver\n");
	}

	ret = driver_create_file(&mem_bw_finetune_mm.driver, &driver_attr_finetune_mm);
	if (ret) {
		xlog_printk(ANDROID_LOG_ERROR, "EMI/BWL",
			    "fail to create EMI_BW_FINETUNE_MM sysfs file\n");
	}
#endif

	return 0;
}

/*
 * emi_bwl_mod_exit: module exit function.
 */
static void __exit emi_bwl_mod_exit(void)
{
}
EXPORT_SYMBOL(get_ddr_type);

module_init(emi_bwl_mod_init);
module_exit(emi_bwl_mod_exit);
