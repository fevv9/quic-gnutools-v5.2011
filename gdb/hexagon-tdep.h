/* Target-dependent code for QUALCOMM HEXAGON GDB, the GNU Debugger.
   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#if !defined (HEXAGON_TDEP_H)
#define HEXAGON_TDEP_H 1


/* Enable printing of register names */
typedef struct 
{
  char* reg_name; /* reg name */
  int   index;    /* Offset in reg file */
} hexagon_regtype_t;

/*
   - data structure: hexagon_system_register_offsets
   - description:
	-- This is used to provide a mapping to registers between different
  	   HEXAGON architecture revisions.
	-- TODO: more V4 registers to be added.
 */
struct hexagon_system_register_offsets
{
    int reg_badva;
    int reg_brkptcfg0;
    int reg_brkptcfg1;
    int reg_brkptinfo;
    int reg_brkptpc0;
    int reg_brkptpc1;
    int reg_diag;
    int reg_elr;
    int reg_evb;
    int reg_fp;
    int reg_gp;
    int reg_iad;
    int reg_iahl;
    int reg_iel;
    int reg_imask;
    int reg_ipend;
    int reg_isdbcfg0;
    int reg_isdbcfg1;
    int reg_isdbcmd;
    int reg_isdben;
    int reg_isdbgpr;
    int reg_isdbmbxin;
    int reg_isdbmbxout;
    int reg_isdbst;
    int reg_isdbver;
    int reg_lc0;
    int reg_lc1;
    int reg_lr;
    int reg_m0;
    int reg_m1;
    int reg_modectl;
    int reg_p3_0;
    int reg_pc;
    int reg_pcyclehi;
    int reg_pcyclelo;
    int reg_r00;
    int reg_r01;
    int reg_r02;
    int reg_r03;
    int reg_r04;
    int reg_r05;
    int reg_r06;
    int reg_r07;
    int reg_r08;
    int reg_r09;
    int reg_r10;
    int reg_r11;
    int reg_r12;
    int reg_r13;
    int reg_r14;
    int reg_r15;
    int reg_r16;
    int reg_r17;
    int reg_r18;
    int reg_r19;
    int reg_r20;
    int reg_r21;
    int reg_r22;
    int reg_r23;
    int reg_r24;
    int reg_r25;
    int reg_r26;
    int reg_r27;
    int reg_r28;
    int reg_r29;
    int reg_r30;
    int reg_r31;
    int reg_rev;
    int reg_sa0;
    int reg_sa1;
    int reg_sgp;
    int reg_sp;
    int reg_sr;
    int reg_ssr;
    int reg_stfinst;
    int reg_sysconf;
    int reg_syscfg;
    int reg_tid;   /* If used in v4 mode then it should point to reg_stid */
    int reg_tlbhi;
    int reg_tlbidx;
    int reg_tlblo;
    int reg_ugp;
    int reg_usr;
/* V4 specific */
    int reg_acc0;
    int reg_acc1;
    int reg_avs;
    int reg_badva0; /* User mode same as reg_badva on v2/3 */
    int reg_badva1; /* Monitor mode v4 only */
    int reg_ccr;
    int reg_cfgbase;
    int reg_chicken;
    int reg_cs0;
    int reg_cs1;
    int reg_g0;
    int reg_g1;
    int reg_g2;
    int reg_g3;
    int reg_htid;   /* v2/v3 id was store as part of reg_ssr */
    int reg_pmucfg;
    int reg_pmucnt0;
    int reg_pmucnt1;
    int reg_pmucnt2;
    int reg_pmucnt3;
    int reg_pmuevtcfg;
    int reg_rgdr;
    int reg_sgp0;   /* User mode same as reg_resgp on v2/3 */
    int reg_sgp1;   /* Monitor mode v4 only */
    int reg_stid;   /* Re-named TID register */
    int reg_upcyclehi;
    int reg_upcyclelo;
    int reg_upmucnt0;
    int reg_upmucnt1;
    int reg_upmucnt2;
    int reg_upmucnt3;
    int reg_vid;
};


#define MAX_PACKET 4
struct packet {
	unsigned int insn[MAX_PACKET];
	CORE_ADDR addr[MAX_PACKET];
	unsigned int count;
};


