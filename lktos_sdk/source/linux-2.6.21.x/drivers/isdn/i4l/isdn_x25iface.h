/* $Id: //WIFI_SOC/release/SDK_4_1_0_0/source/linux-2.6.21.x/drivers/isdn/i4l/isdn_x25iface.h#1 $
 *
 * header for Linux ISDN subsystem, x.25 related functions
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */

#ifndef _LINUX_ISDN_X25IFACE_H
#define _LINUX_ISDN_X25IFACE_H

#define ISDN_X25IFACE_MAGIC 0x1e75a2b9
/* #define DEBUG_ISDN_X25 if you want isdn_x25 debugging messages */
#ifdef DEBUG_ISDN_X25
#   define IX25DEBUG(fmt,args...) printk(KERN_DEBUG fmt , ## args)
#else
#   define IX25DEBUG(fmt,args...)
#endif

#include <linux/skbuff.h>
#include <linux/wanrouter.h>
#include <linux/isdn.h>
#include <linux/concap.h>

extern struct concap_proto_ops * isdn_x25iface_concap_proto_ops_pt;
extern struct concap_proto     * isdn_x25iface_proto_new(void);



#endif







