//////////////////////////////////////////////////////////////
//                      unlikely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beq) {
  if (GR_SV == GR_TV)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bne) {
  if (GR_SV != GR_TV)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(blez) {
  InstAssert(GR_T == 0);
  if ((int32_t)GR_SV <= 0)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgtz) {
  if ((int32_t)GR_SV > 0)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bltz) {
  if ((int32_t)GR_SV < 0)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgez) {
  if ((int32_t)GR_SV >= 0)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgezal) {
  local_cpu.gpr[31] = local_cpu.pc + 8;
  if ((int32_t)GR_SV >= 0)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bltzal) {
  local_cpu.gpr[31] = local_cpu.pc + 8;
  if ((int32_t)GR_SV < 0)
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
  else
    local_cpu.br_target = local_cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(jal) {
  local_cpu.gpr[31] = local_cpu.pc + 8;
  local_cpu.br_target = (local_cpu.pc & 0xf0000000) | (ops->addr << 2);
#if CONFIG_FUNCTION_TRACE_LOG
  frames_enqueue_call(local_cpu.pc, local_cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(jalr) {
  InstAssert(GR_T == 0 && I_SA == 0);
  GR_DV = local_cpu.pc + 8;
  local_cpu.br_target = GR_SV;
#if CONFIG_FUNCTION_TRACE_LOG
  frames_enqueue_call(local_cpu.pc, local_cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(j) {
  local_cpu.br_target = (local_cpu.pc & 0xf0000000) | (ops->addr << 2);
  prepare_delayslot();
}

make_exec_handler(jr) {
  InstAssert(GR_T == 0 && GR_D == 0);
  local_cpu.br_target = GR_SV;
#if CONFIG_FUNCTION_TRACE_LOG
  if (GR_S == R_ra)
    frames_enqueue_ret(local_cpu.pc, local_cpu.br_target);
#endif
  prepare_delayslot();
}

//////////////////////////////////////////////////////////////
//                      likely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beql) {
  if (GR_SV == GR_TV) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(bnel) {
  if (GR_SV != GR_TV) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(blezl) {
  InstAssert(GR_T == 0);
  if ((int32_t)GR_SV <= 0) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(bgtzl) {
  if ((int32_t)GR_SV > 0) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(bltzl) {
  if ((int32_t)GR_SV < 0) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(bgezl) {
  if ((int32_t)GR_SV >= 0) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(bgezall) {
  local_cpu.gpr[31] = local_cpu.pc + 8;
  if ((int32_t)GR_SV >= 0) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}

make_exec_handler(bltzall) {
  local_cpu.gpr[31] = local_cpu.pc + 8;
  if ((int32_t)GR_SV < 0) {
    local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    local_cpu.br_target = local_cpu.pc + 8;
    local_cpu.pc += 4;
  }
}