#include "reg_offsets.h"
static struct hexagon_system_register_offsets hexagon_reg_offset =
{
    reg_badva: REG_BADVA,
    reg_brkptcfg0: REG_BRKPTCFG0,
    reg_brkptcfg1: REG_BRKPTCFG1,
    reg_brkptinfo: REG_BRKPTINFO,
    reg_brkptpc0: REG_BRKPTPC0,
    reg_brkptpc1: REG_BRKPTPC1,
    reg_diag: REG_DIAG,
    reg_elr: REG_ELR,
    reg_evb: REG_EVB,
    reg_fp: REG_FP,
    reg_gp: REG_GP,
    reg_iad: REG_IAD,
    reg_iahl: REG_IAHL,
    reg_iel: REG_IEL,
    reg_imask: REG_IMASK,
    reg_ipend: REG_IPEND,
    reg_isdbcfg0: REG_ISDBCFG0,
    reg_isdbcfg1: REG_ISDBCFG1,
    reg_isdbcmd: REG_ISDBCMD,
    reg_isdben: REG_ISDBEN,
    reg_isdbgpr: REG_ISDBGPR,
    reg_isdbmbxin: REG_ISDBMBXIN,
    reg_isdbmbxout: REG_ISDBMBXOUT,
    reg_isdbst: REG_ISDBST,
    reg_isdbver: REG_ISDBVER,
    reg_lc0: REG_LC0,
    reg_lc1: REG_LC1,
    reg_lr: REG_LR,
    reg_m0: REG_M0,
    reg_m1: REG_M1,
    reg_modectl: REG_MODECTL,
    reg_p3_0: REG_P3_0,
    reg_pc: REG_PC,
    reg_pcyclehi: REG_PCYCLEHI,
    reg_pcyclelo: REG_PCYCLELO,
    reg_r00: REG_R00,
    reg_r01: REG_R01,
    reg_r02: REG_R02,
    reg_r03: REG_R03,
    reg_r04: REG_R04,
    reg_r05: REG_R05,
    reg_r06: REG_R06,
    reg_r07: REG_R07,
    reg_r08: REG_R08,
    reg_r09: REG_R09,
    reg_r10: REG_R10,
    reg_r11: REG_R11,
    reg_r12: REG_R12,
    reg_r13: REG_R13,
    reg_r14: REG_R14,
    reg_r15: REG_R15,
    reg_r16: REG_R16,
    reg_r17: REG_R17,
    reg_r18: REG_R18,
    reg_r19: REG_R19,
    reg_r20: REG_R20,
    reg_r21: REG_R21,
    reg_r22: REG_R22,
    reg_r23: REG_R23,
    reg_r24: REG_R24,
    reg_r25: REG_R25,
    reg_r26: REG_R26,
    reg_r27: REG_R27,
    reg_r28: REG_R28,
    reg_r29: REG_R29,
    reg_r30: REG_R30,
    reg_r31: REG_R31,
    reg_rev: REG_REV,
    reg_sa0: REG_SA0,
    reg_sa1: REG_SA1,
    reg_sgp: REG_SGP,
    reg_sp: REG_SP,
    reg_sr: REG_SR,
    reg_ssr: REG_SSR,
    reg_stfinst: REG_STFINST,
    reg_sysconf: REG_SYSCONF,
    reg_syscfg: REG_SYSCFG,
    reg_tid: REG_TID,
    reg_htid: -1,
    reg_tlbhi: REG_TLBHI,
    reg_tlbidx: REG_TLBIDX,
    reg_tlblo: REG_TLBLO,
    reg_ugp: REG_UGP,
    reg_usr: REG_USR,

/* Reference to v4 offsets outside of v4 mode should trigger a 
   fatal exception */
    reg_acc0: -1,
    reg_acc1: -1,
    reg_avs: -1,
    reg_badva0: -1,
    reg_badva1: -1,
    reg_ccr: -1,
    reg_cfgbase: -1,
    reg_chicken: -1,
    reg_cs0: -1,
    reg_cs1: -1,
    reg_g0: -1,
    reg_g1: -1,
    reg_g2: -1,
    reg_g3: -1,
    reg_htid: -1,
    reg_pmucfg: -1,
    reg_pmucnt0: -1,
    reg_pmucnt1: -1,
    reg_pmucnt2: -1,
    reg_pmucnt3: -1,
    reg_pmuevtcfg: -1,
    reg_rgdr: -1,
    reg_sgp0: -1,
    reg_sgp1: -1,
    reg_stid: -1,
    reg_upcyclehi: -1,
    reg_upcyclelo: -1,
    reg_upmucnt0: -1,
    reg_upmucnt1: -1,
    reg_upmucnt2: -1,
    reg_upmucnt3: -1,
    reg_vid: -1
};

