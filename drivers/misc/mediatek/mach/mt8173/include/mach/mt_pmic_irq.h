#ifndef __MT_PMIC_IRQ_H__
#define __MT_PMIC_IRQ_H__

#define NR_INT_STATUS0     (16)
#define NR_INT_STATUS1     (16)
#define NR_PMIC_IRQ  (NR_INT_STATUS0 + NR_INT_STATUS1)

#define PMIC_INT_MAX_NUM  NR_PMIC_IRQ
#define PMIC_IRQ_BASE (NR_MT_IRQ_LINE + 224)	/* After EINT's irq */

#define PMIC_IRQ(x) ((x) + PMIC_IRQ_BASE)

enum PMIC_INT_STATUS_GRP {
	GRP_INT_STATUS0 = 0,
	GRP_INT_STATUS1,
};

enum PMIC_INT_STATUS {
	RG_INT_STATUS_SPKL_AB = 0,
	RG_INT_STATUS_SPKR_AB = 1,
	RG_INT_STATUS_SPKL = 2,
	RG_INT_STATUS_SPKR = 3,
	RG_INT_STATUS_BAT_L = 4,
	RG_INT_STATUS_BAT_H = 5,
	RG_INT_STATUS_FG_BAT_L = 6,
	RG_INT_STATUS_FG_BAT_H = 7,
	RG_INT_STATUS_WATCHDOG = 8,
	RG_INT_STATUS_PWRKEY = 9,
	RG_INT_STATUS_THR_L = 10,
	RG_INT_STATUS_THR_H = 11,
	RG_INT_STATUS_VBATON_UNDET = 12,
	RG_INT_STATUS_BVALID_DET = 13,
	RG_INT_STATUS_CHRDET = 14,
	RG_INT_STATUS_OV = 15,
	RG_INT_STATUS_LDO = (16 + 0),
	RG_INT_STATUS_HOMEKEY = (16 + 1),
	RG_INT_STATUS_ACCDET = (16 + 2),
	RG_INT_STATUS_AUDIO = (16 + 3),
	RG_INT_STATUS_RTC = (16 + 4),
	RG_INT_STATUS_PWRKEY_RSTB = (16 + 5),
	RG_INT_STATUS_HDMI_SIFM = (16 + 6),
	RG_INT_STATUS_HDMI_CEC = (16 + 7),
	RG_INT_STATUS_VCA15 = (16 + 8),
	RG_INT_STATUS_VSRMCA15 = (16 + 9),
	RG_INT_STATUS_VCORE = (16 + 10),
	RG_INT_STATUS_VGPU = (16 + 11),
	RG_INT_STATUS_VIO18 = (16 + 12),
	RG_INT_STATUS_VPCA7 = (16 + 13),
	RG_INT_STATUS_VSRMCA7 = (16 + 14),
	RG_INT_STATUS_VDRM = (16 + 15),
};

#endif				/*  !MT_PMIC_IRQ_H__ */
