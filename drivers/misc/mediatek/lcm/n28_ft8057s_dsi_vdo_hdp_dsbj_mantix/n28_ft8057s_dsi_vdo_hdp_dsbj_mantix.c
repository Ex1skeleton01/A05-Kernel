
// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
#define LOG_TAG "LCM"
#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"
#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include "disp_dts_gpio.h"
#endif
#ifndef MACH_FPGA
#include <lcm_pmic.h>
#endif
#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

extern char *saved_command_line;

static struct LCM_UTIL_FUNCS lcm_util;
#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))
#define MDELAY(n)		(lcm_util.mdelay(n))
#define UDELAY(n)		(lcm_util.udelay(n))
#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	  lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif
#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH										(720)
#define FRAME_HEIGHT									(1600)
//+S96818AA1-1936,liyuhong1.wt,modify,2023/06/14,nt36528 modify the physical size of the screen
#define LCM_PHYSICAL_WIDTH								(70380)
#define LCM_PHYSICAL_HEIGHT								(156240)
//-S96818AA1-1936,liyuhong1.wt,modify,2023/06/14,nt36528 modify the physical size of the screen
#define REGFLAG_DELAY			0xFFFC
#define REGFLAG_UDELAY			0xFFFB
#define REGFLAG_END_OF_TABLE	0xFFFD
#define REGFLAG_RESET_LOW		0xFFFE
#define REGFLAG_RESET_HIGH		0xFFFF
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};
static struct LCM_setting_table lcm_suspend_setting[] = {