#include "v4/reg_offsets.h"
static struct hexagon_system_register_offsets hexagon_reg_offset_v4 =
{
    reg_badva: REG_BADVA,
    reg_brkptcfg0: REG_BRKPTCFG0,
    reg_brkptcfg1: REG_BRKPTCFG1,
    reg_brkptinfo: REG_BRKPTINFO,
    reg_brkptpc0: REG_BRKPTPC0,
    reg_brkptpc1: REG_BRKPTPC1,
    reg_diag: REG_DIAG,
    reg_elr: REG_ELR,
    reg_evb: REG_EVB,
    reg_fp: REG_FP,
    reg_gp: REG_GP,
    reg_iad: REG_IAD,
    reg_iahl: REG_IAHL,
    reg_iel: REG_IEL,
    reg_imask: REG_IMASK,
    reg_ipend: REG_IPEND,
    reg_isdbcfg0: REG_ISDBCFG0,
    reg_isdbcfg1: REG_ISDBCFG1,
    reg_isdbcmd: REG_ISDBCMD,
    reg_isdben: REG_ISDBEN,
    reg_isdbgpr: REG_ISDBGPR,
    reg_isdbmbxin: REG_ISDBMBXIN,
    reg_isdbmbxout: REG_ISDBMBXOUT,
    reg_isdbst: REG_ISDBST,
    reg_isdbver: REG_ISDBVER,
    reg_lc0: REG_LC0,
    reg_lc1: REG_LC1,
    reg_lr: REG_LR,
    reg_m0: REG_M0,
    reg_m1: REG_M1,
    reg_modectl: REG_MODECTL,
    reg_p3_0: REG_P3_0,
    reg_pc: REG_PC,
    reg_pcyclehi: REG_PCYCLEHI,
    reg_pcyclelo: REG_PCYCLELO,
    reg_r00: REG_R00,
    reg_r01: REG_R01,
    reg_r02: REG_R02,
    reg_r03: REG_R03,
    reg_r04: REG_R04,
    reg_r05: REG_R05,
    reg_r06: REG_R06,
    reg_r07: REG_R07,
    reg_r08: REG_R08,
    reg_r09: REG_R09,
    reg_r10: REG_R10,
    reg_r11: REG_R11,
    reg_r12: REG_R12,
    reg_r13: REG_R13,
    reg_r14: REG_R14,
    reg_r15: REG_R15,
    reg_r16: REG_R16,
    reg_r17: REG_R17,
    reg_r18: REG_R18,
    reg_r19: REG_R19,
    reg_r20: REG_R20,
    reg_r21: REG_R21,
    reg_r22: REG_R22,
    reg_r23: REG_R23,
    reg_r24: REG_R24,
    reg_r25: REG_R25,
    reg_r26: REG_R26,
    reg_r27: REG_R27,
    reg_r28: REG_R28,
    reg_r29: REG_R29,
    reg_r30: REG_R30,
    reg_r31: REG_R31,
    reg_rev: REG_REV,
    reg_sa0: REG_SA0,
    reg_sa1: REG_SA1,
    reg_sgp: -1,
    reg_sp: REG_SP,
    reg_sr: -1,
    reg_ssr: REG_SSR,
    reg_stfinst: REG_STFINST,
    reg_sysconf: -1,
    reg_syscfg: REG_SYSCFG,
    reg_tid: -1,	/* TID maps to STID on V4 */
    reg_htid: REG_HTID,
    reg_tlbhi: -1,
    reg_tlbidx: -1,
    reg_tlblo: -1,
    reg_ugp: REG_UGP,
    reg_usr: REG_USR,

/* V4 specific registers */
    reg_acc0: REG_ACC0,
    reg_acc1: REG_ACC1,
    reg_avs: REG_AVS,
    reg_badva0: REG_BADVA0,
    reg_badva1: REG_BADVA1,
    reg_ccr: REG_CCR,
    reg_cfgbase: REG_CFGBASE,
    reg_chicken: REG_CHICKEN,
    reg_cs0: REG_CS0,
    reg_cs1: REG_CS1,
    reg_g0: REG_G0,
    reg_g1: REG_G1,
    reg_g2: REG_G2,
    reg_g3: REG_G3,
    reg_htid: REG_HTID,
    reg_pmucfg: REG_PMUCFG,
    reg_pmucnt0: REG_PMUCNT0,
    reg_pmucnt1: REG_PMUCNT1,
    reg_pmucnt2: REG_PMUCNT2,
    reg_pmucnt3: REG_PMUCNT3,
    reg_pmuevtcfg: REG_PMUEVTCFG,
    reg_rgdr: REG_RGDR,
    reg_sgp0: REG_SGP0,
    reg_sgp1: REG_SGP1,
    reg_stid: REG_STID,
    reg_upcyclehi: REG_UPCYCLEHI,
    reg_upcyclelo: REG_UPCYCLELO,
    reg_upmucnt0: REG_UPMUCNT0,
    reg_upmucnt1: REG_UPMUCNT1,
    reg_upmucnt2: REG_UPMUCNT2,
    reg_upmucnt3: REG_UPMUCNT3,
    reg_vid: REG_VID
};

/*
 * Now re-define all of these architecture specific offsets to use the
 * mapping structure.  Trigger a segv if usage is made prior to setting
 * the pointer.
 */
