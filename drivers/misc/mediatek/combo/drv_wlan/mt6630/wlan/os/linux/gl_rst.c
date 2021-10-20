/*
** $Id: @(#) gl_rst.c@@
*/

/*! \file   gl_rst.c
    \brief  Main routines for supporintg MT6620 whole-chip reset mechanism

    This file contains the support routines of Linux driver for MediaTek Inc. 802.11
    Wireless LAN Adapters.
*/



/*
** $Log: gl_rst.c $
**
** 09 17 2012 cm.chang
** [BORA00002149] [MT6630 Wi-Fi] Initial software development
** Duplicate source from MT6620 v2.3 driver branch
** (Davinci label: MT6620_WIFI_Driver_V2_3_120913_1942_As_MT6630_Base)
 *
 * 11 10 2011 cp.wu
 * [WCXRP00001098] [MT6620 Wi-Fi][Driver] Replace printk by DBG LOG macros in linux porting layer
 * 1. eliminaite direct calls to printk in porting layer.
 * 2. replaced by DBGLOG, which would be XLOG on ALPS platforms.
 *
 * 04 22 2011 cp.wu
 * [WCXRP00000598] [MT6620 Wi-Fi][Driver] Implementation of interface for communicating with user space process for RESET_START and RESET_END events
 * skip power-off handshaking when RESET indication is received.
 *
 * 04 14 2011 cp.wu
 * [WCXRP00000598] [MT6620 Wi-Fi][Driver] Implementation of interface for communicating with user space process for RESET_START and RESET_END events
 * 1. add code to put whole-chip resetting trigger when abnormal firmware assertion is detected
 * 2. add dummy function for both Win32 and Linux part.
 *
 * 03 30 2011 cp.wu
 * [WCXRP00000598] [MT6620 Wi-Fi][Driver] Implementation of interface for communicating with user space process for RESET_START and RESET_END events
 * use netlink unicast instead of broadcast
 *
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "gl_rst.h"
#include "precomp.h"
#include <linux/poll.h>
#include <net/netlink.h>
#include <net/genetlink.h>

#if CFG_CHIP_RESET_SUPPORT

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define MAX_BIND_PROCESS    (4)

#define MTK_WIFI_FAMILY_NAME        "MTK_WIFI"
#define MTK_WIFI_RESET_START_NAME   "RESET_START"
#define MTK_WIFI_RESET_END_NAME     "RESET_END"
#define MTK_WIFI_RESET_TEST_NAME    "GENETLINK_START"

#define CFG_WMT_RESET_API_SUPPORT   0

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
enum {
	__MTK_WIFI_ATTR_INVALID,
	MTK_WIFI_ATTR_MSG,
	__MTK_WIFI_ATTR_MAX,
};
#define MTK_WIFI_ATTR_MAX       (__MTK_WIFI_ATTR_MAX - 1)


enum {
	__MTK_WIFI_COMMAND_INVALID,
	MTK_WIFI_COMMAND_BIND,
	MTK_WIFI_COMMAND_RESET,
	__MTK_WIFI_COMMAND_MAX,
};
#define MTK_WIFI_COMMAND_MAX    (__MTK_WIFI_COMMAND_MAX - 1)

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
/* static UINT_32 mtk_wifi_seqnum = 0; */
static int num_bind_process;
static UINT_32 bind_id[MAX_BIND_PROCESS];
static BOOLEAN fgIsResetting = FALSE;
static struct work_struct work_rst;

/* attribute policy */
static struct nla_policy mtk_wifi_genl_policy[MTK_WIFI_ATTR_MAX + 1] = {
	[MTK_WIFI_ATTR_MSG] = {.type = NLA_NUL_STRING},
};

/* family definition */
static struct genl_family mtk_wifi_gnl_family = {
	.id = GENL_ID_GENERATE,
	.hdrsize = 0,
	.name = MTK_WIFI_FAMILY_NAME,
	.version = 1,
	.maxattr = MTK_WIFI_ATTR_MAX,
};

/* forward declaration */
static int mtk_wifi_bind(struct sk_buff *skb, struct genl_info *info);

static void mtk_wifi_reset(struct work_struct *prWorkStruct);

/* operation definition */
static struct genl_ops mtk_wifi_gnl_ops_bind = {
	.cmd = MTK_WIFI_COMMAND_BIND,
	.flags = 0,
	.policy = mtk_wifi_genl_policy,
	.doit = mtk_wifi_bind,
	.dumpit = NULL,
};