	{0x28, 0,{0x00}},
	{REGFLAG_DELAY, 20,{}},
	{0x10, 0,{0x00}},
	//+S96818AA1-1936,liyuhong1.wt,modify,2023/06/06,reduce sleep power consumption
	{REGFLAG_DELAY, 140,{}},
	{0x00, 1,{0x00}},
	{0xF7, 4,{0x5A,0xA5,0x95,0x27}},
	{REGFLAG_DELAY, 3,{}},
	//-S96818AA1-1936,liyuhong1.wt,modify,2023/06/06,reduce sleep power consumption
};
static struct LCM_setting_table init_setting_vdo[] = {
	{0x00, 1, {0x00}},
	{0xFF, 3, {0x80, 0x57, 0x01}},
	{0x00, 1, {0x80}},
	{0xFF, 2, {0x80, 0x57}},
	{0x00, 1, {0xA3}},
	{0xB3, 4, {0x06, 0x40, 0x00, 0x18}},
	{0x00, 1, {0x93}},
	{0xC5, 1, {0x6B}},
	{0x00, 1, {0x97}},
	{0xC5, 1, {0x6B}},
	{0x00, 1, {0x9A}},
	{0xC5, 1, {0x41}},
	{0x00, 1, {0x9C}},
	{0xC5, 1, {0x41}},
	{0x00, 1, {0xB6}},
	{0xC5, 2, {0x57, 0x57}},
	{0x00, 1, {0xB8}},
	{0xC5, 2, {0x37, 0x37}},
	{0x00, 1, {0x00}},
	{0xD8, 2, {0x2B, 0x2B}},
	{0x00, 1, {0x82}},
	{0xC5, 1, {0x55}},
	{0x00, 1, {0x83}},
	{0xC5, 1, {0x07}},
	{0x00, 1, {0x96}},
	{0xF5, 1, {0x0D}},
	{0x00, 1, {0x86}},
	{0xF5, 1, {0x0D}},
	{0x00, 1, {0x94}},
	{0xC5, 1, {0x15}},
	{0x00, 1, {0x9B}},
	{0xC5, 1, {0x51}},
	{0x00, 1, {0xA3}},
	{0xA5, 1, {0x04}},
	{0x00, 1, {0x99}},
	{0xCF, 1, {0x56}},
	{0x00, 1, {0x00}},
	{0xE1, 16 ,{0x2D, 0x31, 0x3C, 0x48, 0x50, 0x58, 0x65, 0x71, 0x6F, 0x7A, 0x7A, 0x8B, 0x7B, 0x6A, 0x6D, 0x61}},
	{0x00, 1, {0x10}},
	{0xE1, 8, {0x5A, 0x4F, 0x3F, 0x35, 0x2D, 0x1C, 0x10, 0x0F}},
	{0x00, 1, {0x00}},
	{0xE2, 16 ,{0x2D, 0x31, 0x3C, 0x48, 0x50, 0x58, 0x65, 0x71, 0x6F, 0x7A, 0x7A, 0x8B, 0x7B, 0x6A, 0x6D, 0x61}},
	{0x00, 1, {0x10}},
	{0xE2, 8, {0x5A, 0x4F, 0x3F, 0x35, 0x2D, 0x1C, 0x10, 0x0F}},
	{0x00, 1, {0x80}},
	{0xC0, 6, {0x00, 0xD2, 0x00, 0x3A, 0x00, 0x10}},
	{0x00, 1, {0x90}},
	{0xC0, 6, {0x00, 0xEC, 0x00, 0x3A, 0x00, 0x10}},
	{0x00, 1, {0xA0}},
	{0xC0, 6, {0x00, 0xD2, 0x00, 0x3A, 0x00, 0x10}},
	{0x00, 1, {0xB0}},
	{0xC0, 5, {0x01, 0x11, 0x00, 0x3A, 0x10}},
	{0x00, 1, {0xC1}},
	{0xC0, 8, {0x01, 0x33, 0x01, 0x0A, 0x00, 0xCD, 0x01, 0x90}},
	{0x00, 1, {0x70}},
	{0xC0, 6, {0x00, 0xEC, 0x00, 0x3A, 0x00, 0x10}},
	{0x00, 1, {0xA3}},
	{0xC1, 6, {0x00, 0x33, 0x00, 0x3C, 0x00, 0x02}},
	{0x00, 1, {0xB7}},
	{0xC1, 2, {0x00, 0x33}},
	{0x00, 1, {0x73}},
	{0xCE, 2, {0x09, 0x09}},
	{0x00, 1, {0x80}},
	{0xCE, 16 ,{0x01, 0x81, 0x09, 0x09, 0x00, 0x78, 0x00, 0x96, 0x00, 0x78, 0x00, 0x96, 0x00, 0x78, 0x00, 0x96}},
	{0x00, 1, {0x90}},
	{0xCE, 15 ,{0x00, 0xA5, 0x16, 0x8F, 0x00, 0xA5, 0x80, 0x09, 0x09, 0x00, 0x07, 0xD0, 0x16, 0x16, 0x27}},
	{0x00, 1, {0xA0}},
	{0xCE, 3, {0x20, 0x00, 0x00}},
	{0x00, 1, {0xB0}},
	{0xCE, 3, {0x87, 0x00, 0x00}},
	{0x00, 1, {0xD1}},
	{0xCE, 7, {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xE1}},
	{0xCE, 11 ,{0x08, 0x03, 0xC3, 0x03, 0xC3, 0x02, 0xB0, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xF1}},
	{0xCE, 9, {0x14, 0x14, 0x1E, 0x01, 0x45, 0x01, 0x45, 0x01, 0x2B}},
	{0x00, 1, {0xB0}},
	{0xCF, 4, {0x00, 0x00, 0x6D, 0x71}},
	{0x00, 1, {0xB5}},
	{0xCF, 4, {0x03, 0x03, 0x5B, 0x5F}},
	{0x00, 1, {0xC0}},
	{0xCF, 4, {0x06, 0x06, 0x3B, 0x3F}},
	{0x00, 1, {0xC5}},
	{0xCF, 4, {0x06, 0x06, 0x3F, 0x43}},
	{0x00, 1, {0x60}},
	{0xCF, 8, {0x00, 0x00, 0x6D, 0x71, 0x03, 0x03, 0x5B, 0x5F}},
	{0x00, 1, {0x70}},
	{0xCF, 8, {0x00, 0x00, 0x6F, 0x73, 0x03, 0x03, 0x5D, 0x61}},
	{0x00, 1, {0xAA}},
	{0xCF, 4, {0x80, 0x80, 0x10, 0x0C}},
	{0x00, 1, {0xD1}},
	{0xC1, 12 ,{0x03, 0xAA, 0x05, 0x22, 0x09, 0x59, 0x05, 0x87, 0x08, 0x23, 0x0F, 0xAC}},
	{0x00, 1, {0xE1}},
	{0xC1, 2, {0x05, 0x22}},
	{0x00, 1, {0xE2}},
	{0xCF, 12 ,{0x06, 0xDE, 0x06, 0xDD, 0x06, 0xDD, 0x06, 0xDD, 0x06, 0xDD, 0x06, 0xDD}},
	{0x00, 1, {0x80}},
	{0xC1, 2, {0x00, 0x00}},
	{0x00, 1, {0x90}},
	{0xC1, 1, {0x01}},
	{0x00, 1, {0xF5}},
	{0xCF, 1, {0x01}},
	{0x00, 1, {0xF6}},
	{0xCF, 1, {0x3C}},
	{0x00, 1, {0xF1}},
	{0xCF, 1, {0x3C}},
	{0x00, 1, {0xF7}},
	{0xCF, 1, {0x11}},
	{0x00, 1, {0x00}},
	{0x1F, 2, {0x3C, 0x3C}},
	{0x00, 1, {0xD1}},
	{0xCE, 7, {0x00, 0x13, 0x01, 0x01, 0x00, 0xA3, 0x01}},
	{0x00, 1, {0xE8}},
	{0xCE, 4, {0x00, 0xA3, 0x00, 0xA3}},
	{0x00, 1, {0x80}},
	{0xCC, 16 ,{0x00, 0x16, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1E, 0x26, 0x1A, 0x14, 0x12, 0x10, 0x0E}},
	{0x00, 1, {0x90}},
	{0xCC, 8, {0x0C, 0x0A, 0x08, 0x06, 0x04, 0x02, 0x00, 0x00}},
	{0x00, 1, {0x80}},
	{0xCD, 16 ,{0x00, 0x17, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1E, 0x26, 0x1B, 0x15, 0x13, 0x11, 0x0F}},
	{0x00, 1, {0x90}},
	{0xCD, 8, {0x0D, 0x0B, 0x09, 0x07, 0x05, 0x03, 0x00, 0x00}},
	{0x00, 1, {0x80}},
	{0xCB, 16 ,{0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1}},
	{0x00, 1, {0xED}},
	{0xCB, 1, {0xC1}},
	{0x00, 1, {0x90}},
	{0xCB, 16 ,{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xEE}},
	{0xCB, 1, {0x00}},
	{0x00, 1, {0x90}},
	{0xC3, 1, {0x00}},
	{0x00, 1, {0xA0}},
	{0xCB, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xB0}},
	{0xCB, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xC0}},
	{0xCB, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x00, 1, {0xD2}},
	{0xCB, 11 ,{0x83, 0x00, 0x83, 0x83, 0x00, 0x83, 0x83, 0x00, 0x83, 0x83, 0x00}},
	{0x00, 1, {0xE0}},
	{0xCB, 13 ,{0x83, 0x83, 0x00, 0x83, 0x83, 0x00, 0x83, 0x83, 0x00, 0x83, 0x83, 0x00, 0x83}},
	{0x00, 1, {0xFA}},
	{0xCB, 2, {0x83, 0x00}},
	{0x00, 1, {0xEF}},
	{0xCB, 1, {0x00}},
	{0x00, 1, {0x68}},
	{0xC2, 4, {0x8A, 0x09, 0x68, 0xB6}},
	{0x00, 1, {0x6C}},
	{0xC2, 4, {0x89, 0x09, 0x68, 0xB6}},
	{0x00, 1, {0x70}},
	{0xC2, 4, {0x86, 0x09, 0x68, 0xB6}},
	{0x00, 1, {0x74}},
	{0xC2, 4, {0x85, 0x09, 0x68, 0xB6}},
	{0x00, 1, {0x78}},
	{0xC2, 4, {0x06, 0x04, 0x68, 0xB6}},
	{0x00, 1, {0x7C}},
	{0xC2, 4, {0x07, 0x04, 0x68, 0xB6}},
	{0x00, 1, {0x80}},
	{0xC2, 4, {0x08, 0x04, 0x68, 0xB6}},
	{0x00, 1, {0x84}},
	{0xC2, 4, {0x09, 0x04, 0x68, 0xB6}},
	{0x00, 1, {0x88}},
	{0xC2, 4, {0x0D, 0x09, 0x68, 0xB6}},
	{0x00, 1, {0xE4}},
	{0xC2, 4, {0x0E, 0x09, 0x68, 0xB6}},
	{0x00, 1, {0x8C}},
	{0xC2, 5, {0x81, 0x02, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0x91}},
	{0xC2, 5, {0x80, 0x03, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0x96}},
	{0xC2, 5, {0x01, 0x04, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0x9B}},
	{0xC2, 5, {0x02, 0x05, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xA0}},
	{0xC2, 5, {0x03, 0x06, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xA5}},
	{0xC2, 5, {0x04, 0x07, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xAA}},
	{0xC2, 5, {0x05, 0x08, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xAF}},
	{0xC2, 5, {0x06, 0x09, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xB4}},
	{0xC2, 5, {0x07, 0x0A, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xB9}},
	{0xC2, 5, {0x08, 0x0B, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xBE}},
	{0xC2, 5, {0x09, 0x0C, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xC3}},
	{0xC2, 5, {0x0A, 0x0D, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xC8}},
	{0xC2, 5, {0x0B, 0x0E, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xCD}},
	{0xC2, 5, {0x0C, 0x0F, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xD2}},
	{0xC2, 5, {0x0D, 0x10, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xD7}},
	{0xC2, 5, {0x0E, 0x11, 0x02, 0x68, 0xC6}},
	{0x00, 1, {0xDC}},
	{0xC2, 8, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
	{0x00, 1, {0x98}},
	{0xC4, 1, {0x08}},
	{0x00, 1, {0x91}},
	{0xE9, 4, {0xFF, 0xFF, 0xFF, 0x00}},
	{0x00, 1, {0x85}},
	{0xC4, 1, {0x80}},
	{0x00, 1, {0x81}},
	{0xA4, 1, {0x73}},
	{0x00, 1, {0x86}},
	{0xA4, 1, {0xB6}},
	{0x00, 1, {0x95}},
	{0xC4, 1, {0x80}},
	{0x00, 1, {0xCA}},
	{0xC0, 2, {0x90, 0x00}},
	{0x00, 1, {0xB7}},
	{0xF5, 1, {0x1D}},
	{0x00, 1, {0xB1}},
	{0xF5, 1, {0x1B}},
	{0x00, 1, {0x83}},
	{0xF5, 1, {0x11}},
	{0x00, 1, {0x94}},
	{0xF5, 1, {0x11}},
	{0x00, 1, {0xB0}},
	{0xC5, 1, {0x00}},
	{0x00, 1, {0xB3}},
	{0xC5, 1, {0x00}},
	{0x00, 1, {0xB2}},
	{0xC5, 1, {0x0D}},
	{0x00, 1, {0xB5}},
	{0xC5, 1, {0x02}},
	{0x00, 1, {0xC2}},
	{0xF5, 1, {0x42}},
	{0x00, 1, {0x80}},
	{0xCE, 1, {0x00}},
	{0x00, 1, {0xD0}},
	{0xCE, 1, {0x01}},
	{0x00, 1, {0xE0}},
	{0xCE, 1, {0x00}},
	{0x00, 1, {0xA1}},
	{0xC1, 1, {0xCC}},
	{0x00, 1, {0xA6}},
	{0xC1, 1, {0x10}},
	{0x00, 1, {0x71}},
	{0xC0, 5, {0xEC, 0x01, 0x2B, 0x00, 0x22}},
	{0x00, 1, {0x86}},
	{0xB7, 1, {0x80}},
	{0x00, 1, {0xA5}},
	{0xB0, 1, {0x1D}},
	{0x00, 1, {0xB0}},
	{0xCA, 6, {0x00,0x00,0x0C,0x00,0x00,0x06}},
	{0x00, 1, {0x00}},
	{0xFF, 3, {0x00, 0x00, 0x00}},
	{0x00, 1, {0x80}},
	{0xFF, 2, {0x00, 0x00}},
	{0x51, 2, {0x00, 0x00}},//CABC 12bit
	{0x53, 1, {0x24}},//close dimming
	{0x55, 1, {0x00}},
	{0x11, 0, {}},
	{REGFLAG_DELAY,100, {}},
	{0x29, 0, {}},
	{0x35, 1, {0x00}},
	{REGFLAG_DELAY,10, {}},
};