extern struct hexagon_system_register_offsets *hexagon_RegOffset;
#define hexagon_stopme(a) a

#undef REG_ACC0
#define REG_ACC0 hexagon_RegOffset?hexagon_RegOffset->reg_acc0:hexagon_stopme(-1)
#undef REG_ACC1
#define REG_ACC1 hexagon_RegOffset?hexagon_RegOffset->reg_acc1:hexagon_stopme(-1)
#undef REG_AVS
#define REG_AVS hexagon_RegOffset?hexagon_RegOffset->reg_avs:hexagon_stopme(-1)
#undef REG_BADVA
#define REG_BADVA hexagon_RegOffset?hexagon_RegOffset->reg_badva:hexagon_stopme(-1)
#undef REG_BADVA0
#define REG_BADVA0 hexagon_RegOffset?hexagon_RegOffset->reg_badva0:hexagon_stopme(-1)
#undef REG_BADVA1
#define REG_BADVA1 hexagon_RegOffset?hexagon_RegOffset->reg_badva1:hexagon_stopme(-1)
#undef REG_BRKPTCFG0
#define REG_BRKPTCFG0 hexagon_RegOffset?hexagon_RegOffset->reg_brkptcfg0:hexagon_stopme(-1)
#undef REG_BRKPTCFG1
#define REG_BRKPTCFG1 hexagon_RegOffset?hexagon_RegOffset->reg_brkptcfg1:hexagon_stopme(-1)
#undef REG_BRKPTINFO
#define REG_BRKPTINFO hexagon_RegOffset?hexagon_RegOffset->reg_brkptinfo:hexagon_stopme(-1)
#undef REG_BRKPTPC0
#define REG_BRKPTPC0 hexagon_RegOffset?hexagon_RegOffset->reg_brkptpc0:hexagon_stopme(-1)
#undef REG_BRKPTPC1
#define REG_BRKPTPC1 hexagon_RegOffset?hexagon_RegOffset->reg_brkptpc1:hexagon_stopme(-1)
#undef REG_CCR
#define REG_CCR hexagon_RegOffset?hexagon_RegOffset->reg_ccr:hexagon_stopme(-1)
#undef REG_CFGBASE
#define REG_CFGBASE hexagon_RegOffset?hexagon_RegOffset->reg_cfgbase:hexagon_stopme(-1)
#undef REG_CHICKEN
#define REG_CHICKEN hexagon_RegOffset?hexagon_RegOffset->reg_chicken:hexagon_stopme(-1)
#undef REG_CS0
#define REG_CS0 hexagon_RegOffset?hexagon_RegOffset->reg_cs0:hexagon_stopme(-1)
#undef REG_CS1
#define REG_CS1 hexagon_RegOffset?hexagon_RegOffset->reg_cs1:hexagon_stopme(-1)
#undef REG_DIAG
#define REG_DIAG hexagon_RegOffset?hexagon_RegOffset->reg_diag:hexagon_stopme(-1)
#undef REG_ELR
#define REG_ELR hexagon_RegOffset?hexagon_RegOffset->reg_elr:hexagon_stopme(-1)
#undef REG_EVB
#define REG_EVB hexagon_RegOffset?hexagon_RegOffset->reg_evb:hexagon_stopme(-1)
#undef REG_FP
#define REG_FP hexagon_RegOffset?hexagon_RegOffset->reg_fp:hexagon_stopme(-1)
#undef REG_G0
#define REG_G0 hexagon_RegOffset?hexagon_RegOffset->reg_g0:hexagon_stopme(-1)
#undef REG_G1
#define REG_G1 hexagon_RegOffset?hexagon_RegOffset->reg_g1:hexagon_stopme(-1)
#undef REG_G2
#define REG_G2 hexagon_RegOffset?hexagon_RegOffset->reg_g2:hexagon_stopme(-1)
#undef REG_G3
#define REG_G3 hexagon_RegOffset?hexagon_RegOffset->reg_g3:hexagon_stopme(-1)
#undef REG_GP
#define REG_GP hexagon_RegOffset?hexagon_RegOffset->reg_gp:hexagon_stopme(-1)
#undef REG_HTID
#define REG_HTID hexagon_RegOffset?hexagon_RegOffset->reg_htid:hexagon_stopme(-1)
#undef REG_IAD
#define REG_IAD hexagon_RegOffset?hexagon_RegOffset->reg_iad:hexagon_stopme(-1)
#undef REG_IAHL
#define REG_IAHL hexagon_RegOffset?hexagon_RegOffset->reg_iahl:hexagon_stopme(-1)
#undef REG_IEL
#define REG_IEL hexagon_RegOffset?hexagon_RegOffset->reg_iel:hexagon_stopme(-1)
#undef REG_IMASK
#define REG_IMASK hexagon_RegOffset?hexagon_RegOffset->reg_imask:hexagon_stopme(-1)
#undef REG_IPEND
#define REG_IPEND hexagon_RegOffset?hexagon_RegOffset->reg_ipend:hexagon_stopme(-1)
#undef REG_ISDBCFG0
#define REG_ISDBCFG0 hexagon_RegOffset?hexagon_RegOffset->reg_isdbcfg0:hexagon_stopme(-1)
#undef REG_ISDBCFG1
#define REG_ISDBCFG1 hexagon_RegOffset?hexagon_RegOffset->reg_isdbcfg1:hexagon_stopme(-1)
#undef REG_ISDBCMD
#define REG_ISDBCMD hexagon_RegOffset?hexagon_RegOffset->reg_isdbcmd:hexagon_stopme(-1)
#undef REG_ISDBEN
#define REG_ISDBEN hexagon_RegOffset?hexagon_RegOffset->reg_isdben:hexagon_stopme(-1)
#undef REG_ISDBGPR
#define REG_ISDBGPR hexagon_RegOffset?hexagon_RegOffset->reg_isdbgpr:hexagon_stopme(-1)
#undef REG_ISDBMBXIN
#define REG_ISDBMBXIN hexagon_RegOffset?hexagon_RegOffset->reg_isdbmbxin:hexagon_stopme(-1)
#undef REG_ISDBMBXOUT
#define REG_ISDBMBXOUT hexagon_RegOffset?hexagon_RegOffset->reg_isdbmbxout:hexagon_stopme(-1)
#undef REG_ISDBST
#define REG_ISDBST hexagon_RegOffset?hexagon_RegOffset->reg_isdbst:hexagon_stopme(-1)
#undef REG_ISDBVER
#define REG_ISDBVER hexagon_RegOffset?hexagon_RegOffset->reg_isdbver:hexagon_stopme(-1)
#undef REG_LC0
#define REG_LC0 hexagon_RegOffset?hexagon_RegOffset->reg_lc0:hexagon_stopme(-1)
#undef REG_LC1
#define REG_LC1 hexagon_RegOffset?hexagon_RegOffset->reg_lc1:hexagon_stopme(-1)
#undef REG_LR
#define REG_LR hexagon_RegOffset?hexagon_RegOffset->reg_lr:hexagon_stopme(-1)
#undef REG_M0
#define REG_M0 hexagon_RegOffset?hexagon_RegOffset->reg_m0:hexagon_stopme(-1)
#undef REG_M1
#define REG_M1 hexagon_RegOffset?hexagon_RegOffset->reg_m1:hexagon_stopme(-1)
#undef REG_MODECTL
#define REG_MODECTL hexagon_RegOffset?hexagon_RegOffset->reg_modectl:hexagon_stopme(-1)
#undef REG_P3_0
#define REG_P3_0 hexagon_RegOffset?hexagon_RegOffset->reg_p3_0:hexagon_stopme(-1)
#undef REG_PC
#define REG_PC hexagon_RegOffset?hexagon_RegOffset->reg_pc:hexagon_stopme(-1)
#undef REG_PCYCLEHI
#define REG_PCYCLEHI hexagon_RegOffset?hexagon_RegOffset->reg_pcyclehi:hexagon_stopme(-1)
#undef REG_PCYCLELO
#define REG_PCYCLELO hexagon_RegOffset?hexagon_RegOffset->reg_pcyclelo:hexagon_stopme(-1)
#undef REG_PMUCFG
#define REG_PMUCFG hexagon_RegOffset?hexagon_RegOffset->reg_pmucfg:hexagon_stopme(-1)
#undef REG_PMUCNT0
#define REG_PMUCNT0 hexagon_RegOffset?hexagon_RegOffset->reg_pmucnt0:hexagon_stopme(-1)
#undef REG_PMUCNT1
#define REG_PMUCNT1 hexagon_RegOffset?hexagon_RegOffset->reg_pmucnt1:hexagon_stopme(-1)
#undef REG_PMUCNT2
#define REG_PMUCNT2 hexagon_RegOffset?hexagon_RegOffset->reg_pmucnt2:hexagon_stopme(-1)
#undef REG_PMUCNT3
#define REG_PMUCNT3 hexagon_RegOffset?hexagon_RegOffset->reg_pmucnt3:hexagon_stopme(-1)
#undef REG_PMUEVTCFG
#define REG_PMUEVTCFG hexagon_RegOffset?hexagon_RegOffset->reg_pmuevtcfg:hexagon_stopme(-1)
#undef REG_R00
#define REG_R00 hexagon_RegOffset?hexagon_RegOffset->reg_r00:hexagon_stopme(-1)
#undef REG_R01
#define REG_R01 hexagon_RegOffset?hexagon_RegOffset->reg_r01:hexagon_stopme(-1)
#undef REG_R02
#define REG_R02 hexagon_RegOffset?hexagon_RegOffset->reg_r02:hexagon_stopme(-1)
#undef REG_R03
#define REG_R03 hexagon_RegOffset?hexagon_RegOffset->reg_r03:hexagon_stopme(-1)
#undef REG_R04
#define REG_R04 hexagon_RegOffset?hexagon_RegOffset->reg_r04:hexagon_stopme(-1)
#undef REG_R05
#define REG_R05 hexagon_RegOffset?hexagon_RegOffset->reg_r05:hexagon_stopme(-1)
#undef REG_R06
#define REG_R06 hexagon_RegOffset?hexagon_RegOffset->reg_r06:hexagon_stopme(-1)
#undef REG_R07
#define REG_R07 hexagon_RegOffset?hexagon_RegOffset->reg_r07:hexagon_stopme(-1)
#undef REG_R08
#define REG_R08 hexagon_RegOffset?hexagon_RegOffset->reg_r08:hexagon_stopme(-1)
#undef REG_R09
#define REG_R09 hexagon_RegOffset?hexagon_RegOffset->reg_r09:hexagon_stopme(-1)
#undef REG_R10
#define REG_R10 hexagon_RegOffset?hexagon_RegOffset->reg_r10:hexagon_stopme(-1)
#undef REG_R11
#define REG_R11 hexagon_RegOffset?hexagon_RegOffset->reg_r11:hexagon_stopme(-1)
#undef REG_R12
#define REG_R12 hexagon_RegOffset?hexagon_RegOffset->reg_r12:hexagon_stopme(-1)
#undef REG_R13
#define REG_R13 hexagon_RegOffset?hexagon_RegOffset->reg_r13:hexagon_stopme(-1)
#undef REG_R14
#define REG_R14 hexagon_RegOffset?hexagon_RegOffset->reg_r14:hexagon_stopme(-1)
#undef REG_R15
#define REG_R15 hexagon_RegOffset?hexagon_RegOffset->reg_r15:hexagon_stopme(-1)
#undef REG_R16
#define REG_R16 hexagon_RegOffset?hexagon_RegOffset->reg_r16:hexagon_stopme(-1)
#undef REG_R17
#define REG_R17 hexagon_RegOffset?hexagon_RegOffset->reg_r17:hexagon_stopme(-1)
#undef REG_R18
#define REG_R18 hexagon_RegOffset?hexagon_RegOffset->reg_r18:hexagon_stopme(-1)
#undef REG_R19
#define REG_R19 hexagon_RegOffset?hexagon_RegOffset->reg_r19:hexagon_stopme(-1)
#undef REG_R20
#define REG_R20 hexagon_RegOffset?hexagon_RegOffset->reg_r20:hexagon_stopme(-1)
#undef REG_R21
#define REG_R21 hexagon_RegOffset?hexagon_RegOffset->reg_r21:hexagon_stopme(-1)
#undef REG_R22
#define REG_R22 hexagon_RegOffset?hexagon_RegOffset->reg_r22:hexagon_stopme(-1)
#undef REG_R23
#define REG_R23 hexagon_RegOffset?hexagon_RegOffset->reg_r23:hexagon_stopme(-1)
#undef REG_R24
#define REG_R24 hexagon_RegOffset?hexagon_RegOffset->reg_r24:hexagon_stopme(-1)
#undef REG_R25
#define REG_R25 hexagon_RegOffset?hexagon_RegOffset->reg_r25:hexagon_stopme(-1)
#undef REG_R26
#define REG_R26 hexagon_RegOffset?hexagon_RegOffset->reg_r26:hexagon_stopme(-1)
#undef REG_R27
#define REG_R27 hexagon_RegOffset?hexagon_RegOffset->reg_r27:hexagon_stopme(-1)
#undef REG_R28
#define REG_R28 hexagon_RegOffset?hexagon_RegOffset->reg_r28:hexagon_stopme(-1)
#undef REG_R29
#define REG_R29 hexagon_RegOffset?hexagon_RegOffset->reg_r29:hexagon_stopme(-1)
#undef REG_R30
#define REG_R30 hexagon_RegOffset?hexagon_RegOffset->reg_r30:hexagon_stopme(-1)
#undef REG_R31
#define REG_R31 hexagon_RegOffset?hexagon_RegOffset->reg_r31:hexagon_stopme(-1)
#undef REG_REV
#define REG_REV hexagon_RegOffset?hexagon_RegOffset->reg_rev:hexagon_stopme(-1)
#undef REG_RGDR
#define REG_RGDR hexagon_RegOffset?hexagon_RegOffset->reg_rgdr:hexagon_stopme(-1)
#undef REG_SA0
#define REG_SA0 hexagon_RegOffset?hexagon_RegOffset->reg_sa0:hexagon_stopme(-1)
#undef REG_SA1
#define REG_SA1 hexagon_RegOffset?hexagon_RegOffset->reg_sa1:hexagon_stopme(-1)
#undef REG_SGP0
#define REG_SGP0 hexagon_RegOffset?hexagon_RegOffset->reg_sgp0:hexagon_stopme(-1)
#undef REG_SGP1
#define REG_SGP1 hexagon_RegOffset?hexagon_RegOffset->reg_sgp1:hexagon_stopme(-1)
#undef REG_SP
#define REG_SP hexagon_RegOffset?hexagon_RegOffset->reg_sp:hexagon_stopme(-1)
#undef REG_SSR
#define REG_SSR hexagon_RegOffset?hexagon_RegOffset->reg_ssr:hexagon_stopme(-1)
#undef REG_STFINST
#define REG_STFINST hexagon_RegOffset?hexagon_RegOffset->reg_stfinst:hexagon_stopme(-1)
#undef REG_STID
#define REG_STID hexagon_RegOffset?hexagon_RegOffset->reg_stid:hexagon_stopme(-1)
#undef REG_SYSCFG
#define REG_SYSCFG hexagon_RegOffset?hexagon_RegOffset->reg_syscfg:hexagon_stopme(-1)
#undef REG_UGP
#define REG_UGP hexagon_RegOffset?hexagon_RegOffset->reg_ugp:hexagon_stopme(-1)
#undef REG_UPCYCLEHI
#define REG_UPCYCLEHI hexagon_RegOffset?hexagon_RegOffset->reg_upcyclehi:hexagon_stopme(-1)
#undef REG_UPCYCLELO
#define REG_UPCYCLELO hexagon_RegOffset?hexagon_RegOffset->reg_upcyclelo:hexagon_stopme(-1)
#undef REG_UPMUCNT0
#define REG_UPMUCNT0 hexagon_RegOffset?hexagon_RegOffset->reg_upmucnt0:hexagon_stopme(-1)
#undef REG_UPMUCNT1
#define REG_UPMUCNT1 hexagon_RegOffset?hexagon_RegOffset->reg_upmucnt1:hexagon_stopme(-1)
#undef REG_UPMUCNT2
#define REG_UPMUCNT2 hexagon_RegOffset?hexagon_RegOffset->reg_upmucnt2:hexagon_stopme(-1)
#undef REG_UPMUCNT3
#define REG_UPMUCNT3 hexagon_RegOffset?hexagon_RegOffset->reg_upmucnt3:hexagon_stopme(-1)
#undef REG_VID
#define REG_VID hexagon_RegOffset?hexagon_RegOffset->reg_vid:hexagon_stopme(-1)
#undef REG_USR
#define REG_USR hexagon_RegOffset?hexagon_RegOffset->reg_usr:hexagon_stopme(-1)
#undef REG_SGP
#define REG_SGP hexagon_RegOffset?hexagon_RegOffset->reg_sgp:hexagon_stopme(-1)
#undef REG_SR
#define REG_SR hexagon_RegOffset?hexagon_RegOffset->reg_sr:hexagon_stopme(-1)
#undef REG_SYSCONF
#define REG_SYSCONF hexagon_RegOffset?hexagon_RegOffset->reg_sysconf:hexagon_stopme(-1)
#undef REG_TID
#define REG_TID hexagon_RegOffset?hexagon_RegOffset->reg_tid:hexagon_stopme(-1)
#undef REG_TLBHI
#define REG_TLBHI hexagon_RegOffset?hexagon_RegOffset->reg_tlbhi:hexagon_stopme(-1)
#undef REG_TLBIDX
#define REG_TLBIDX hexagon_RegOffset?hexagon_RegOffset->reg_tlbidx:hexagon_stopme(-1)
#undef REG_TLBLO
#define REG_TLBLO hexagon_RegOffset?hexagon_RegOffset->reg_tlblo:hexagon_stopme(-1)


