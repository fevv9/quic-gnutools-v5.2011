/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.  All Rights
# Reserved.  Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/* Target-dependent code for QUALCOMM QDSP6 GDB, the GNU Debugger.
   Copyright 2002, 2003, 2004 Free Software Foundation, Inc.

   This file is part of GDB.

*/

/*
   - data structure: qdsp6_system_register_offsets
   - description:
	-- This is used to provide a mapping to registers between different
  	   QDSP6 architecture revisions.
	-- TODO: more V4 registers to be added.
 */
struct qdsp6_system_register_offsets
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
    int reg_tid;
    int reg_htid;
    int reg_tlbhi;
    int reg_tlbidx;
    int reg_tlblo;
    int reg_ugp;
    int reg_usr;
};


#include "reg_offsets.h"
struct qdsp6_system_register_offsets reg_offset =
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
    reg_usr: REG_USR
};

#include "v4/reg_offsets.h"
struct qdsp6_system_register_offsets reg_offset_v4 =
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
    reg_htid: REG_HTID,
    reg_tlbhi: REG_TLBHI,
    reg_tlbidx: REG_TLBIDX,
    reg_tlblo: REG_TLBLO,
    reg_ugp: REG_UGP,
    reg_usr: REG_USR
};

/*
 * Now re-define all of these architecture specific offsets to use the
 * mapping structure.  Trigger a segv if usage is made prior to setting
 * the pointer.
 */
#undef REG_SA0
#define REG_SA0 q6RegOffset?q6RegOffset->reg_sa0:-1
