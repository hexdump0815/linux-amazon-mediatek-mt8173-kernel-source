#include <linux/power/mt_battery_custom_data.h>
#include <linux/power/mt_battery_meter.h>
#include <linux/of.h>
#include <linux/device.h>

/* T0 -10C */
static BATTERY_PROFILE_STRUC p1v1_custom_battery_profile_t0[] = {
	{0,  4082},
	{2,  4062},
	{5,  4050},
	{7,  4036},
	{10, 4024},
	{12, 4013},
	{15, 4001},
	{17, 3990},
	{20, 3977},
	{22, 3949},
	{25, 3893},
	{27, 3875},
	{30, 3862},
	{32, 3850},
	{35, 3840},
	{37, 3829},
	{39, 3820},
	{42, 3815},
	{44, 3809},
	{47, 3804},
	{49, 3799},
	{52, 3795},
	{54, 3792},
	{57, 3788},
	{59, 3787},
	{62, 3784},
	{64, 3778},
	{67, 3773},
	{69, 3768},
	{72, 3761},
	{74, 3753},
	{77, 3743},
	{79, 3732},
	{81, 3721},
	{84, 3712},
	{86, 3705},
	{88, 3701},
	{89, 3698},
	{90, 3695},
	{91, 3691},
	{92, 3688},
	{93, 3687},
	{94, 3685},
	{94, 3681},
	{95, 3678},
	{96, 3674},
	{96, 3669},
	{97, 3666},
	{97, 3662},
	{97, 3657},
	{98, 3652},
	{98, 3647},
	{98, 3643},
	{98, 3639},
	{99, 3634},
	{99, 3630},
	{99, 3628},
	{99, 3624},
	{99, 3622},
	{99, 3621},
	{99, 3619},
	{99, 3617},
	{99, 3616},
	{99, 3613},
	{99, 3612},
	{99, 3612},
	{100, 3610},
	{100, 3610},
	{100, 3607},
};

/* T1 0C */
static BATTERY_PROFILE_STRUC p1v1_custom_battery_profile_t1[] = {
	{0  , 4167},
	{2  , 4142},
	{3  , 4089},
	{5  , 4062},
	{7  , 4046},
	{9  , 4030},
	{10 , 4016},
	{12 , 4004},
	{14 , 3995},
	{16 , 3984},
	{17 , 3975},
	{19 , 3964},
	{21 , 3954},
	{23 , 3944},
	{24 , 3934},
	{26 , 3924},
	{28 , 3912},
	{29 , 3901},
	{31 , 3888},
	{33 , 3877},
	{35 , 3865},
	{36 , 3854},
	{38 , 3845},
	{40 , 3838},
	{42 , 3830},
	{43 , 3824},
	{45 , 3819},
	{47 , 3813},
	{48 , 3809},
	{50 , 3804},
	{52 , 3798},
	{54 , 3796},
	{55 , 3792},
	{57 , 3787},
	{59 , 3785},
	{61 , 3783},
	{62 , 3780},
	{64 , 3779},
	{66 , 3776},
	{68 , 3773},
	{69 , 3770},
	{71 , 3768},
	{73 , 3764},
	{74 , 3760},
	{76 , 3754},
	{78 , 3747},
	{80 , 3741},
	{81 , 3732},
	{83 , 3722},
	{85 , 3711},
	{87 , 3700},
	{88 , 3693},
	{90 , 3688},
	{92 , 3683},
	{94 , 3676},
	{95 , 3659},
	{97 , 3629},
	{97 , 3602},
	{98 , 3580},
	{98 , 3563},
	{99 , 3550},
	{99 , 3539},
	{99 , 3529},
	{99 , 3520},
	{99 , 3513},
	{99 , 3507},
	{99 , 3502},
	{99 , 3498},
	{100, 3494},
};