static void push_table(void *cmdq, struct LCM_setting_table *table,
	unsigned int count, unsigned char force_update)
{
	unsigned int i;
	unsigned cmd;
	for (i = 0; i < count; i++) {
		cmd = table[i].cmd;
		switch (cmd) {
		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;
		case REGFLAG_UDELAY:
			UDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V22(cmdq, cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}
static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}
static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));
	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;
#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
#endif
	params->dsi.switch_mode_enable = 0;
	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */
	//+S96818AA1-1936,liyuhong1.wt,modify,2023/07/05,adjust mipi 660Mhz
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.vertical_sync_active = 6;
	params->dsi.vertical_backporch = 28;
	params->dsi.vertical_frontporch = 300;
	//params->dsi.vertical_frontporch_for_low_power = 540;/*disable dynamic frame rate*/
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	//+S96818AA1-1936,liuzhizun2.wt,modify,2023/05/06,adjust frame rate 60hz
	params->dsi.horizontal_sync_active = 4;
	params->dsi.horizontal_backporch = 14;
	params->dsi.horizontal_frontporch = 156;
	//-S96818AA1-1936,liuzhizun2.wt,modify,2023/05/06,adjust frame rate 60hz
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.ssc_range = 4;
	params->dsi.ssc_disable = 1;