/*
 * Then following register values taken from architecture register
 * definitions
 */
#define MAXREGNAMELEN     50
#define NUM_GEN_REGS      32
#define NUM_PER_THREAD_CR 40
#define NUM_GLOBAL_REGS   64
#define TOTAL_PER_THREAD_REGS (NUM_GEN_REGS+NUM_PER_THREAD_CR)
#define NUM_GLOBAL_REGS 64


/*
 * Prolog mapping to specific instructions
 * TODO: this is not sufficient for v4 insn
 * pairing.
 */
    /* Scan the prologue.  */
    /* for ignoring 11 immediate bits */
#define   ALLOCFRAME_OPCODE_MASK 0xFFFFF800UL
    /* allocframe opcode ignoring immediate bits */
#define   ALLOCFRAME_OPCODE_BITS 0xA09DC000UL
#define   ALLOCFRAME_SIZE_BITS 0x000007FFUL
#define   ALLOCFRAME_SIZE_SHIFT 3UL


    /* for ignoring immediate bits */
#define   MORE_SP_UPDATE_OPCODE_MASK 0xF01FC01FUL
    /* sp update opcode ignoring immediate bits 
      (uses Rd=add(Rs,#s16) where Rd and Rs r29 */
#define   MORE_SP_UPDATE_OPCODE_BITS 0xB01DC01DUL
    /* for ignoring immediate and Rt bits for 
        byte/half/single/double word */
