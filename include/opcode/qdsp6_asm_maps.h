/*
 * Source Tag: A2_addsp
 * Source Syntax: Rdd32=add(Rs32,Rtt32)
 * Dest Syntax: Rdd32=add(Rss32,Rtt32):raw:hi
 * Dest Syntax2: Rdd32=add(Rss32,Rtt32):raw:lo
 * Condition: Rs32 & 1
 *
 */
static
MAP_FUNCTION(A2_addsp)
{
	if (GET_OP_VAL(1) & 1) {
		sprintf(DEST,"R%d:%d=add(R%d:%d,R%d:%d):raw:hi",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1) |1,GET_OP_VAL(1) & -2,GET_OP_VAL(2)+1,GET_OP_VAL(2));
	} else {
		sprintf(DEST,"R%d:%d=add(R%d:%d,R%d:%d):raw:lo",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1) |1,GET_OP_VAL(1) & -2,GET_OP_VAL(2)+1,GET_OP_VAL(2));
	}
}

/*
 * Source Tag: A2_neg
 * Source Syntax: Rd32=neg(Rs32)
 * Dest Syntax: Rd32=sub(#0,Rs32)
 *
 */
static
MAP_FUNCTION(A2_neg)
{
    sprintf(DEST,"R%d=sub(#0,R%d)",GET_OP_VAL(0),GET_OP_VAL(1));
}

/*
 * Source Tag: A2_not
 * Source Syntax: Rd32=not(Rs32)
 * Dest Syntax: Rd32=sub(#-1,Rs32)
 *
 */
static
MAP_FUNCTION(A2_not)
{
    sprintf(DEST,"R%d=sub(#-1,R%d)",GET_OP_VAL(0),GET_OP_VAL(1));
}

/*
 * Source Tag: A2_tfrf
 * Source Syntax: if (!Pu4) Rd32=Rs32
 * Dest Syntax: if (!Pu4) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrf)
{
    sprintf(DEST,"if (!P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrf_alt
 * Source Syntax: if !Pu4 Rd32=Rs32
 * Dest Syntax: if (!Pu4) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrf_alt)
{
    sprintf(DEST,"if (!P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrfnew
 * Source Syntax: if (!Pu4.new) Rd32=Rs32
 * Dest Syntax: if (!Pu4.new) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrfnew)
{
    sprintf(DEST,"if (!P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrfnew_alt
 * Source Syntax: if !Pu4.new Rd32=Rs32
 * Dest Syntax: if (!Pu4.new) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrfnew_alt)
{
    sprintf(DEST,"if (!P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrp
 * Source Syntax: Rdd32=Rss32
 * Dest Syntax: Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrp)
{
    sprintf(DEST,"R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
}

/*
 * Source Tag: A2_tfrpf
 * Source Syntax: if (!Pu4) Rdd32=Rss32
 * Dest Syntax: if (!Pu4) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrpf)
{
    sprintf(DEST,"if (!P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrpf_alt
 * Source Syntax: if !Pu4 Rdd32=Rss32
 * Dest Syntax: if (!Pu4) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrpf_alt)
{
    sprintf(DEST,"if (!P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrpfnew
 * Source Syntax: if (!Pu4.new) Rdd32=Rss32
 * Dest Syntax: if (!Pu4.new) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrpfnew)
{
    sprintf(DEST,"if (!P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrpfnew_alt
 * Source Syntax: if !Pu4.new Rdd32=Rss32
 * Dest Syntax: if (!Pu4.new) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrpfnew_alt)
{
    sprintf(DEST,"if (!P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrpi
 * Source Syntax: Rdd32=#s8
 * Dest Syntax: Rdd32=combine(#-1,#s8)
 * Dest Syntax2: Rdd32=combine(#0,#s8)
 * Condition: #s8<0
 *
 */
