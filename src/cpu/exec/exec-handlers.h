/*
 * ABS.D ADD.D ADDIU ADD.S ADDU AND ANDI BC1F BC1T BEQ
 * BGEZ BGEZAL BGTZ BLEZ BLTZ BLTZAL BNE BREAK C.EQ.D
 * C.EQ.S CFC1 C.LE.D C.LE.S C.LT.D C.LT.S CLZ CTC1
 * C.ULE.D C.ULE.S C.ULT.D C.ULT.S C.UN.D C.UN.S
 * CVT.D.S CVT.D.W CVT.S.D CVT.S.W DIV DIV.D DIV.S DIVU
 * EXT INS J JAL JALR JR LB LBU LDC1 LH LHU LL LUI LW
 * LWC1 LWL LWR MADD MADDU MFC1 MFHC1 MFHI MFLO MOV.D
 * MOVF MOVF.D MOVF.S MOVN MOVN.D MOVN.S MOV.S MOVT
 * MOVT.D MOVT.S MOVZ MOVZ.S MSUB MTC1 MTHC1 MTHI MTLO
 * MUL MUL.D MUL.S MULT MULTU NEG.D NOR OR ORI PREF RDHWR
 * ROR RORV SB SC SDC1 SEB SEH SH SLL SLLV SLT SLTI SLTIU
 * SLTU SQRT.D SQRT.S SRA SRAV SRL SRLV SUB.D SUB.S SUBU
 * SW SWC1 SWL SWR SYNC SYSCALL TEQ TRUNC.W.D TRUNC.W.S
 * WSBH XOR XORI
 * */

