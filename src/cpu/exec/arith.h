#define R_SIMPLE(name, op, t)     \
  make_exec_handler(name) {       \
    InstAssert(I_SA == 0);        \
    GR_DV = (t)GR_SV op(t) GR_TV; \
  }

R_SIMPLE(or, |, uint32_t)
R_SIMPLE(xor, ^, uint32_t)
R_SIMPLE(and, &, uint32_t)
R_SIMPLE(addu, +, uint32_t)
R_SIMPLE(subu, -, uint32_t)
R_SIMPLE(mul, *, uint32_t)

make_exec_handler(lui) {
  InstAssert(GR_S == 0);
  GR_TV = I_UI << 16;
}

make_exec_handler(addi) {
  // should throw exception
  L64_t ret;
  ret.val =
      (int64_t)(int32_t)GR_SV + (int64_t)(int32_t)I_SI;
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    raise_exception(EXC_OV);
#else
    CPUAssert(
        0, "addi overflow, %08x + %08x\n", GR_SV, I_SI);
#endif
  } else {
    GR_TV = ret.lo;
  }
}

make_exec_handler(addiu) { GR_TV = GR_SV + I_SI; }
make_exec_handler(andi) { GR_TV = GR_SV & I_UI; }
make_exec_handler(ori) { GR_TV = GR_SV | I_UI; }
make_exec_handler(xori) { GR_TV = GR_SV ^ I_UI; }

make_exec_handler(add) {
  InstAssert(I_SA == 0);
  L64_t ret;
  ret.val =
      (int64_t)(int32_t)GR_SV + (int64_t)(int32_t)GR_TV;
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    raise_exception(EXC_OV);
#else
    CPUAssert(
        0, "add overflow, %08x + %08x\n", GR_SV, GR_TV);
#endif
  } else {
    GR_DV = ret.lo;
  }
}

make_exec_handler(sub) {
  InstAssert(I_SA == 0);
  L64_t ret;
  ret.val =
      (int64_t)(int32_t)GR_SV - (int64_t)(int32_t)GR_TV;
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    raise_exception(EXC_OV);
#else
    CPUAssert(
        0, "sub overflow, %08x - %08x\n", GR_SV, GR_TV);
#endif
  } else {
    GR_DV = ret.lo;
  }
}

make_exec_handler(nor) {
  InstAssert(I_SA == 0);
  GR_DV = ~(GR_SV | GR_TV);
}
