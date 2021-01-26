make_exec_handler(sltiu) { GR_TV = GR_SV < I_SI; }
make_exec_handler(slti) { GR_TV = (int32_t)GR_SV < I_SI; }

make_exec_handler(slt) {
  InstAssert(I_SA == 0);
  GR_DV = (int32_t)GR_SV < (int32_t)GR_TV;
}

make_exec_handler(sltu) {
  InstAssert(I_SA == 0);
  GR_DV = GR_SV < GR_TV;
}

make_exec_handler(movn) {
  InstAssert(I_SA == 0);
  if (GR_TV != 0) GR_DV = GR_SV;
}

make_exec_handler(movz) {
  InstAssert(I_SA == 0);
  if (GR_TV == 0) GR_DV = GR_SV;
}