/* clang-format off */
/* R-type */
static const void *special_table[64] = {
    /* 0x00 */ &&sll, &&movci, &&srl, &&sra,
    /* 0x04 */ &&sllv, &&inv, &&srlv, &&srav,
    /* 0x08 */ &&jr, &&jalr, &&movz, &&movn,
    /* 0x0c */ &&syscall, &&breakpoint, &&inv, &&sync,
    /* 0x10 */ &&mfhi, &&mthi, &&mflo, &&mtlo,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&mult, &&multu, &&divide, &&divu,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&add, &&addu, &&sub, &&subu,
    /* 0x24 */ &&and, && or, &&xor, &&nor,
    /* 0x28 */ &&inv, &&inv, &&slt, &&sltu,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&tge, &&tgeu, &&tlt, &&tltu,
    /* 0x34 */ &&teq, &&inv, &&tne, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

static const void *special2_table[64] = {
    /* 0x00 */ &&madd, &&maddu, &&mul, &&inv,
    /* 0x04 */ &&msub, &&msubu, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&clz, &&clo, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

static const void *special3_table[64] = {
    /* 0x00 */ &&ext, &&inv, &&mul, &&inv,
    /* 0x04 */ &&ins, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&exec_bshfl, &&inv, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

/* shamt */
static const void *bshfl_table[64] = {
    /* 0x00 */ &&inv, &&inv, &&wsbh, &&inv,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&seb, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&seh, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
};

/* I-type */
static const void *regimm_table[64] = {
    /* 0x00 */ &&bltz, &&bgez, &&bltzl, &&bgezl,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&tgei, &&tgeiu, &&tlti, &&tltiu,
    /* 0x0c */ &&teqi, &&inv, &&tnei, &&inv,
    /* 0x10 */ &&bltzal, &&bgezal, &&bltzall, &&bgezall,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&synci,
};

/* R-type */
static const void *cop0_table_rs[32] = {
    /* 0x00 */ &&mfc0, &&inv, &&inv, &&inv,
    /* 0x04 */ &&mtc0, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv,  &&inv, &&inv, &&inv,
};

static const void *cop0_table_func[64] = {
    /* 0x00 */ &&inv,  &&tlbr, &&tlbwi, &&inv,
    /* 0x04 */ &&inv,  &&inv,  &&tlbwr, &&inv,
    /* 0x08 */ &&tlbp, &&inv,  &&inv,   &&inv,
    /* 0x0c */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x10 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x14 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x18 */ &&eret, &&inv,  &&inv,   &&inv,
    /* 0x1c */ &&inv,  &&inv,  &&inv,   &&inv,

    /* 0x20 */ &&wait,  &&inv,  &&inv,   &&inv,
    /* 0x24 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x28 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x2c */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x30 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x34 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x38 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x3c */ &&inv,  &&inv,  &&inv,   &&inv,
};

static const void *cop1_table_rs[32] = {
    /* 0x00 */ &&mfc1, &&inv, &&cfc1, &&mfhc1,
    /* 0x04 */ &&mtc1, &&inv, &&ctc1, &&mthc1,
    /* 0x08 */ &&bc1, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
};

static const void *cop1_table_rs_S[64] = {
    /* 0x00 */ &&add_s, &&sub_s, &&mul_s, &&div_s,
    /* 0x04 */ &&sqrt_s, &&abs_s, &&mov_s, &&neg_s,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&trunc_w_s, &&inv, &&inv,
    /* 0x10 */ &&inv, &&movcf_s, &&movz_s, &&movn_s,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,

    /* 0x20 */ &&inv, &&cvt_d_s, &&inv, &&inv,
    /* 0x24 */ &&inv, &&cvt_w_s, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&c_un_s, &&c_eq_s, &&c_ueq_s,
    /* 0x34 */ &&c_olt_s, &&c_ult_s, &&c_ole_s, &&c_ule_s,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&c_lt_s, &&inv, &&c_le_s, &&inv,
};

static const void *cop1_table_rs_D[64] = {
    /* 0x00 */ &&add_d, &&sub_d, &&mul_d, &&div_d,
    /* 0x04 */ &&sqrt_d, &&abs_d, &&mov_d, &&neg_d,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&trunc_w_d, &&inv, &&inv,
    /* 0x10 */ &&inv, &&movcf_d, &&movz_d, &&movn_d,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,

    /* 0x20 */ &&cvt_s_d, &&inv, &&inv, &&inv,
    /* 0x24 */ &&inv, &&cvt_w_d, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&c_un_d, &&c_eq_d, &&c_ueq_d,
    /* 0x34 */ &&c_olt_d, &&c_ult_d, &&c_ole_d, &&c_ule_d,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&c_lt_d, &&inv, &&c_le_d, &&inv,
};

static const void *cop1_table_rs_W[64] = {
    /* 0x00 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,

    /* 0x20 */ &&cvt_s_w, &&cvt_d_w, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

/* I-type */
static const void *opcode_table[64] = {
    /* 0x00 */ &&exec_special, &&exec_regimm, &&j, &&jal,
    /* 0x04 */ &&beq, &&bne, &&blez, &&bgtz,
    /* 0x08 */ &&addi, &&addiu, &&slti, &&sltiu,
    /* 0x0c */ &&andi, &&ori, &&xori, &&lui,
    /* 0x10 */ &&exec_cop0, &&exec_cop1, &&inv, &&inv,
    /* 0x14 */ &&beql, &&bnel, &&blezl, &&bgtzl,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&exec_special2, &&inv, &&inv, &&exec_special3,
    /* 0x20 */ &&lb, &&lh, &&lwl, &&lw,
    /* 0x24 */ &&lbu, &&lhu, &&lwr, &&inv,
    /* 0x28 */ &&sb, &&sh, &&swl, &&sw,
    /* 0x2c */ &&inv, &&inv, &&swr, &&cache,
    /* 0x30 */ &&ll, &&lwc1, &&inv, &&pref,
    /* 0x34 */ &&inv, &&ldc1, &&inv, &&inv,
    /* 0x38 */ &&sc, &&swc1, &&inv, &&inv,
    /* 0x3c */ &&inv, &&sdc1, &&inv, &&inv,
};

