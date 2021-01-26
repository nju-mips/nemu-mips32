make_exec_handler(lw) {
  CHECK_ALIGNED_ADDR_AdEL(4, GR_SV + I_SI);
  uint32_t rdata = vaddr_read(GR_SV + I_SI, 4);
  if (!local_cpu.has_exception) { GR_TV = rdata; }
}

make_exec_handler(lb) {
  CHECK_ALIGNED_ADDR_AdEL(1, GR_SV + I_SI);
  uint32_t rdata =
      (int32_t)(int8_t)vaddr_read(GR_SV + I_SI, 1);
  if (!local_cpu.has_exception) { GR_TV = rdata; }
}

make_exec_handler(lbu) {
  CHECK_ALIGNED_ADDR_AdEL(1, GR_SV + I_SI);
  uint32_t rdata = vaddr_read(GR_SV + I_SI, 1);
  if (!local_cpu.has_exception) { GR_TV = rdata; }
}

make_exec_handler(lh) {
  CHECK_ALIGNED_ADDR_AdEL(2, GR_SV + I_SI);
  uint32_t rdata =
      (int32_t)(int16_t)vaddr_read(GR_SV + I_SI, 2);
  if (!local_cpu.has_exception) { GR_TV = rdata; }
}

make_exec_handler(lhu) {
  CHECK_ALIGNED_ADDR_AdEL(2, GR_SV + I_SI);
  uint32_t rdata = vaddr_read(GR_SV + I_SI, 2);
  if (!local_cpu.has_exception) { GR_TV = rdata; }
}

make_exec_handler(sw) {
  CHECK_ALIGNED_ADDR_AdES(4, GR_SV + I_SI);
  vaddr_write(GR_SV + I_SI, 4, GR_TV);
}

make_exec_handler(sh) {
  CHECK_ALIGNED_ADDR_AdES(2, GR_SV + I_SI);
  vaddr_write(GR_SV + I_SI, 2, GR_TV);
}

make_exec_handler(sb) {
  CHECK_ALIGNED_ADDR_AdES(1, GR_SV + I_SI);
  vaddr_write(GR_SV + I_SI, 1, GR_TV);
}

make_exec_handler(swl) {
  uint32_t waddr = GR_SV + I_SI;
  int idx = waddr & 0x3;
  int len = idx + 1;
  uint32_t wdata = GR_TV >> ((3 - idx) * 8);

  vaddr_write((waddr >> 2) << 2, len, wdata);
  if (local_cpu.has_exception) local_cpu.cp0.badvaddr = waddr;
}

make_exec_handler(swr) {
  uint32_t waddr = GR_SV + I_SI;
  int len = 4 - (waddr & 0x3);
  uint32_t wdata = GR_TV;

  vaddr_write(waddr, len, wdata);
}

make_exec_handler(lwl) {
  uint32_t raddr = GR_SV + I_SI;
  int len = (raddr & 0x3) + 1;
  uint32_t rdata = vaddr_read((raddr >> 2) << 2, len);

  if (!local_cpu.has_exception) {
    if (len < 4)
      GR_TV = rdata << ((4 - len) * 8) |
              ((uint32_t)GR_TV << (len * 8)) >> (len * 8);
    else
      GR_TV = rdata;
  } else {
    local_cpu.cp0.badvaddr = raddr;
  }
}

make_exec_handler(lwr) {
  uint32_t raddr = GR_SV + I_SI;
  int idx = raddr & 0x3;
  int len = 4 - idx;
  uint32_t rdata = vaddr_read(raddr, len);
  if (!local_cpu.has_exception) {
    if (len < 4)
      GR_TV = (rdata << idx * 8) >> (idx * 8) |
              ((uint32_t)GR_TV >> (len * 8)) << (len * 8);
    else
      GR_TV = (rdata << idx * 8) >> (idx * 8);
  }
}

make_exec_handler(pref) {}

make_exec_handler(ll) {
  CHECK_ALIGNED_ADDR_AdEL(4, GR_SV + I_SI);
  GR_TV = vaddr_read(GR_SV + I_SI, 4);
}

make_exec_handler(sc) {
  CHECK_ALIGNED_ADDR_AdES(4, GR_SV + I_SI);
  vaddr_write(GR_SV + I_SI, 4, GR_TV);
  if (!local_cpu.has_exception) GR_TV = 1;
}