/* T2 25C */
static BATTERY_PROFILE_STRUC p1v1_custom_battery_profile_t2[] = {
	{0  , 4176},
	{2  , 4158},
	{3  , 4142},
	{5  , 4127},
	{6  , 4113},
	{8  , 4098},
	{9  , 4087},
	{11 , 4072},
	{12 , 4060},
	{14 , 4048},
	{15 , 4035},
	{17 , 4022},
	{19 , 4013},
	{20 , 4000},
	{22 , 3990},
	{23 , 3980},
	{25 , 3970},
	{26 , 3960},
	{28 , 3951},
	{29 , 3942},
	{31 , 3934},
	{32 , 3926},
	{34 , 3916},
	{35 , 3906},
	{37 , 3897},
	{39 , 3885},
	{40 , 3873},
	{42 , 3860},
	{43 , 3849},
	{45 , 3839},
	{46 , 3832},
	{48 , 3825},
	{49 , 3820},
	{51 , 3814},
	{52 , 3809},
	{54 , 3804},
	{56 , 3800},
	{57 , 3795},
	{59 , 3791},
	{60 , 3789},
	{62 , 3785},
	{63 , 3782},
	{65 , 3778},
	{66 , 3776},
	{68 , 3773},
	{69 , 3772},
	{71 , 3768},
	{73 , 3765},
	{74 , 3762},
	{76 , 3756},
	{77 , 3751},
	{79 , 3746},
	{80 , 3741},
	{82 , 3734},
	{83 , 3724},
	{85 , 3715},
	{86 , 3704},
	{88 , 3691},
	{89 , 3687},
	{91 , 3684},
	{93 , 3683},
	{94 , 3678},
	{96 , 3665},
	{97 , 3610},
	{99 , 3523},
	{100, 3378},
	{101, 3324},
	{101, 3311},
	{101, 3306},
};

/* T3 50C */
static BATTERY_PROFILE_STRUC p1v1_custom_battery_profile_t3[] = {
	{0  , 4184},
	{2  , 4167},
	{3  , 4154},
	{5  , 4137},
	{6  , 4124},
	{8  , 4111},
	{9  , 4097},
	{11 , 4084},
	{12 , 4071},
	{14 , 4057},
	{15 , 4046},
	{17 , 4033},
	{18 , 4020},
	{20 , 4009},
	{21 , 3998},
	{23 , 3988},
	{24 , 3976},
	{26 , 3967},
	{27 , 3958},
	{29 , 3947},
	{31 , 3937},
	{32 , 3930},
	{34 , 3921},
	{35 , 3910},
	{37 , 3903},
	{38 , 3894},
	{40 , 3883},
	{41 , 3869},
	{43 , 3856},
	{44 , 3844},
	{46 , 3836},
	{47 , 3830},
	{49 , 3823},
	{50 , 3817},
	{52 , 3811},
	{53 , 3807},
	{55 , 3802},
	{57 , 3797},
	{58 , 3793},
	{60 , 3789},
	{61 , 3785},
	{63 , 3782},
	{64 , 3779},
	{66 , 3775},
	{67 , 3772},
	{69 , 3768},
	{70 , 3763},
	{72 , 3755},
	{73 , 3749},
	{75 , 3744},
	{76 , 3739},
	{78 , 3733},
	{79 , 3727},
	{81 , 3721},
	{83 , 3715},
	{84 , 3703},
	{86 , 3695},
	{87 , 3683},
	{89 , 3674},
	{90 , 3671},
	{92 , 3669},
	{93 , 3666},
	{95 , 3661},
	{96 , 3634},
	{98 , 3570},
	{99 , 3472},
	{101, 3311},
	{101, 3289},
	{101, 3282},
};

/* T0 -10C */
static R_PROFILE_STRUC p1v1_custom_r_profile_t0[] = {
	{345, 4082},
	{345, 4062},
	{367, 4050},
	{367, 4036},
	{377, 4024},
	{378, 4013},
	{388, 4001},
	{398, 3990},
	{420, 3977},
	{453, 3949},
	{637, 3893},
	{690, 3875},
	{698, 3862},
	{698, 3850},
	{697, 3840},
	{705, 3829},
	{695, 3820},
	{705, 3815},
	{702, 3809},
	{708, 3804},
	{710, 3799},
	{715, 3795},
	{720, 3792},
	{725, 3788},
	{732, 3787},
	{743, 3784},
	{743, 3778},
	{753, 3773},
	{757, 3768},
	{770, 3761},
	{782, 3753},
	{792, 3743},
	{805, 3732},
	{812, 3721},
	{835, 3712},
	{842, 3705},
	{837, 3701},
	{832, 3698},
	{825, 3695},
	{820, 3691},
	{815, 3688},
	{812, 3687},
	{810, 3685},
	{803, 3681},
	{798, 3678},
	{792, 3674},
	{782, 3669},
	{777, 3666},
	{770, 3662},
	{768, 3657},
	{755, 3652},
	{747, 3647},
	{740, 3643},
	{733, 3639},
	{728, 3634},
	{720, 3630},
	{713, 3628},
	{708, 3624},
	{705, 3622},
	{705, 3621},
	{702, 3619},
	{697, 3617},
	{693, 3616},
	{692, 3613},
	{688, 3612},
	{687, 3612},
	{687, 3610},
	{685, 3610},
	{682, 3607},
};

