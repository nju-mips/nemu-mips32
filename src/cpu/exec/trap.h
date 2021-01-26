make_exec_handler(teq) {
  if ((int32_t)GR_SV == (int32_t)GR_TV) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(teqi) {
  if ((int32_t)GR_SV == I_SI) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tge) {
  if ((int32_t)GR_SV >= (int32_t)GR_TV) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tgei) {
  if ((int32_t)GR_SV >= I_SI) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tgeiu) {
  if (GR_SV >= I_SI) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tgeu) {
  if (GR_SV >= GR_TV) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tlt) {
  if ((int32_t)GR_SV < (int32_t)GR_TV) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tlti) {
  if ((int32_t)GR_SV < I_SI) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tltiu) {
  if (GR_SV < I_SI) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tltu) {
  if (GR_SV < GR_TV) { raise_exception(EXC_TRAP); }
}

make_exec_handler(tne) {
  if ((int32_t)GR_SV != (int32_t)GR_TV) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tnei) {
  if ((int32_t)GR_SV != I_SI) { raise_exception(EXC_TRAP); }
}
