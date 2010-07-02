
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
    reg_upmucnt3: -1
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
    reg_upmucnt3: REG_UPMUCNT3
};

/*
 * Now re-define all of these architecture specific offsets to use the
 * mapping structure.  Trigger a segv if usage is made prior to setting
 * the pointer.
 */
static int q6stopme(int var)
{
	while (1);
}
#undef REG_ACC0
#define REG_ACC0 q6RegOffset?q6RegOffset->reg_acc0:q6stopme(-1)
#undef REG_ACC1
#define REG_ACC1 q6RegOffset?q6RegOffset->reg_acc1:q6stopme(-1)
#undef REG_AVS
#define REG_AVS q6RegOffset?q6RegOffset->reg_avs:q6stopme(-1)
#undef REG_BADVA
#define REG_BADVA q6RegOffset?q6RegOffset->reg_badva:q6stopme(-1)
#undef REG_BADVA0
#define REG_BADVA0 q6RegOffset?q6RegOffset->reg_badva0:q6stopme(-1)
#undef REG_BADVA1
#define REG_BADVA1 q6RegOffset?q6RegOffset->reg_badva1:q6stopme(-1)
#undef REG_BRKPTCFG0
#define REG_BRKPTCFG0 q6RegOffset?q6RegOffset->reg_brkptcfg0:q6stopme(-1)
#undef REG_BRKPTCFG1
#define REG_BRKPTCFG1 q6RegOffset?q6RegOffset->reg_brkptcfg1:q6stopme(-1)
#undef REG_BRKPTINFO
#define REG_BRKPTINFO q6RegOffset?q6RegOffset->reg_brkptinfo:q6stopme(-1)
#undef REG_BRKPTPC0
#define REG_BRKPTPC0 q6RegOffset?q6RegOffset->reg_brkptpc0:q6stopme(-1)
#undef REG_BRKPTPC1
#define REG_BRKPTPC1 q6RegOffset?q6RegOffset->reg_brkptpc1:q6stopme(-1)
#undef REG_CCR
#define REG_CCR q6RegOffset?q6RegOffset->reg_ccr:q6stopme(-1)
#undef REG_CFGBASE
#define REG_CFGBASE q6RegOffset?q6RegOffset->reg_cfgbase:q6stopme(-1)
#undef REG_CHICKEN
#define REG_CHICKEN q6RegOffset?q6RegOffset->reg_chicken:q6stopme(-1)
#undef REG_CS0
#define REG_CS0 q6RegOffset?q6RegOffset->reg_cs0:q6stopme(-1)
#undef REG_CS1
#define REG_CS1 q6RegOffset?q6RegOffset->reg_cs1:q6stopme(-1)
#undef REG_DIAG
#define REG_DIAG q6RegOffset?q6RegOffset->reg_diag:q6stopme(-1)
#undef REG_ELR
#define REG_ELR q6RegOffset?q6RegOffset->reg_elr:q6stopme(-1)
#undef REG_EVB
#define REG_EVB q6RegOffset?q6RegOffset->reg_evb:q6stopme(-1)
#undef REG_FP
#define REG_FP q6RegOffset?q6RegOffset->reg_fp:q6stopme(-1)
#undef REG_G0
#define REG_G0 q6RegOffset?q6RegOffset->reg_g0:q6stopme(-1)
#undef REG_G1
#define REG_G1 q6RegOffset?q6RegOffset->reg_g1:q6stopme(-1)
#undef REG_G2
#define REG_G2 q6RegOffset?q6RegOffset->reg_g2:q6stopme(-1)
#undef REG_G3
#define REG_G3 q6RegOffset?q6RegOffset->reg_g3:q6stopme(-1)
#undef REG_GP
#define REG_GP q6RegOffset?q6RegOffset->reg_gp:q6stopme(-1)
#undef REG_HTID
#define REG_HTID q6RegOffset?q6RegOffset->reg_htid:q6stopme(-1)
#undef REG_IAD
#define REG_IAD q6RegOffset?q6RegOffset->reg_iad:q6stopme(-1)
#undef REG_IAHL
#define REG_IAHL q6RegOffset?q6RegOffset->reg_iahl:q6stopme(-1)
#undef REG_IEL
#define REG_IEL q6RegOffset?q6RegOffset->reg_iel:q6stopme(-1)
#undef REG_IMASK
#define REG_IMASK q6RegOffset?q6RegOffset->reg_imask:q6stopme(-1)
#undef REG_IPEND
#define REG_IPEND q6RegOffset?q6RegOffset->reg_ipend:q6stopme(-1)
#undef REG_ISDBCFG0
#define REG_ISDBCFG0 q6RegOffset?q6RegOffset->reg_isdbcfg0:q6stopme(-1)
#undef REG_ISDBCFG1
#define REG_ISDBCFG1 q6RegOffset?q6RegOffset->reg_isdbcfg1:q6stopme(-1)
#undef REG_ISDBCMD
#define REG_ISDBCMD q6RegOffset?q6RegOffset->reg_isdbcmd:q6stopme(-1)
#undef REG_ISDBEN
#define REG_ISDBEN q6RegOffset?q6RegOffset->reg_isdben:q6stopme(-1)
#undef REG_ISDBGPR
#define REG_ISDBGPR q6RegOffset?q6RegOffset->reg_isdbgpr:q6stopme(-1)
#undef REG_ISDBMBXIN
#define REG_ISDBMBXIN q6RegOffset?q6RegOffset->reg_isdbmbxin:q6stopme(-1)
#undef REG_ISDBMBXOUT
#define REG_ISDBMBXOUT q6RegOffset?q6RegOffset->reg_isdbmbxout:q6stopme(-1)
#undef REG_ISDBST
#define REG_ISDBST q6RegOffset?q6RegOffset->reg_isdbst:q6stopme(-1)
#undef REG_ISDBVER
#define REG_ISDBVER q6RegOffset?q6RegOffset->reg_isdbver:q6stopme(-1)
#undef REG_LC0
#define REG_LC0 q6RegOffset?q6RegOffset->reg_lc0:q6stopme(-1)
#undef REG_LC1
#define REG_LC1 q6RegOffset?q6RegOffset->reg_lc1:q6stopme(-1)
#undef REG_LR
#define REG_LR q6RegOffset?q6RegOffset->reg_lr:q6stopme(-1)
#undef REG_M0
#define REG_M0 q6RegOffset?q6RegOffset->reg_m0:q6stopme(-1)
#undef REG_M1
#define REG_M1 q6RegOffset?q6RegOffset->reg_m1:q6stopme(-1)
#undef REG_MODECTL
#define REG_MODECTL q6RegOffset?q6RegOffset->reg_modectl:q6stopme(-1)
#undef REG_P3_0
#define REG_P3_0 q6RegOffset?q6RegOffset->reg_p3_0:q6stopme(-1)
#undef REG_PC
#define REG_PC q6RegOffset?q6RegOffset->reg_pc:q6stopme(-1)
#undef REG_PCYCLEHI
#define REG_PCYCLEHI q6RegOffset?q6RegOffset->reg_pcyclehi:q6stopme(-1)
#undef REG_PCYCLELO
#define REG_PCYCLELO q6RegOffset?q6RegOffset->reg_pcyclelo:q6stopme(-1)
#undef REG_PMUCFG
#define REG_PMUCFG q6RegOffset?q6RegOffset->reg_pmucfg:q6stopme(-1)
#undef REG_PMUCNT0
#define REG_PMUCNT0 q6RegOffset?q6RegOffset->reg_pmucnt0:q6stopme(-1)
#undef REG_PMUCNT1
#define REG_PMUCNT1 q6RegOffset?q6RegOffset->reg_pmucnt1:q6stopme(-1)
#undef REG_PMUCNT2
#define REG_PMUCNT2 q6RegOffset?q6RegOffset->reg_pmucnt2:q6stopme(-1)
#undef REG_PMUCNT3
#define REG_PMUCNT3 q6RegOffset?q6RegOffset->reg_pmucnt3:q6stopme(-1)
#undef REG_PMUEVTCFG
#define REG_PMUEVTCFG q6RegOffset?q6RegOffset->reg_pmuevtcfg:q6stopme(-1)
#undef REG_R00
#define REG_R00 q6RegOffset?q6RegOffset->reg_r00:q6stopme(-1)
#undef REG_R01
#define REG_R01 q6RegOffset?q6RegOffset->reg_r01:q6stopme(-1)
#undef REG_R02
#define REG_R02 q6RegOffset?q6RegOffset->reg_r02:q6stopme(-1)
#undef REG_R03
#define REG_R03 q6RegOffset?q6RegOffset->reg_r03:q6stopme(-1)
#undef REG_R04
#define REG_R04 q6RegOffset?q6RegOffset->reg_r04:q6stopme(-1)
#undef REG_R05
#define REG_R05 q6RegOffset?q6RegOffset->reg_r05:q6stopme(-1)
#undef REG_R06
#define REG_R06 q6RegOffset?q6RegOffset->reg_r06:q6stopme(-1)
#undef REG_R07
#define REG_R07 q6RegOffset?q6RegOffset->reg_r07:q6stopme(-1)
#undef REG_R08
#define REG_R08 q6RegOffset?q6RegOffset->reg_r08:q6stopme(-1)
#undef REG_R09
#define REG_R09 q6RegOffset?q6RegOffset->reg_r09:q6stopme(-1)
#undef REG_R10
#define REG_R10 q6RegOffset?q6RegOffset->reg_r10:q6stopme(-1)
#undef REG_R11
#define REG_R11 q6RegOffset?q6RegOffset->reg_r11:q6stopme(-1)
#undef REG_R12
#define REG_R12 q6RegOffset?q6RegOffset->reg_r12:q6stopme(-1)
#undef REG_R13
#define REG_R13 q6RegOffset?q6RegOffset->reg_r13:q6stopme(-1)
#undef REG_R14
#define REG_R14 q6RegOffset?q6RegOffset->reg_r14:q6stopme(-1)
#undef REG_R15
#define REG_R15 q6RegOffset?q6RegOffset->reg_r15:q6stopme(-1)
#undef REG_R16
#define REG_R16 q6RegOffset?q6RegOffset->reg_r16:q6stopme(-1)
#undef REG_R17
#define REG_R17 q6RegOffset?q6RegOffset->reg_r17:q6stopme(-1)
#undef REG_R18
#define REG_R18 q6RegOffset?q6RegOffset->reg_r18:q6stopme(-1)
#undef REG_R19
#define REG_R19 q6RegOffset?q6RegOffset->reg_r19:q6stopme(-1)
#undef REG_R20
#define REG_R20 q6RegOffset?q6RegOffset->reg_r20:q6stopme(-1)
#undef REG_R21
#define REG_R21 q6RegOffset?q6RegOffset->reg_r21:q6stopme(-1)
#undef REG_R22
#define REG_R22 q6RegOffset?q6RegOffset->reg_r22:q6stopme(-1)
#undef REG_R23
#define REG_R23 q6RegOffset?q6RegOffset->reg_r23:q6stopme(-1)
#undef REG_R24
#define REG_R24 q6RegOffset?q6RegOffset->reg_r24:q6stopme(-1)
#undef REG_R25
#define REG_R25 q6RegOffset?q6RegOffset->reg_r25:q6stopme(-1)
#undef REG_R26
#define REG_R26 q6RegOffset?q6RegOffset->reg_r26:q6stopme(-1)
#undef REG_R27
#define REG_R27 q6RegOffset?q6RegOffset->reg_r27:q6stopme(-1)
#undef REG_R28
#define REG_R28 q6RegOffset?q6RegOffset->reg_r28:q6stopme(-1)
#undef REG_R29
#define REG_R29 q6RegOffset?q6RegOffset->reg_r29:q6stopme(-1)
#undef REG_R30
#define REG_R30 q6RegOffset?q6RegOffset->reg_r30:q6stopme(-1)
#undef REG_R31
#define REG_R31 q6RegOffset?q6RegOffset->reg_r31:q6stopme(-1)
#undef REG_REV
#define REG_REV q6RegOffset?q6RegOffset->reg_rev:q6stopme(-1)
#undef REG_RGDR
#define REG_RGDR q6RegOffset?q6RegOffset->reg_rgdr:q6stopme(-1)
#undef REG_SA0
#define REG_SA0 q6RegOffset?q6RegOffset->reg_sa0:q6stopme(-1)
#undef REG_SA1
#define REG_SA1 q6RegOffset?q6RegOffset->reg_sa1:q6stopme(-1)
#undef REG_SGP0
#define REG_SGP0 q6RegOffset?q6RegOffset->reg_sgp0:q6stopme(-1)
#undef REG_SGP1
#define REG_SGP1 q6RegOffset?q6RegOffset->reg_sgp1:q6stopme(-1)
#undef REG_SP
#define REG_SP q6RegOffset?q6RegOffset->reg_sp:q6stopme(-1)
#undef REG_SSR
#define REG_SSR q6RegOffset?q6RegOffset->reg_ssr:q6stopme(-1)
#undef REG_STFINST
#define REG_STFINST q6RegOffset?q6RegOffset->reg_stfinst:q6stopme(-1)
#undef REG_STID
#define REG_STID q6RegOffset?q6RegOffset->reg_stid:q6stopme(-1)
#undef REG_SYSCFG
#define REG_SYSCFG q6RegOffset?q6RegOffset->reg_syscfg:q6stopme(-1)
#undef REG_UGP
#define REG_UGP q6RegOffset?q6RegOffset->reg_ugp:q6stopme(-1)
#undef REG_UPCYCLEHI
#define REG_UPCYCLEHI q6RegOffset?q6RegOffset->reg_upcyclehi:q6stopme(-1)
#undef REG_UPCYCLELO
#define REG_UPCYCLELO q6RegOffset?q6RegOffset->reg_upcyclelo:q6stopme(-1)
#undef REG_UPMUCNT0
#define REG_UPMUCNT0 q6RegOffset?q6RegOffset->reg_upmucnt0:q6stopme(-1)
#undef REG_UPMUCNT1
#define REG_UPMUCNT1 q6RegOffset?q6RegOffset->reg_upmucnt1:q6stopme(-1)
#undef REG_UPMUCNT2
#define REG_UPMUCNT2 q6RegOffset?q6RegOffset->reg_upmucnt2:q6stopme(-1)
#undef REG_UPMUCNT3
#define REG_UPMUCNT3 q6RegOffset?q6RegOffset->reg_upmucnt3:q6stopme(-1)
#undef REG_USR
#define REG_USR q6RegOffset?q6RegOffset->reg_usr:q6stopme(-1)
#undef REG_SGP
#define REG_SGP q6RegOffset?q6RegOffset->reg_sgp:q6stopme(-1)
#undef REG_SR
#define REG_SR q6RegOffset?q6RegOffset->reg_sr:q6stopme(-1)
#undef REG_SYSCONF
#define REG_SYSCONF q6RegOffset?q6RegOffset->reg_sysconf:q6stopme(-1)
#undef REG_TID
#define REG_TID q6RegOffset?q6RegOffset->reg_tid:q6stopme(-1)
#undef REG_TLBHI
#define REG_TLBHI q6RegOffset?q6RegOffset->reg_tlbhi:q6stopme(-1)
#undef REG_TLBIDX
#define REG_TLBIDX q6RegOffset?q6RegOffset->reg_tlbidx:q6stopme(-1)
#undef REG_TLBLO
#define REG_TLBLO q6RegOffset?q6RegOffset->reg_tlblo:q6stopme(-1)