/*
static struct genl_ops mtk_wifi_gnl_ops_reset = {
    .cmd = MTK_WIFI_COMMAND_RESET,
    .flags  = 0,
    .policy = mtk_wifi_genl_policy,
    .doit   = mtk_wifi_reset,
    .dumpit = NULL,
};
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/



static void *glResetCallback(ENUM_WMTDRV_TYPE_T eSrcType,
			     ENUM_WMTDRV_TYPE_T eDstType,
			     ENUM_WMTMSG_TYPE_T eMsgType,
			     void *prMsgBody, unsigned int u4MsgLength);
/*
static BOOLEAN
glResetSendMessage (
    char    *aucMsg,
    u8      cmd
    );
*/

#if CFG_WMT_RESET_API_SUPPORT
extern MTK_WCN_BOOL mtk_wcn_wmt_do_reset(ENUM_WMTDRV_TYPE_T type);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for
 *        1. registering for reset callbacks
 *        2. initialize netlink socket
 *
 * @param none
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
VOID glResetInit(VOID)
{
	/* 1. register for reset callback */
	mtk_wcn_wmt_msgcb_reg(WMTDRV_TYPE_WIFI, (PF_WMT_CB) glResetCallback);

	/* 2.1 registration for NETLINK_GENERIC family */
	if (genl_register_family(&mtk_wifi_gnl_family) != 0) {
		DBGLOG(INIT, WARN, ("%s(): GE_NELINK family registration fail\n", __func__));
	} else {
		/* 2.2 operation registration */
		if (genl_register_ops(&mtk_wifi_gnl_family, &mtk_wifi_gnl_ops_bind) != 0) {
			DBGLOG(INIT, WARN, ("%s(): BIND operation registration fail\n", __func__));
		}
		/*
		   if(genl_register_ops(&mtk_wifi_gnl_family, &mtk_wifi_gnl_ops_reset) != 0) {
		   DBGLOG(INIT, WARN, ("%s(): RESET operation registration fail\n", __func__));
		   }
		 */
	}

	INIT_WORK(&work_rst, mtk_wifi_reset);

	return;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for
 *        1. uninitialize netlink socket
 *        2. deregistering for reset callbacks
 *
 * @param none
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
VOID glResetUninit(VOID)
{
	/* 1. release NETLINK_GENERIC family */
	genl_unregister_family(&mtk_wifi_gnl_family);

	/* 2. deregister for reset callback */
	mtk_wcn_wmt_msgcb_unreg(WMTDRV_TYPE_WIFI);

	return;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is invoked when there is reset messages indicated
 *
 * @param   eSrcType
 *          eDstType
 *          eMsgType
 *          prMsgBody
 *          u4MsgLength
 *
 * @retval
 */
/*----------------------------------------------------------------------------*/
static void *glResetCallback(ENUM_WMTDRV_TYPE_T eSrcType,
			     ENUM_WMTDRV_TYPE_T eDstType,
			     ENUM_WMTMSG_TYPE_T eMsgType, void *prMsgBody, unsigned int u4MsgLength)
{
	switch (eMsgType) {
	case WMTMSG_TYPE_RESET:
		if (u4MsgLength == sizeof(ENUM_WMTRSTMSG_TYPE_T)) {
			P_ENUM_WMTRSTMSG_TYPE_T prRstMsg = (P_ENUM_WMTRSTMSG_TYPE_T) prMsgBody;

			switch (*prRstMsg) {
			case WMTRSTMSG_RESET_START:
				DBGLOG(INIT, WARN, ("Whole chip reset start!\n"));
				fgIsResetting = TRUE;
				/* glResetSendMessage(MTK_WIFI_RESET_START_NAME, MTK_WIFI_COMMAND_RESET); */
				wifi_reset_start();
				break;

			case WMTRSTMSG_RESET_END_FAIL:
			case WMTRSTMSG_RESET_END:
				DBGLOG(INIT, WARN,
				       ("Whole chip reset end, message[%u]!\n", *prRstMsg));
				schedule_work(&work_rst);
				/* glResetSendMessage(MTK_WIFI_RESET_END_NAME, MTK_WIFI_COMMAND_RESET); */
				fgIsResetting = FALSE;
				break;

			default:
				break;
			}
		}

		break;

	default:
		break;
	}

	return NULL;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine send out message via netlink socket
 *
 * @param   aucMsg
 *          u4MsgLength
 *
 * @retval  TRUE
 *          FALSE
 */
/*----------------------------------------------------------------------------*/
#if 0
static BOOLEAN glResetSendMessage(char *aucMsg, u8 cmd)
{
	struct sk_buff *skb = NULL;
	void *msg_head = NULL;
	int rc = -1;
	int i;

	if (num_bind_process == 0) {
		/* no listening process */
		return FALSE;
	}

	for (i = 0; i < num_bind_process; i++) {
		skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);

		if (skb) {
			msg_head =
			    genlmsg_put(skb, 0, mtk_wifi_seqnum++, &mtk_wifi_gnl_family, 0, cmd);

			if (msg_head == NULL) {
				nlmsg_free(skb);
				return FALSE;
			}

			rc = nla_put_string(skb, MTK_WIFI_ATTR_MSG, aucMsg);
			if (rc != 0) {
				nlmsg_free(skb);
				return FALSE;
			}

			/* finalize the message */
			genlmsg_end(skb, msg_head);

			/* sending message */
			rc = genlmsg_unicast(&init_net, skb, bind_id[i]);
			if (rc != 0) {
				return FALSE;
			}
		} else {
			return FALSE;
		}
	}

	return TRUE;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called to identify PID for process binding
 *
 * @param   skb
 *          info
 *
 * @retval  0
 *          nonzero
 */
/*----------------------------------------------------------------------------*/
int mtk_wifi_bind(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	char *mydata;

	if (info == NULL) {
		goto out;
	}

	/*for each attribute there is an index in info->attrs which points to a nlattr structure
	 *in this structure the data is given
	 */

	na = info->attrs[MTK_WIFI_ATTR_MSG];
	if (na) {
		mydata = (char *)nla_data(na);

		/* no need to parse mydata */
	}

	/* collect PID */
	if (num_bind_process < MAX_BIND_PROCESS) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
		bind_id[num_bind_process] = info->snd_portid;
#else
		bind_id[num_bind_process] = info->snd_pid;
#endif
		num_bind_process++;
	} else {
		DBGLOG(INIT, WARN,
		       ("%s(): exceeding binding limit %d\n", __func__, MAX_BIND_PROCESS));
	}

 out:
	return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for wifi reset
 *
 * @param   skb
 *          info
 *
 * @retval  0
 *          nonzero
 */
/*----------------------------------------------------------------------------*/
static void mtk_wifi_reset(struct work_struct *prWorkStruct)
{
	wifi_reset_end();
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for generating reset request to WMT
 *
 * @param   None
 *
 * @retval  None
 */
/*----------------------------------------------------------------------------*/
VOID glSendResetRequest(VOID)
{
	/* WMT thread would trigger whole chip resetting itself */
	return;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for checking if MT6620 is resetting
 *
 * @param   None
 *
 * @retval  TRUE
 *          FALSE
 */
/*----------------------------------------------------------------------------*/
BOOLEAN kalIsResetting(VOID)
{
	return fgIsResetting;
}

BOOLEAN glResetTrigger(P_ADAPTER_T prAdapter)
{
	BOOLEAN fgResult = TRUE;

#if CFG_WMT_RESET_API_SUPPORT
	DBGLOG(INIT, ERROR,
	       ("Try to trigger whole-chip reset! Chip[%04X E%u] FW Ver DEC[%u.%u] HEX[%x.%x], Driver Ver[%u.%u]\n",
		MTK_CHIP_REV, wlanGetEcoVersion(prAdapter),
		(prAdapter->rVerInfo.u2FwOwnVersion >> 8),
		(prAdapter->rVerInfo.u2FwOwnVersion & BITS(0, 7)),
		(prAdapter->rVerInfo.u2FwOwnVersion >> 8),
		(prAdapter->rVerInfo.u2FwOwnVersion & BITS(0, 7)),
		(prAdapter->rVerInfo.u2FwPeerVersion >> 8),
		(prAdapter->rVerInfo.u2FwPeerVersion & BITS(0, 7))));

	if (kalIsResetting()) {
		DBGLOG(INIT, ERROR, ("Skip triggering whole-chip reset during reseting!\n"));
		fgResult = TRUE;
	} else {
		fgResult = mtk_wcn_wmt_do_reset(WMTDRV_TYPE_WIFI);
	}
#endif

	return fgResult;
}

#endif				/* CFG_CHIP_RESET_SUPPORT */