#define   CALLEE_SAVE_OPCODE_MASK 0xF1DFC000UL
    /* callee save opcode ignoring immediate and Rt
       bits (uses memb(Rs+#11:0)=Rt where Rs r29
       and Rt callee save register */
#define   CALLEE_SAVE_OPCODE_BITS_B 0xA11DC000UL
    /* callee save opcode ignoring immediate and Rt
       bits (uses memh(Rs+#11:1)=Rt where Rs r29
       and Rt callee save register */
#define   CALLEE_SAVE_OPCODE_BITS_H 0xA15DC000UL
    /* callee save opcode ignoring immediate and Rt
       bits (uses memw(Rs+#11:2)=Rt where Rs r29
       and Rt callee save register */
#define   CALLEE_SAVE_OPCODE_BITS_W 0xA19DC000UL
    /* callee save opcode ignoring immediate and Rt
       bits (uses memd(Rs+#11:3)=Rt where Rs r29
       and Rt callee save register */
#define   CALLEE_SAVE_OPCODE_BITS_D 0xA1DDC000UL
    /* for ignoring immediate and Rt bits */
#define   FUNC_ARG_SAVE_OPCODE_MASK 0xA19FC000L
    /* callee save opcode ignoring immediate and Rt
       bits (uses memw(Rs+#s11:2)=Rt where Rs r30
       and Rt callee save register */
