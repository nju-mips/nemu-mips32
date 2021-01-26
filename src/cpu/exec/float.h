make_exec_handler(mfc1) {
  GR_TV = cpu.fpr32i[operands->fs];
}

make_exec_handler(cfc1) {
  uint32_t fs = operands->fs;
  fcsr_t *rt = (fcsr_t *)&GR_TV;
  rt->val = 0;
  if (fs == 0) {
    InstAssert(0);
  } else if (fs == 25) {
    rt->fcc1_7 = cpu.fcsr.fcc1_7;
    rt->fcc0 = cpu.fcsr.fcc0;
  } else if (fs == 26) {
    rt->flags = cpu.fcsr.flags;
    rt->causes = cpu.fcsr.causes;
  } else if (fs == 28) {
    rt->RM = cpu.fcsr.RM;
    rt->flags |= cpu.fcsr.fcc0;
    rt->enables = cpu.fcsr.enables;
  } else if (fs == 31) {
    *rt = cpu.fcsr;
  }
}
make_exec_handler(mfhc1) {
  GR_TV = cpu.fpr32i[operands->fs | 1];
}
make_exec_handler(mtc1) {
  cpu.fpr32i[operands->fs] = GR_TV;
}
make_exec_handler(ctc1) {
  /* copy a word to fpu control register */
  uint32_t fs = operands->fs;
  uint32_t rt_val = GR_TV;
  fcsr_t *rt = (fcsr_t *)&rt_val;
  if (fs == 25) {
    cpu.fcsr.fcc0 = rt_val & 0x1;
    cpu.fcsr.fcc1_7 = (rt_val >> 1) & 0x7F;
  } else if (fs == 26) {
    cpu.fcsr.flags = rt->flags;
    cpu.fcsr.causes = rt->causes;
  } else if (fs == 28) {
    cpu.fcsr.RM = rt->RM;
    cpu.fcsr.enables = rt->enables;
    cpu.fcsr.fs = (rt_val & 0x4) >> 2;
  } else if (fs == 31) {
    cpu.fcsr.val = rt_val;
  }
}
make_exec_handler(mthc1) {
  cpu.fpr32i[operands->fs | 1] = GR_TV;
}
make_exec_handler(bc1) {
  InstAssert(operands->nd == 0);
  if (operands->tf == 0) {
    /* bc1f */
    if (getFPCondCode(operands->cc2) == 0)
      cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    else
      cpu.br_target = cpu.pc + 8;
  } else if (operands->tf == 1) {
    /* bc1t */
    if (getFPCondCode(operands->cc2) == 1)
      cpu.br_target = cpu.pc + (I_SI << 2) + 4;
    else
      cpu.br_target = cpu.pc + 8;
  }

  prepare_delayslot();
}