/* T1 0C */
R_PROFILE_STRUC p1v1_custom_r_profile_t1[] = {
	{235, 4167},
	{235, 4142},
	{252, 4089},
	{343, 4062},
	{357, 4046},
	{362, 4030},
	{363, 4016},
	{368, 4004},
	{377, 3995},
	{377, 3984},
	{385, 3975},
	{382, 3964},
	{387, 3954},
	{390, 3944},
	{388, 3934},
	{395, 3924},
	{388, 3912},
	{392, 3901},
	{383, 3888},
	{385, 3877},
	{378, 3865},
	{377, 3854},
	{375, 3845},
	{378, 3838},
	{382, 3830},
	{380, 3824},
	{383, 3819},
	{387, 3813},
	{390, 3809},
	{388, 3804},
	{393, 3798},
	{393, 3796},
	{397, 3792},
	{400, 3787},
	{398, 3785},
	{408, 3783},
	{407, 3780},
	{417, 3779},
	{415, 3776},
	{420, 3773},
	{423, 3770},
	{433, 3768},
	{435, 3764},
	{447, 3760},
	{453, 3754},
	{457, 3747},
	{477, 3741},
	{480, 3732},
	{492, 3722},
	{503, 3711},
	{520, 3700},
	{537, 3693},
	{565, 3688},
	{608, 3683},
	{652, 3676},
	{697, 3659},
	{715, 3629},
	{672, 3602},
	{637, 3580},
	{607, 3563},
	{585, 3550},
	{567, 3539},
	{548, 3529},
	{533, 3520},
	{523, 3513},
	{515, 3507},
	{507, 3502},
	{498, 3498},
	{495, 3494},
};

/* T2 25C */
static R_PROFILE_STRUC p1v1_custom_r_profile_t2[] = {
	{150, 4176},
	{150, 4158},
	{148, 4142},
	{150, 4127},
	{152, 4113},
	{150, 4098},
	{155, 4087},
	{157, 4072},
	{158, 4060},
	{158, 4048},
	{162, 4035},
	{162, 4022},
	{167, 4013},
	{167, 4000},
	{168, 3990},
	{170, 3980},
	{172, 3970},
	{173, 3960},
	{177, 3951},
	{177, 3942},
	{183, 3934},
	{183, 3926},
	{183, 3916},
	{182, 3906},
	{182, 3897},
	{177, 3885},
	{173, 3873},
	{163, 3860},
	{160, 3849},
	{155, 3839},
	{153, 3832},
	{153, 3825},
	{155, 3820},
	{155, 3814},
	{157, 3809},
	{157, 3804},
	{158, 3800},
	{158, 3795},
	{158, 3791},
	{160, 3789},
	{158, 3785},
	{162, 3782},
	{158, 3778},
	{160, 3776},
	{158, 3773},
	{162, 3772},
	{160, 3768},
	{158, 3765},
	{160, 3762},
	{158, 3756},
	{157, 3751},
	{155, 3746},
	{158, 3741},
	{158, 3734},
	{157, 3724},
	{157, 3715},
	{158, 3704},
	{157, 3691},
	{158, 3687},
	{162, 3684},
	{173, 3683},
	{185, 3678},
	{197, 3665},
	{180, 3610},
	{192, 3523},
	{220, 3378},
	{210, 3324},
	{188, 3311},
	{182, 3306},
};

/* T3 50C */
static R_PROFILE_STRUC p1v1_custom_r_profile_t3[] = {
	{120, 4184},
	{120, 4167},
	{123, 4154},
	{118, 4137},
	{123, 4124},
	{125, 4111},
	{125, 4097},
	{125, 4084},
	{127, 4071},
	{127, 4057},
	{130, 4046},
	{127, 4033},
	{127, 4020},
	{128, 4009},
	{130, 3998},
	{132, 3988},
	{132, 3976},
	{135, 3967},
	{138, 3958},
	{138, 3947},
	{135, 3937},
	{142, 3930},
	{143, 3921},
	{142, 3910},
	{148, 3903},
	{147, 3894},
	{145, 3883},
	{138, 3869},
	{132, 3856},
	{127, 3844},
	{123, 3836},
	{123, 3830},
	{123, 3823},
	{123, 3817},
	{125, 3811},
	{127, 3807},
	{127, 3802},
	{127, 3797},
	{127, 3793},
	{128, 3789},
	{130, 3785},
	{130, 3782},
	{133, 3779},
	{130, 3775},
	{133, 3772},
	{132, 3768},
	{130, 3763},
	{123, 3755},
	{125, 3749},
	{127, 3744},
	{128, 3739},
	{125, 3733},
	{127, 3727},
	{125, 3721},
	{128, 3715},
	{122, 3703},
	{128, 3695},
	{123, 3683},
	{123, 3674},
	{125, 3671},
	{130, 3669},
	{135, 3666},
	{142, 3661},
	{135, 3634},
	{140, 3570},
	{148, 3472},
	{180, 3311},
	{148, 3289},
	{138, 3282},
};

