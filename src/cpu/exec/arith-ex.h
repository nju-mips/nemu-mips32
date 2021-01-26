make_exec_handler(clz) {
  if (GR_SV == 0) {
    GR_DV = 32;
  } else {
    GR_DV = __builtin_clz(GR_SV);
  }
}

make_exec_handler(clo) {
  uint32_t in = GR_SV;
  uint32_t cnt = 0;
  uint32_t b = 0x80000000;
  while ((in & b) != 0) {
    cnt++;
    b >>= 1;
  }
  GR_DV = cnt;
}

make_exec_handler(seb) { GR_DV = (int32_t)(int8_t)GR_TV; }
make_exec_handler(seh) { GR_DV = (int32_t)(int16_t)GR_TV; }

make_exec_handler(wsbh) {
  uint32_t rt_val = GR_TV;
  GR_DV = ((rt_val & 0x00FF0000) << 8) |
          ((rt_val & 0xFF000000) >> 8) |
          ((rt_val & 0x000000FF) << 8) |
          ((rt_val & 0x0000FF00) >> 8);
}

make_exec_handler(ins) {
  uint32_t lsb = I_SA;
  uint32_t msb = GR_D;
  assert(lsb <= msb);
  uint32_t rs_val = GR_SV;
  uint32_t rt_val = GR_TV;
  uint32_t mask = (1ull << (msb - lsb + 1)) - 1;

  GR_TV =
      (rt_val & ~(mask << lsb)) | ((rs_val & mask) << lsb);
}

make_exec_handler(ext) {
  uint32_t lsb = I_SA;
  uint32_t msbd = GR_D;
  uint32_t size = msbd + 1;
  uint32_t rs_val = GR_SV;
  uint32_t mask = (1ull << size) - 1;

  GR_TV = ((rs_val & (mask << lsb)) >> lsb);
}