#ifndef CONFIG_FPGA_EARLY_PORTING
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 282;	/* this value must be in MTK suggested table */
#else
	params->dsi.PLL_CLOCK = 330;	/* this value must be in MTK suggested table */
	params->dsi.data_rate = 660;
#endif
	//-S96818AA1-1936,liyuhong1.wt,modify,2023/07/05,adjust mipi 660Mhz
	//params->dsi.PLL_CK_CMD = 220;
	//params->dsi.PLL_CK_VDO = 255;
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	/* mipi hopping setting params */
	params->dsi.dynamic_switch_mipi = 0;    /* turn off frequency hopping */
	params->dsi.data_rate_dyn = 610;
	params->dsi.PLL_CLOCK_dyn = 305;
	params->dsi.horizontal_sync_active_dyn = 4;
	params->dsi.horizontal_backporch_dyn = 84;
	params->dsi.horizontal_frontporch_dyn  = 88;

	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 0;
	params->corner_pattern_width = 720;
	params->corner_pattern_height = 32;
#endif
}
enum Color {
	LOW,
	HIGH
};
static void lcm_init_power(void)
{
	int ret = 0;
	pr_err("[LCM]ft8057s_dsbj_mantix lcm_init_power !\n");
    lcm_reset_pin(LOW);
	//+S96818AA1-1936,liyuhong1.wt,modify,2023/06/07,modify power-on timing
	MDELAY(5);
	//ret = lcm_power_enable();
	lcm_bais_enp_enable(HIGH);
  	MDELAY(3);
  	lcm_bais_enn_enable(HIGH);
	MDELAY(2);
	ret = lcm_set_power_reg(0X00,0x0F,(0x1F << 0));
	ret = lcm_set_power_reg(0X01,0x0F,(0x1F << 0));
	ret = lcm_set_power_reg(0X03,((1<<0) | (1<<1)),((1<<0) | (1<<1)));
	//-S96818AA1-1936,liyuhong1.wt,modify,2023/06/07,modify power-on timing
    MDELAY(11);
}
//+S96818AA1-1936,daijun1.wt,modify,2023/05/06,nt36528 tp add gesture wake-up func
//+S96818AA1-1936,wangtao14.wt,modify,2023/05/17,ft8057s tp gesture wake-up func
extern bool fts_gestrue_status;
static void lcm_suspend_power(void)
{
    if (fts_gestrue_status == 1) {
    pr_err("[LCM] ft8057s_dsbj_mantix lcm_suspend_power fts_gestrue_status = 1!\n");
    } else {
        int ret  = 0;
        pr_err("[LCM] ft8057s_dsbj_mantix lcm_suspend_power fts_gestrue_status = 0!\n");
        //lcm_reset_pin(LOW);
        MDELAY(2);
        ret = lcm_power_disable();
        MDELAY(10);
    }
}
//-S96818AA1-1936,daijun1.wt,modify,2023/05/06,nt36528 tp add gesture wake-up func
//-S96818AA1-1936,wangtao14.wt,modify,2023/05/17,ft8057s tp gesture wake-up func
static void lcm_resume_power(void)
{
	pr_err("[LCM]ft8057s_dsbj_mantix lcm_resume_power !\n");
	lcm_init_power();
}
static int last_brightness = 0;
static int dimming_state = 0;
static void lcm_init(void)
{
	pr_err("[LCM] ft8057s_dsbj_mantix lcm_init\n");
	lcm_reset_pin(HIGH);
	MDELAY(5);
	lcm_reset_pin(LOW);
	MDELAY(5);
	lcm_reset_pin(HIGH);
	MDELAY(10);
	MDELAY(1);
	push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
	pr_err("[LCM] ft8057s_dsbj_mantix----init success ----\n");
	dimming_state = 0;
}
static void lcm_suspend(void)
{
	pr_err("[LCM] lcm_suspend\n");
	push_table(NULL, lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
}
static void lcm_resume(void)
{
	pr_err("[LCM] lcm_resume\n");
	lcm_init();
}
static unsigned int lcm_compare_id(void)
{
	return 1;
}
static unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	unsigned int ret = 0;
	unsigned int x0 = FRAME_WIDTH / 4;
	unsigned int x1 = FRAME_WIDTH * 3 / 4;
	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned int data_array[3];
	unsigned char read_buf[4];
	pr_err("ATA check size = 0x%x,0x%x,0x%x,0x%x\n", x0_MSB, x0_LSB, x1_MSB, x1_LSB);
	data_array[0] = 0x0005390A;	/* HS packet */
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	data_array[0] = 0x00043700;	/* read id return two byte,version and id */
	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0x2A, read_buf, 4);
	if ((read_buf[0] == x0_MSB) && (read_buf[1] == x0_LSB)
	    && (read_buf[2] == x1_MSB) && (read_buf[3] == x1_LSB))
		ret = 1;
	else
		ret = 0;
	x0 = 0;
	x1 = FRAME_WIDTH - 1;
	x0_MSB = ((x0 >> 8) & 0xFF);
	x0_LSB = (x0 & 0xFF);
	x1_MSB = ((x1 >> 8) & 0xFF);
	x1_LSB = (x1 & 0xFF);
	data_array[0] = 0x0005390A;	/* HS packet */
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	return ret;
#else
	return 0;
