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

#ifndef _MERCURY_H_
#define _MERCURY_H_

#define MERCURY_FUNC_ID 0x122e

#define MERCURY_MAJOR_REVISION 0x0008
#define  MERCURY_MAJOR_REVISION_SHIFT 4
#define  MERCURY_MAJOR_REVISION_MASK 0xf
#define  MERCURY_MAJOR_REVISION_SIZE 64
#define MERCURY_MINOR_REVISION MERCURY_MAJOR_REVISION
#define  MERCURY_MINOR_REVISION_SHIFT 0
#define  MERCURY_MINOR_REVISION_MASK 0xf
#define  MERCURY_MINOR_REVISION_SIZE 64

#define MERCURY_ROPE_CFG 0x0610
#define MERCURY_ROPE_2X_L MERCURY_ROPE_CFG
#define  MERCURY_ROPE_2X_L_SHIFT 8
#define  MERCURY_ROPE_2X_L_MASK 0x1
#define  MERCURY_ROPE_2X_L_SIZE 64

#define MERCURY_AGP_BUS_MODE 0x0620
#define  MERCURY_AGP_BUS_MODE_SHIFT 0
#define  MERCURY_AGP_BUS_MODE_MASK 0x1
#define  MERCURY_AGP_BUS_MODE_SIZE 64
#define MERCURY_BUS_OP_MODE 0x0620
#define  MERCURY_BUS_OP_MODE_SHIFT 13
#define  MERCURY_BUS_OP_MODE_MASK 0x3
#define  MERCURY_BUS_OP_MODE_SIZE 64
#define MERCURY_AGP_DATA_RATE 0x0068
#define  MERCURY_AGP_DATA_RATE_SHIFT 0
#define  MERCURY_AGP_DATA_RATE_MASK 0x7
#define  MERCURY_AGP_DATA_RATE_SIZE 64


#define MERCURY_DIAG_EN0 0x0500
#define  MERCURY_DIAG_EN0_SHIFT 0
#define  MERCURY_DIAG_EN0_MASK 0xf
#define  MERCURY_DIAG_EN0_SIZE 64

#define MERCURY_CTRL0A_ALWAYS_CNT 0x0508
#define  MERCURY_CTRL0A_ALWAYS_CNT_SHIFT 0
#define  MERCURY_CTRL0A_ALWAYS_CNT_MASK 0x1
#define  MERCURY_CTRL0A_ALWAYS_CNT_SIZE 64

#define MERCURY_DIAG_CTRL1 0x0510
#define  MERCURY_DIAG_CTRL1_SHIFT 0
#define  MERCURY_DIAG_CTRL1_MASK -1
#define  MERCURY_DIAG_CTRL1_SIZE 64

#define MERCURY_DIAG_CTRL0_TIMER 0x0518
#define  MERCURY_DIAG_CTRL0_TIMER_SHIFT 0
#define  MERCURY_DIAG_CTRL0_TIMER_MASK 0xffffffffffff
#define  MERCURY_DIAG_CTRL0_TIMER_SIZE 64

#define MERCURY_DIAG_CTRL1_CNTR1 0x0520
#define  MERCURY_DIAG_CTRL1_CNTR1_SHIFT 0
#define  MERCURY_DIAG_CTRL1_CNTR1_MASK 0xffffffffffff
#define  MERCURY_DIAG_CTRL1_CNTR1_SIZE 64

extern struct lba_ops mercury_ops;

#endif // _MERCURY_H_ //