static
MAP_FUNCTION(A2_tfrpi)
{
	if (GET_OP_VAL(1)<0) {
		sprintf(DEST,"R%d:%d=combine(#-1,#%d)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
	} else {
		sprintf(DEST,"R%d:%d=combine(#0,#%d)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
	}
}

/*
 * Source Tag: A2_tfrpt
 * Source Syntax: if (Pu4) Rdd32=Rss32
 * Dest Syntax: if (Pu4) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrpt)
{
    sprintf(DEST,"if (P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrpt_alt
 * Source Syntax: if Pu4 Rdd32=Rss32
 * Dest Syntax: if (Pu4) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrpt_alt)
{
    sprintf(DEST,"if (P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrptnew
 * Source Syntax: if (Pu4.new) Rdd32=Rss32
 * Dest Syntax: if (Pu4.new) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrptnew)
{
    sprintf(DEST,"if (P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrptnew_alt
 * Source Syntax: if Pu4.new Rdd32=Rss32
 * Dest Syntax: if (Pu4.new) Rdd32=combine(Rss.H32,Rss.L32)
 *
 */
static
MAP_FUNCTION(A2_tfrptnew_alt)
{
    sprintf(DEST,"if (P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrt
 * Source Syntax: if (Pu4) Rd32=Rs32
 * Dest Syntax: if (Pu4) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrt)
{
    sprintf(DEST,"if (P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrt_alt
 * Source Syntax: if Pu4 Rd32=Rs32
 * Dest Syntax: if (Pu4) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrt_alt)
{
    sprintf(DEST,"if (P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrtnew
 * Source Syntax: if (Pu4.new) Rd32=Rs32
 * Dest Syntax: if (Pu4.new) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrtnew)
{
    sprintf(DEST,"if (P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_tfrtnew_alt
 * Source Syntax: if Pu4.new Rd32=Rs32
 * Dest Syntax: if (Pu4.new) Rd32=add(Rs32,#0)
 *
 */
static
MAP_FUNCTION(A2_tfrtnew_alt)
{
    sprintf(DEST,"if (P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: A2_zxtb
 * Source Syntax: Rd32=zxtb(Rs32)
 * Dest Syntax: Rd32=and(Rs32,#255)
 *
 */
static
MAP_FUNCTION(A2_zxtb)
{
    sprintf(DEST,"R%d=and(R%d,#255)",GET_OP_VAL(0),GET_OP_VAL(1));
}

/*
 * Source Tag: C2_cmpgei
 * Source Syntax: Pd4=cmp.ge(Rs32,#s8)
 * Dest Syntax: Pd4=cmp.gt(Rs32,#s8-1)
 *
 */
static
MAP_FUNCTION(C2_cmpgei)
{
    sprintf(DEST,"P%d=cmp.gt(R%d,#%d-1)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: C2_cmpgeui
 * Source Syntax: Pd4=cmp.geu(Rs32,#u8)
 * Dest Syntax: Pd4=cmp.eq(Rs32,Rs32)
 * Dest Syntax2: Pd4=cmp.gtu(Rs32,#u8-1)
 * Condition: #u8==0
 *
 */
static
MAP_FUNCTION(C2_cmpgeui)
{
	if (GET_OP_VAL(2)==0) {
		sprintf(DEST,"P%d=cmp.eq(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(1));
	} else {
		sprintf(DEST,"P%d=cmp.gtu(R%d,#%d-1)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
	}
}

/*
 * Source Tag: C2_cmplt
 * Source Syntax: Pd4=cmp.lt(Rs32,Rt32)
 * Dest Syntax: Pd4=cmp.gt(Rt32,Rs32)
 *
 */
static
MAP_FUNCTION(C2_cmplt)
{
    sprintf(DEST,"P%d=cmp.gt(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(2),GET_OP_VAL(1));
}

/*
 * Source Tag: C2_cmpltu
 * Source Syntax: Pd4=cmp.ltu(Rs32,Rt32)
 * Dest Syntax: Pd4=cmp.gtu(Rt32,Rs32)
 *
 */
static
MAP_FUNCTION(C2_cmpltu)
{
    sprintf(DEST,"P%d=cmp.gtu(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(2),GET_OP_VAL(1));
}

/*
 * Source Tag: C2_pxfer_map
 * Source Syntax: Pd4=Ps4
 * Dest Syntax: Pd4=or(Ps4,Ps4)
 *
 */
static
MAP_FUNCTION(C2_pxfer_map)
{
    sprintf(DEST,"P%d=or(P%d,P%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(1));
}

/*
 * Source Tag: M2_mpysmi
 * Source Syntax: Rd32=mpyi(Rs32,#m9)
 * Dest Syntax: Rd32=-mpyi(Rs32,#m9*(-1))
 * Dest Syntax2: Rd32=+mpyi(Rs32,#m9)
 * Condition: #m9<0
 *
 */
static
MAP_FUNCTION(M2_mpysmi)
{
	if (GET_OP_VAL(2)<0) {
		sprintf(DEST,"R%d=-mpyi(R%d,#%d*(-1))",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
	} else {
		sprintf(DEST,"R%d=+mpyi(R%d,#%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
	}
}

/*
 * Source Tag: M2_mpyui
 * Source Syntax: Rd32=mpyui(Rs32,Rt32)
 * Dest Syntax: Rd32=mpyi(Rs32,Rt32)
 *
 */
static
MAP_FUNCTION(M2_mpyui)
{
    sprintf(DEST,"R%d=mpyi(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}

/*
 * Source Tag: M2_vrcmpys_acc_s1
 * Source Syntax: Rxx32+=vrcmpys(Rss32,Rt32):<<1:sat
 * Dest Syntax: Rxx32+=vrcmpys(Rss32,Rtt32):<<1:sat:raw:hi
 * Dest Syntax2: Rxx32+=vrcmpys(Rss32,Rtt32):<<1:sat:raw:lo
 * Condition: Rt32 & 1
 *
 */
static
MAP_FUNCTION(M2_vrcmpys_acc_s1)
{
	if (GET_OP_VAL(2) & 1) {
		sprintf(DEST,"R%d:%d+=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:hi",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
	} else {
		sprintf(DEST,"R%d:%d+=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:lo",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
	}
}

/*
 * Source Tag: M2_vrcmpys_s1
 * Source Syntax: Rdd32=vrcmpys(Rss32,Rt32):<<1:sat
 * Dest Syntax: Rdd32=vrcmpys(Rss32,Rtt32):<<1:sat:raw:hi
 * Dest Syntax2: Rdd32=vrcmpys(Rss32,Rtt32):<<1:sat:raw:lo
 * Condition: Rt32 & 1
 *
 */
static
MAP_FUNCTION(M2_vrcmpys_s1)
{
	if (GET_OP_VAL(2) & 1) {
		sprintf(DEST,"R%d:%d=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:hi",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
	} else {
		sprintf(DEST,"R%d:%d=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:lo",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
	}
}

/*
 * Source Tag: M2_vrcmpys_s1rp
 * Source Syntax: Rd32=vrcmpys(Rss32,Rt32):<<1:rnd:sat
 * Dest Syntax: Rd32=vrcmpys(Rss32,Rtt32):<<1:rnd:sat:raw:hi
 * Dest Syntax2: Rd32=vrcmpys(Rss32,Rtt32):<<1:rnd:sat:raw:lo
 * Condition: Rt32 & 1
 *
 */
static
MAP_FUNCTION(M2_vrcmpys_s1rp)
{
	if (GET_OP_VAL(2) & 1) {
		sprintf(DEST,"R%d=vrcmpys(R%d:%d,R%d:%d):<<1:rnd:sat:raw:hi",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
	} else {
		sprintf(DEST,"R%d=vrcmpys(R%d:%d,R%d:%d):<<1:rnd:sat:raw:lo",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
	}
}

/*
 * Source Tag: S2_asr_i_r_rnd_goodsyntax
 * Source Syntax: Rd32=asrrnd(Rs32,#u5)
 * Dest Syntax: Rd32=Rs32
 * Dest Syntax2: Rd32=asr(Rs32,#u5-1):rnd
 * Condition: #u5==0
 *
 */
static
MAP_FUNCTION(S2_asr_i_r_rnd_goodsyntax)
{
	if (GET_OP_VAL(2)==0) {
		sprintf(DEST,"R%d=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
	} else {
		sprintf(DEST,"R%d=asr(R%d,#%d-1):rnd",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
	}
}

/*
 * Source Tag: S2_tableidxb_goodsyntax
 * Source Syntax: Rx32=tableidxb(Rs32,#u4,#U5)
 * Dest Syntax: Rx32=tableidxb(Rs32,#u4,#U5):raw
 *
 */
static
MAP_FUNCTION(S2_tableidxb_goodsyntax)
{
    sprintf(DEST,"R%d=tableidxb(R%d,#%d,#%d):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2),GET_OP_VAL(3));
}

/*
 * Source Tag: S2_tableidxd_goodsyntax
 * Source Syntax: Rx32=tableidxd(Rs32,#u4,#U5)
 * Dest Syntax: Rx32=tableidxd(Rs32,#u4,#U5-3):raw
 *
 */
static
MAP_FUNCTION(S2_tableidxd_goodsyntax)
{
    sprintf(DEST,"R%d=tableidxd(R%d,#%d,#%d-3):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2),GET_OP_VAL(3));
}

/*
 * Source Tag: S2_tableidxh_goodsyntax
 * Source Syntax: Rx32=tableidxh(Rs32,#u4,#U5)
 * Dest Syntax: Rx32=tableidxh(Rs32,#u4,#U5-1):raw
 *
 */
static
MAP_FUNCTION(S2_tableidxh_goodsyntax)
{
    sprintf(DEST,"R%d=tableidxh(R%d,#%d,#%d-1):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2),GET_OP_VAL(3));
}

/*
 * Source Tag: S2_tableidxw_goodsyntax
 * Source Syntax: Rx32=tableidxw(Rs32,#u4,#U5)
 * Dest Syntax: Rx32=tableidxw(Rs32,#u4,#U5-2):raw
 *
 */
static
MAP_FUNCTION(S2_tableidxw_goodsyntax)
{
    sprintf(DEST,"R%d=tableidxw(R%d,#%d,#%d-2):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2),GET_OP_VAL(3));
}

/*
 * Source Tag: Y2_dcfetch
 * Source Syntax: dcfetch(Rs32)
 * Dest Syntax: dcfetch(Rs32+#0)
 *
 */
static
MAP_FUNCTION(Y2_dcfetch)
{
    sprintf(DEST,"dcfetch(R%d+#0)",GET_OP_VAL(0));
}