#endif
}

static struct LCM_setting_table bl_level[] = {
	{ 0x51, 0x02, {0xFF,0x0F} },
};

static struct LCM_setting_table dimming[] = {
	{ 0x53, 0x01, {0x2C} },
};

static void lcm_setbacklight_cmdq(void* handle,unsigned int level)
{
	unsigned int bl_lvl;
	//-S96818AA1-1936,liyuhong1.wt,modify,2023/07/19,lcd adjust dimming func
	if((dimming_state == 0) && (last_brightness !=0))
	{
		push_table(handle, dimming, sizeof(dimming) / sizeof(struct LCM_setting_table), 1);
		MDELAY(20);
		dimming_state = 1;
	}
	//-S96818AA1-1936,liyuhong1.wt,modify,2023/07/19,lcd adjust dimming func
	bl_lvl = wingtech_bright_to_bl(level,255,10,4095,48);
	pr_err("%s,ft8057s_dsbj_mantix backlight: level = %d,bl_lvl=%d\n", __func__, level,bl_lvl);
	// set 12bit
	bl_level[0].para_list[0] = (bl_lvl&0xFF0)>>4;
	bl_level[0].para_list[1] = (bl_lvl&0xF);
	pr_err("ft8057s_dsbj_mantix backlight: para_list[0]=%x,para_list[1]=%x\n",bl_level[0].para_list[0],bl_level[0].para_list[1]);
	push_table(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
	last_brightness = bl_lvl;
}
struct LCM_DRIVER n28_ft8057s_dsi_vdo_hdp_dsbj_mantix_drv = {
	.name = "n28_ft8057s_dsi_vdo_hdp_dsbj_mantix",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
	.ata_check = lcm_ata_check,
};