#define   FUNC_ARG_SAVE_OPCODE_BITS 0xA19EC000UL
#define   FUNC_ARG_SAVE_REG_BITS 0x1FUL
#define   FUNC_ARG_SAVE_REG_SHIFT 8UL

#define FUNC_ARG_SAVE_REG(opcode) (((opcode) >> FUNC_ARG_SAVE_REG_SHIFT) & FUNC_ARG_SAVE_REG_BITS)

#define ALLOCFRAME_MATCH(opcode) \
        (ALLOCFRAME_OPCODE_BITS == (ALLOCFRAME_OPCODE_MASK & (opcode)))
#define ALLOCFRAME_SIZE(opcode) (((opcode) & ALLOCFRAME_SIZE_BITS) << ALLOCFRAME_SIZE_SHIFT)

#define IMMEXT_MATCH(opcode) ((opcode & 0xF0000000) == 0x0)

#define MORE_SP_UPDATE_MATCH(opcode) \
        (MORE_SP_UPDATE_OPCODE_BITS == (MORE_SP_UPDATE_OPCODE_MASK & (opcode)))
#define MORE_SP_UPDATE_SIZE(opcode) \
        ((signed short) ((((unsigned short) ((op >> 21) & 0x7F)) << 9) | \
                          ((unsigned short) ((op >> 5) & 0x1FF))))
