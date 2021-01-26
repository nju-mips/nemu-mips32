R_SIMPLE(or, |, uint32_t)
R_SIMPLE(xor, ^, uint32_t)
R_SIMPLE(and, &, uint32_t)
R_SIMPLE(addu, +, uint32_t)
R_SIMPLE(subu, -, uint32_t)
R_SIMPLE(mul, *, uint32_t)

make_exec_handler(lui) {
  InstAssert(operands->rs == 0);
  cpu.gpr[operands->rt] = operands->uimm << 16;
}

make_exec_handler(addi) {
  // should throw exception
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[operands->rs] +
            (int64_t)(int32_t)operands->simm;
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    raise_exception(EXC_OV);
#else
    CPUAssert(0, "addi overflow, %08x + %08x\n",
        cpu.gpr[operands->rs], operands->simm);
#endif
  } else {
    cpu.gpr[operands->rt] = ret.lo;
  }
}

make_exec_handler(addiu) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] + operands->simm;
}

make_exec_handler(andi) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] & operands->uimm;
}

make_exec_handler(ori) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] | operands->uimm;
}

make_exec_handler(xori) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] ^ operands->uimm;
}

make_exec_handler(add) {
  InstAssert(operands->shamt == 0);
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[operands->rs] +
            (int64_t)(int32_t)cpu.gpr[operands->rt];
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    raise_exception(EXC_OV);
#else
    CPUAssert(0, "add overflow, %08x + %08x\n",
        cpu.gpr[operands->rs], cpu.gpr[operands->rt]);
#endif
  } else {
    cpu.gpr[operands->rd] = ret.lo;
  }
}

make_exec_handler(sub) {
  InstAssert(operands->shamt == 0);
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[operands->rs] -
            (int64_t)(int32_t)cpu.gpr[operands->rt];
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    raise_exception(EXC_OV);
#else
    CPUAssert(0, "sub overflow, %08x - %08x\n",
        cpu.gpr[operands->rs], cpu.gpr[operands->rt]);
#endif
  } else {
    cpu.gpr[operands->rd] = ret.lo;
  }
}

make_exec_handler(nor) {
  InstAssert(operands->shamt == 0);
  cpu.gpr[operands->rd] =
      ~(cpu.gpr[operands->rs] | cpu.gpr[operands->rt]);
}
