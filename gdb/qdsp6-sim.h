
static __inline__ int
fromhex (int a) 
{
    if (a >= '0' && a <= '9') 
      return a - '0';
    else if (a >= 'a' && a <= 'f') 
      return a - 'a' + 10;
    else if (a >= 'A' && a <= 'F') 
      return a - 'A' + 10;
    else    
      error ("Reply contains invalid hex digit %d", a);
}

static __inline__ int
hex2bin (const char *hex, char *bin, int count)
{
    int i;  

    for (i = 0; i < count; i++)
    {
        if (hex[0] == 0 || hex[1] == 0)
        {
          /* Hex string is short, or of uneven length.
             Return the count that has been converted so far. */
            return i;
        }
        *bin++ = fromhex (hex[0]) * 16 + fromhex (hex[1]);
        hex += 2;
    }
    return i;
}

#define THIS_TARGET() \
	(current_q6_target ? 					\
		(strcmp(current_q6_target, TARGET_NAME) == 0)	\
		:						\
		0)


extern char *q6targetargsInfo[256];

#ifdef WIN32
#define strdupa strdup
#define sleep(n) Sleep(n*1000)
#endif


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