make_exec_handler(add_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_add(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(add_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_add(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}
make_exec_handler(sub_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_sub(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(sub_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_sub(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}
make_exec_handler(mul_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_mul(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(mul_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_mul(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}
make_exec_handler(div_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_div(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(div_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_div(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}

make_exec_handler(sqrt_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(float32_sqrt(
      make_float32(cpu.fpr32i[operands->fs]), &status));
}
make_exec_handler(sqrt_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_sqrt(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      &status));
}

make_exec_handler(abs_s) {
  cpu.fpr32i[operands->fd] = float32_val(
      float32_abs(make_float32(cpu.fpr32i[operands->fs])));
}
make_exec_handler(abs_d) {
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_abs(
      make_float64(cpu.fpr64i[operands->fs >> 1])));
}

make_exec_handler(mov_s) {
  cpu.fpr32i[operands->fd] = cpu.fpr32i[operands->fs];
}
make_exec_handler(mov_d) {
  cpu.fpr64i[operands->fd >> 1] =
      cpu.fpr64i[operands->fs >> 1];
}

make_exec_handler(neg_s) {
  cpu.fpr32i[operands->fd] =
      cpu.fpr32i[operands->fs] ^ (1u << 31);
}
make_exec_handler(neg_d) {
  cpu.fpr64i[operands->fd >> 1] =
      cpu.fpr64i[operands->fs >> 1] ^ (1ull << 63);
}

make_exec_handler(trunc_w_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(float32_to_int32(
      make_float32(cpu.fpr32i[operands->fs]), &status));
}
make_exec_handler(trunc_w_d) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(float64_to_int32(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      &status));
}

make_exec_handler(movci) {
  if (operands->tf == 0) {
    /* movf */
    if (getFPCondCode(operands->cc2) == 0) GR_DV = GR_SV;
  } else if (operands->tf == 1) {
    /* movt */
    if (getFPCondCode(operands->cc2) == 1) GR_DV = GR_SV;
  }
}

make_exec_handler(movcf_s) {
  if (getFPCondCode(operands->cc2) == 0) {
    cpu.fpr32i[operands->fd] = cpu.fpr32i[operands->fs];
  }
}
make_exec_handler(movcf_d) {
  if (getFPCondCode(operands->cc2) == 0) {
    cpu.fpr64i[operands->fd64] = cpu.fpr32i[operands->fs64];
  }
}

make_exec_handler(movz_s) {
  if (GR_TV == 0) {
    cpu.fpr32i[operands->fd] = cpu.fpr32i[operands->fs];
  }
}

make_exec_handler(movz_d) {
  if (GR_TV == 0) {
    cpu.fpr64i[operands->fd64] = cpu.fpr64i[operands->fs64];
  }
}
make_exec_handler(movn_s) {
  if (GR_TV != 0) {
    *(float *)&cpu.fpr32i[operands->fd] =
        *(float *)&cpu.fpr32i[operands->fs];
  }
}
make_exec_handler(movn_d) {
  if (GR_TV != 0) {
    *(double *)&cpu.fpr32i[operands->fd & ~1] =
        *(double *)&cpu.fpr32i[operands->fs & ~1];
  }
}
make_exec_handler(cvt_d_s) {
  *(double *)&cpu.fpr32i[operands->fd & ~1] =
      *(float *)&cpu.fpr32i[operands->fs];
}
make_exec_handler(cvt_w_s) {
  cpu.fpr32i[operands->fd] =
      (int32_t) * (float *)&cpu.fpr32i[operands->fs];
}
make_exec_handler(cvt_s_d) {
  *(float *)&cpu.fpr32i[operands->fd] =
      *(double *)&cpu.fpr32i[operands->fs & ~1];
}
make_exec_handler(cvt_w_d) {
  cpu.fpr32i[operands->fd] =
      (int32_t) * (double *)&cpu.fpr32i[operands->fs & ~1];
}
make_exec_handler(cvt_s_w) {
  *(float *)&cpu.fpr32i[operands->fd] =
      (int32_t)cpu.fpr32i[operands->fs];
}
make_exec_handler(cvt_d_w) {
  *(double *)&cpu.fpr32i[operands->fd & ~1] =
      (int32_t)cpu.fpr32i[operands->fs];
}
make_exec_handler(c_un_s) {
  setFPCondCode(operands->cc1, 0);
}
make_exec_handler(c_un_d) {
  setFPCondCode(operands->cc1, 0);
}
make_exec_handler(c_eq_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] == cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_eq_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] ==
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ueq_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] == cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ueq_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] ==
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_olt_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] < cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_olt_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ult_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] < cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ult_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ole_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] <= cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ole_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <=
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ule_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] <= cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ule_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <=
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_lt_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] < cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_lt_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_le_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] <= cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_le_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <=
                         cpu.fpr64f[operands->ft64]);
}

make_exec_handler(lwc1) {
  CHECK_ALIGNED_ADDR_AdEL(4, GR_SV + I_SI);
  uint32_t rdata = vaddr_read(GR_SV + I_SI, 4);
  if (!cpu.has_exception) { cpu.fpr32i[GR_T] = rdata; }
}

make_exec_handler(swc1) {
  uint32_t waddr = GR_SV + I_SI;
  vaddr_write(waddr, 4, cpu.fpr32i[operands->ft]);
}

make_exec_handler(ldc1) {
  uint32_t raddr = GR_SV + I_SI;
  CHECK_ALIGNED_ADDR_AdEL(8, raddr);

  uint32_t rdata_l = vaddr_read(raddr, 4);
  uint32_t rdata_h = vaddr_read(raddr + 4, 4);
  if (!cpu.has_exception) {
    cpu.fpr32i[GR_T & ~1] = rdata_l;
    cpu.fpr32i[GR_T | 1] = rdata_h;
  }
}

make_exec_handler(sdc1) {
  uint32_t waddr = GR_SV + I_SI;
  CHECK_ALIGNED_ADDR_AdES(8, waddr);
  vaddr_write(waddr, 4, cpu.fpr32i[operands->ft & ~1]);
  vaddr_write(waddr + 4, 4, cpu.fpr32i[operands->ft | 1]);
}