/* battery profile for actual temperature. The size should be the same as T1, T2 and T3 */
static BATTERY_PROFILE_STRUC p1v1_custom_battery_profile_temperature[] = {
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
};

/* r-table profile for actual temperature. The size should be the same as T1, T2 and T3 */
static R_PROFILE_STRUC p1v1_custom_r_profile_temperature[] = {
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
};

static BATT_TEMPERATURE p1v1_bat_temperature_table[] = {
	{-20, 499900},
	{-15, 371600},
	{-10, 278700},
	{-5, 210800},
	{0, 160800},
	{5, 123800},
	{10, 96030},
	{15, 75100},
	{20, 59190},
	{25, 47000},
	{30, 37590},
	{35, 30270},
	{40, 24540},
	{45, 20020},
	{50, 16430},
	{55, 13570},
	{60, 11270},
	{65, 9409},
	{70, 7897}
};

void p1v1_custom_battery_init(mt_battery_meter_custom_data *p_meter_data)
{
	p_meter_data->car_tune_value = 102;
	/* NTC 47K */
	p_meter_data->rbat_pull_up_r = 24000;
	p_meter_data->rbat_pull_down_r = 100000000;
	p_meter_data->cust_r_fg_offset = 0;

	p_meter_data->p_batt_temperature_table = p1v1_bat_temperature_table;
	p_meter_data->battery_ntc_table_saddles =
	    sizeof(p1v1_bat_temperature_table) / sizeof(BATT_TEMPERATURE);

	/* load abc123/atl profile */
	p_meter_data->q_max_pos_50 = 3535;
	p_meter_data->q_max_pos_50_h_current = 3502;
	p_meter_data->q_max_pos_25 = 3452;
	p_meter_data->q_max_pos_25_h_current = 3409;
	p_meter_data->q_max_pos_0 = 2819;
	p_meter_data->q_max_pos_0_h_current = 2370;
	p_meter_data->q_max_neg_10 = 2183;
	p_meter_data->q_max_neg_10_h_current = 876;

	p_meter_data->battery_profile_saddles =
	    sizeof(p1v1_custom_battery_profile_temperature) / sizeof(BATTERY_PROFILE_STRUC);
	p_meter_data->battery_r_profile_saddles =
	    sizeof(p1v1_custom_r_profile_temperature) / sizeof(R_PROFILE_STRUC);

	p_meter_data->p_battery_profile_t0 = p1v1_custom_battery_profile_t0;
	p_meter_data->p_battery_profile_t1 = p1v1_custom_battery_profile_t1;
	p_meter_data->p_battery_profile_t2 = p1v1_custom_battery_profile_t2;
	p_meter_data->p_battery_profile_t3 = p1v1_custom_battery_profile_t3;
	p_meter_data->p_r_profile_t0 = p1v1_custom_r_profile_t0;
	p_meter_data->p_r_profile_t1 = p1v1_custom_r_profile_t1;
	p_meter_data->p_r_profile_t2 = p1v1_custom_r_profile_t2;
	p_meter_data->p_r_profile_t3 = p1v1_custom_r_profile_t3;

	p_meter_data->p_battery_profile_temperature = p1v1_custom_battery_profile_temperature;
	p_meter_data->p_r_profile_temperature = p1v1_custom_r_profile_temperature;
}

typedef struct mt_bm_item {
	const char *battery_name;
	void (*func) (mt_battery_meter_custom_data *p_meter_data);
} mt_bm_item;

static mt_bm_item mt_battery_profiles[] = {
	{"p1v1_battery", p1v1_custom_battery_init}
};

int mt_bm_of_probe(struct device *dev, mt_battery_meter_custom_data *p_meter_data)
{
#ifdef CONFIG_OF
	struct device_node *of_node = dev->of_node;
	const char *bname;
	int i;

	bname = of_get_property(of_node, "battery", NULL);
	if (!bname) {
		dev_warn(dev, "No custom battery profile. Use default setting.\n");
		return 0;
	}

	for (i = 0; i < sizeof(mt_battery_profiles) / sizeof(mt_bm_item); i++) {
		if (strcmp(bname, mt_battery_profiles[i].battery_name) == 0) {
			dev_warn(dev, "match battery profile: %s\n", bname);
			mt_battery_profiles[i].func(p_meter_data);
			break;
		}
	}

	of_node_put(of_node);

#endif
	return 0;
}
