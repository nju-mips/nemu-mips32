make_exec_handler(madd) {
  InstAssert(GR_D == 0 && I_SA == 0);
  L64_t hilo;
  hilo.hi = local_cpu.hi;
  hilo.lo = local_cpu.lo;
  hilo.sval +=
      (int64_t)(int32_t)GR_SV * (int64_t)(int32_t)GR_TV;
  local_cpu.hi = hilo.hi;
  local_cpu.lo = hilo.lo;
}

make_exec_handler(maddu) {
  InstAssert(GR_D == 0 && I_SA == 0);
  L64_t hilo;
  hilo.hi = local_cpu.hi;
  hilo.lo = local_cpu.lo;
  hilo.val += (uint64_t)GR_SV * (uint64_t)GR_TV;
  local_cpu.hi = hilo.hi;
  local_cpu.lo = hilo.lo;
}

make_exec_handler(msub) {
  InstAssert(GR_D == 0 && I_SA == 0);
  L64_t hilo;
  hilo.hi = local_cpu.hi;
  hilo.lo = local_cpu.lo;
  hilo.sval -=
      (int64_t)(int32_t)GR_SV * (int64_t)(int32_t)GR_TV;
  local_cpu.hi = hilo.hi;
  local_cpu.lo = hilo.lo;
}

make_exec_handler(msubu) {
  InstAssert(GR_D == 0 && I_SA == 0);
  L64_t hilo;
  hilo.hi = local_cpu.hi;
  hilo.lo = local_cpu.lo;
  hilo.val -= (uint64_t)GR_SV * (uint64_t)GR_TV;
  local_cpu.hi = hilo.hi;
  local_cpu.lo = hilo.lo;
}

make_exec_handler(mult) {
  InstAssert(GR_D == 0 && I_SA == 0);
  int64_t prod =
      (int64_t)(int32_t)GR_SV * (int64_t)(int32_t)GR_TV;
  local_cpu.lo = (uint32_t)prod;
  local_cpu.hi = (uint32_t)(prod >> 32);
}

make_exec_handler(multu) {
  InstAssert(GR_D == 0 && I_SA == 0);
  uint64_t prod = (uint64_t)GR_SV * (uint64_t)GR_TV;
  local_cpu.lo = (uint32_t)prod;
  local_cpu.hi = (uint32_t)(prod >> 32);
}

make_exec_handler(divide) {
  InstAssert(GR_D == 0 && I_SA == 0);
  local_cpu.lo = (int32_t)GR_SV / (int32_t)GR_TV;
  local_cpu.hi = (int32_t)GR_SV % (int32_t)GR_TV;
}

make_exec_handler(divu) {
  InstAssert(GR_D == 0 && I_SA == 0);
  local_cpu.lo = GR_SV / GR_TV;
  local_cpu.hi = GR_SV % GR_TV;
}

make_exec_handler(mfhi) {
  InstAssert(GR_S == 0 && GR_T == 0 && I_SA == 0);
  GR_DV = local_cpu.hi;
}

make_exec_handler(mthi) {
  InstAssert(GR_T == 0 && GR_D == 0 && I_SA == 0);
  local_cpu.hi = GR_SV;
}

make_exec_handler(mflo) {
  InstAssert(GR_S == 0 && GR_T == 0 && I_SA == 0);
  GR_DV = local_cpu.lo;
}

make_exec_handler(mtlo) {
  InstAssert(GR_T == 0 && GR_D == 0 && I_SA == 0);
  local_cpu.lo = GR_SV;
}
