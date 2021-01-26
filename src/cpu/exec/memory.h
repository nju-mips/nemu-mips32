make_exec_handler(lw) {
  CHECK_ALIGNED_ADDR_AdEL(
      4, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 4);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lb) {
  CHECK_ALIGNED_ADDR_AdEL(
      1, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata = (int32_t)(int8_t)vaddr_read(
      cpu.gpr[operands->rs] + operands->simm, 1);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lbu) {
  CHECK_ALIGNED_ADDR_AdEL(
      1, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 1);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lh) {
  CHECK_ALIGNED_ADDR_AdEL(
      2, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata = (int32_t)(int16_t)vaddr_read(
      cpu.gpr[operands->rs] + operands->simm, 2);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lhu) {
  CHECK_ALIGNED_ADDR_AdEL(
      2, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 2);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(sw) {
  CHECK_ALIGNED_ADDR_AdES(
      4, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 4,
      cpu.gpr[operands->rt]);
}

make_exec_handler(sh) {
  CHECK_ALIGNED_ADDR_AdES(
      2, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 2,
      cpu.gpr[operands->rt]);
}

make_exec_handler(sb) {
  CHECK_ALIGNED_ADDR_AdES(
      1, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 1,
      cpu.gpr[operands->rt]);
}

make_exec_handler(swl) {
  uint32_t waddr = cpu.gpr[operands->rs] + operands->simm;
  int idx = waddr & 0x3;
  int len = idx + 1;
  uint32_t wdata = cpu.gpr[operands->rt] >> ((3 - idx) * 8);

  vaddr_write((waddr >> 2) << 2, len, wdata);
  if (cpu.has_exception) cpu.cp0.badvaddr = waddr;
}

make_exec_handler(swr) {
  uint32_t waddr = cpu.gpr[operands->rs] + operands->simm;
  int len = 4 - (waddr & 0x3);
  uint32_t wdata = cpu.gpr[operands->rt];

  vaddr_write(waddr, len, wdata);
}

make_exec_handler(lwl) {
  uint32_t raddr = cpu.gpr[operands->rs] + operands->simm;
  int len = (raddr & 0x3) + 1;
  uint32_t rdata = vaddr_read((raddr >> 2) << 2, len);

  if (!cpu.has_exception) {
    if (len < 4)
      cpu.gpr[operands->rt] =
          rdata << ((4 - len) * 8) |
          ((uint32_t)cpu.gpr[operands->rt] << (len * 8)) >>
              (len * 8);
    else
      cpu.gpr[operands->rt] = rdata;
  } else {
    cpu.cp0.badvaddr = raddr;
  }
}

make_exec_handler(lwr) {
  uint32_t raddr = cpu.gpr[operands->rs] + operands->simm;
  int idx = raddr & 0x3;
  int len = 4 - idx;
  uint32_t rdata = vaddr_read(raddr, len);
  if (!cpu.has_exception) {
    if (len < 4)
      cpu.gpr[operands->rt] =
          (rdata << idx * 8) >> (idx * 8) |
          ((uint32_t)cpu.gpr[operands->rt] >> (len * 8))
              << (len * 8);
    else
      cpu.gpr[operands->rt] =
          (rdata << idx * 8) >> (idx * 8);
  }
}

make_exec_handler(pref) {}

make_exec_handler(ll) {
  CHECK_ALIGNED_ADDR_AdEL(
      4, cpu.gpr[operands->rs] + operands->simm);
  cpu.gpr[operands->rt] =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 4);
}

make_exec_handler(sc) {
  CHECK_ALIGNED_ADDR_AdES(
      4, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 4,
      cpu.gpr[operands->rt]);
  if (!cpu.has_exception) cpu.gpr[operands->rt] = 1;
}


