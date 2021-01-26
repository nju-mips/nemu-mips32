//////////////////////////////////////////////////////////////
//                      unlikely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beq) {
  if (GR_SV == GR_TV)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bne) {
  if (GR_SV != GR_TV)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(blez) {
  InstAssert(GR_T == 0);
  if ((int32_t)GR_SV <= 0)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgtz) {
  if ((int32_t)GR_SV > 0)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bltz) {
  if ((int32_t)GR_SV < 0)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgez) {
  if ((int32_t)GR_SV >= 0)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgezal) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)GR_SV >= 0)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bltzal) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)GR_SV < 0)
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(jal) {
  cpu.gpr[31] = cpu.pc + 8;
  cpu.br_target =
      (cpu.pc & 0xf0000000) | (operands->addr << 2);
#if CONFIG_FUNCTION_TRACE_LOG
  frames_enqueue_call(cpu.pc, cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(jalr) {
  InstAssert(GR_T == 0 && I_SA == 0);
  GR_DV = cpu.pc + 8;
  cpu.br_target = GR_SV;
#if CONFIG_FUNCTION_TRACE_LOG
  frames_enqueue_call(cpu.pc, cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(j) {
  cpu.br_target =
      (cpu.pc & 0xf0000000) | (operands->addr << 2);
  prepare_delayslot();
}

make_exec_handler(jr) {
  InstAssert(GR_T == 0 && GR_D == 0);
  cpu.br_target = GR_SV;
#if CONFIG_FUNCTION_TRACE_LOG
  if (GR_S == R_ra)
    frames_enqueue_ret(cpu.pc, cpu.br_target);
#endif
  prepare_delayslot();
}

//////////////////////////////////////////////////////////////
//                      likely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beql) {
  if (GR_SV == GR_TV) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bnel) {
  if (GR_SV != GR_TV) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(blezl) {
  InstAssert(GR_T == 0);
  if ((int32_t)GR_SV <= 0) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgtzl) {
  if ((int32_t)GR_SV > 0) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bltzl) {
  if ((int32_t)GR_SV < 0) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgezl) {
  if ((int32_t)GR_SV >= 0) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgezall) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)GR_SV >= 0) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bltzall) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)GR_SV < 0) {
    cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}
