make_exec_handler(sll) {
  InstAssert(GR_S == 0);
  GR_DV = GR_TV << I_SA;
}

make_exec_handler(sllv) {
  InstAssert(I_SA == 0);
  GR_DV = GR_TV << (GR_SV & 0x1f);
}

make_exec_handler(sra) {
  InstAssert(GR_S == 0);
  GR_DV = (int32_t)GR_TV >> I_SA;
}

make_exec_handler(srav) {
  InstAssert(I_SA == 0);
  GR_DV = (int32_t)GR_TV >> (GR_SV & 0x1f);
}

make_exec_handler(srl) {
  if ((GR_S & 0x1) == 0x1) {
    /* rotr */
    uint32_t rt_val = GR_TV;
    uint32_t sa = I_SA;
    GR_DV = (rt_val >> sa) | (rt_val << (32 - sa));
  } else {
    InstAssert(GR_S == 0);
    GR_DV = GR_TV >> I_SA;
  }
}

make_exec_handler(srlv) {
  if ((I_SA & 0x1) == 0x1) {
    /* rotrv */
    uint32_t rt_val = GR_TV;
    uint32_t sa = GR_SV & 0x1f;
    GR_DV = (rt_val >> sa) | (rt_val << (32 - sa));
  } else {
    InstAssert(I_SA == 0);
    GR_DV = GR_TV >> (GR_SV & 0x1f);
  }
}