// for byte accesses - check if it is a store byte insn 
#define CALLEE_SAVE_MATCH_B(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_B == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 
// get if Rt if a calle saved register             
#define CALLEE_SAVE_REG(opcode) (((opcode) >> 8) & 0x1F)


// for byte accesses  - skip ignore bits
#define CALLEE_SAVE_OFFSET_B(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  0))))


// for half word  accesses - check if it is a store hword insn 
#define CALLEE_SAVE_MATCH_H(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_H == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 

// for half wordaccesses  - skip ignore bits
#define CALLEE_SAVE_OFFSET_H(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  1))))


// for single word accesses
#define CALLEE_SAVE_MATCH_W(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_W == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 

// for single word accesses
#define CALLEE_SAVE_OFFSET_W(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  2))))




// for double word accesses
#define CALLEE_SAVE_MATCH_D(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_D == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 


// for double word accesses
#define CALLEE_SAVE_OFFSET_D(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  3))))


#define FUNC_ARG_SAVE_MATCH(opcode) \
        (FUNC_ARG_SAVE_OPCODE_BITS == (FUNC_ARG_SAVE_OPCODE_MASK & (opcode)) && \
             is_argument_reg(FUNC_ARG_SAVE_REG(opcode)))



#ifdef WIN32
#define strdupa strdup
#define sleep(n) Sleep(n*1000)
#endif

#endif /* HEXAGON_TDEP_H */
