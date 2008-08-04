/*
 * HP IA-64 LBA driver
 *
 * Copyright (C) 2007 Hewlett-Packard Co
 * Copyright (C) 2007 Andrew Patterson <andrew.patterson@hp.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _APOLLO_H_
#define _APOLLO_H_

#define APOLLO_FUNC_ID 0x4037

#define APOLLO_MAJOR_REVISION 0x0008
#define  APOLLO_MAJOR_REVISION_SHIFT 4
#define  APOLLO_MAJOR_REVISION_MASK 0xf
#define  APOLLO_MAJOR_REVISION_SIZE 64
#define APOLLO_MINOR_REVISION APOLLO_MAJOR_REVISION
#define  APOLLO_MINOR_REVISION_SHIFT 0
#define  APOLLO_MINOR_REVISION_MASK 0xf
#define  APOLLO_MINOR_REVISION_SIZE 64

#define APOLLO_ROPE_CFG 0x0610
#define APOLLO_ROPE_2X_L APOLLO_ROPE_CFG
#define  APOLLO_ROPE_2X_L_SHIFT 8
#define  APOLLO_ROPE_2X_L_MASK 0x1
#define  APOLLO_ROPE_2X_L_SIZE 64
#define APOLLO_ROPE_4X_L APOLLO_ROPE_CFG
#define  APOLLO_ROPE_4X_L_SHIFT 9
#define  APOLLO_ROPE_4X_L_MASK 0x1
#define  APOLLO_ROPE_4X_L_SIZE 64

#define APOLLO_MAX_LINK_WIDTH 0x0670
#define  APOLLO_MAX_LINK_WIDTH_SHIFT 24
#define  APOLLO_MAX_LINK_WIDTH_MASK 0x3
#define  APOLLO_MAX_LINK_WIDTH_SIZE 64

#define APOLLO_DIAG_EN0 0x0500
#define  APOLLO_DIAG_EN0_SHIFT 0
#define  APOLLO_DIAG_EN0_MASK 0xf
#define  APOLLO_DIAG_EN0_SIZE 64

#define APOLLO_CTRL0A_ALWAYS_CNT 0x0508
#define  APOLLO_CTRL0A_ALWAYS_CNT_SHIFT 0
#define  APOLLO_CTRL0A_ALWAYS_CNT_MASK 0x1
#define  APOLLO_CTRL0A_ALWAYS_CNT_SIZE 64

#define APOLLO_DIAG_CTRL1 0x0538
#define  APOLLO_DIAG_CTRL1_SHIFT 33
#define  APOLLO_DIAG_CTRL1_MASK 0x3
#define  APOLLO_DIAG_CTRL1_SIZE 64

#define APOLLO_DIAG_CTRL0_TIMER 0x0550
#define  APOLLO_DIAG_CTRL0_TIMER_SHIFT 0
#define  APOLLO_DIAG_CTRL0_TIMER_MASK 0xffffffffffff
#define  APOLLO_DIAG_CTRL0_TIMER_SIZE 64

#define APOLLO_DIAG_CTRL1_CNTR1 0x0558
#define  APOLLO_DIAG_CTRL1_CNTR1_SHIFT 0
#define  APOLLO_DIAG_CTRL1_CNTR1_MASK 0xffffffffffff
#define  APOLLO_DIAG_CTRL1_CNTR1_SIZE 64

extern struct lba_ops apollo_ops;

#endif // _APOLLO_H_ //
