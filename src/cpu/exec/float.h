make_exec_handler(mfc1) { GR_TV = local_cpu.fpr32i[ops->fs]; }

make_exec_handler(cfc1) {
  uint32_t fs = ops->fs;
  fcsr_t *rt = (fcsr_t *)&GR_TV;
  rt->val = 0;
  if (fs == 0) {
    InstAssert(0);
  } else if (fs == 25) {
    rt->fcc1_7 = local_cpu.fcsr.fcc1_7;
    rt->fcc0 = local_cpu.fcsr.fcc0;
  } else if (fs == 26) {
    rt->flags = local_cpu.fcsr.flags;
    rt->causes = local_cpu.fcsr.causes;
  } else if (fs == 28) {
    rt->RM = local_cpu.fcsr.RM;
    rt->flags |= local_cpu.fcsr.fcc0;
    rt->enables = local_cpu.fcsr.enables;
  } else if (fs == 31) {
    *rt = local_cpu.fcsr;
  }
}
make_exec_handler(mfhc1) {
  GR_TV = local_cpu.fpr32i[ops->fs | 1];
}
make_exec_handler(mtc1) { local_cpu.fpr32i[ops->fs] = GR_TV; }
make_exec_handler(ctc1) {
  /* copy a word to fpu control register */
  uint32_t fs = ops->fs;
  uint32_t rt_val = GR_TV;
  fcsr_t *rt = (fcsr_t *)&rt_val;
  if (fs == 25) {
    local_cpu.fcsr.fcc0 = rt_val & 0x1;
    local_cpu.fcsr.fcc1_7 = (rt_val >> 1) & 0x7F;
  } else if (fs == 26) {
    local_cpu.fcsr.flags = rt->flags;
    local_cpu.fcsr.causes = rt->causes;
  } else if (fs == 28) {
    local_cpu.fcsr.RM = rt->RM;
    local_cpu.fcsr.enables = rt->enables;
    local_cpu.fcsr.fs = (rt_val & 0x4) >> 2;
  } else if (fs == 31) {
    local_cpu.fcsr.val = rt_val;
  }
}
make_exec_handler(mthc1) {
  local_cpu.fpr32i[ops->fs | 1] = GR_TV;
}
make_exec_handler(bc1) {
  InstAssert(ops->nd == 0);
  if (ops->tf == 0) {
    /* bc1f */
    if (getFPCondCode(ops->cc2) == 0)
      local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    else
      local_cpu.br_target = local_cpu.pc + 8;
  } else if (ops->tf == 1) {
    /* bc1t */
    if (getFPCondCode(ops->cc2) == 1)
      local_cpu.br_target = local_cpu.pc + (I_SI << 2) + 4;
    else
      local_cpu.br_target = local_cpu.pc + 8;
  }

  prepare_delayslot();
}

make_exec_handler(add_s) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(
      float32_add(make_float32(local_cpu.fpr32i[ops->fs]),
          make_float32(local_cpu.fpr32i[ops->ft]), &status));
}
make_exec_handler(add_d) {
  float_status status = {};
  local_cpu.fpr64i[ops->fd >> 1] = float64_val(
      float64_add(make_float64(local_cpu.fpr64i[ops->fs >> 1]),
          make_float64(local_cpu.fpr64i[ops->ft >> 1]), &status));
}
make_exec_handler(sub_s) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(
      float32_sub(make_float32(local_cpu.fpr32i[ops->fs]),
          make_float32(local_cpu.fpr32i[ops->ft]), &status));
}
make_exec_handler(sub_d) {
  float_status status = {};
  local_cpu.fpr64i[ops->fd >> 1] = float64_val(
      float64_sub(make_float64(local_cpu.fpr64i[ops->fs >> 1]),
          make_float64(local_cpu.fpr64i[ops->ft >> 1]), &status));
}
make_exec_handler(mul_s) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(
      float32_mul(make_float32(local_cpu.fpr32i[ops->fs]),
          make_float32(local_cpu.fpr32i[ops->ft]), &status));
}
make_exec_handler(mul_d) {
  float_status status = {};
  local_cpu.fpr64i[ops->fd >> 1] = float64_val(
      float64_mul(make_float64(local_cpu.fpr64i[ops->fs >> 1]),
          make_float64(local_cpu.fpr64i[ops->ft >> 1]), &status));
}
make_exec_handler(div_s) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(
      float32_div(make_float32(local_cpu.fpr32i[ops->fs]),
          make_float32(local_cpu.fpr32i[ops->ft]), &status));
}
make_exec_handler(div_d) {
  float_status status = {};
  local_cpu.fpr64i[ops->fd >> 1] = float64_val(
      float64_div(make_float64(local_cpu.fpr64i[ops->fs >> 1]),
          make_float64(local_cpu.fpr64i[ops->ft >> 1]), &status));
}

make_exec_handler(sqrt_s) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(float32_sqrt(
      make_float32(local_cpu.fpr32i[ops->fs]), &status));
}
make_exec_handler(sqrt_d) {
  float_status status = {};
  local_cpu.fpr64i[ops->fd >> 1] = float64_val(float64_sqrt(
      make_float64(local_cpu.fpr64i[ops->fs >> 1]), &status));
}

make_exec_handler(abs_s) {
  local_cpu.fpr32i[ops->fd] = float32_val(
      float32_abs(make_float32(local_cpu.fpr32i[ops->fs])));
}
make_exec_handler(abs_d) {
  local_cpu.fpr64i[ops->fd >> 1] = float64_val(
      float64_abs(make_float64(local_cpu.fpr64i[ops->fs >> 1])));
}

make_exec_handler(mov_s) {
  local_cpu.fpr32i[ops->fd] = local_cpu.fpr32i[ops->fs];
}
make_exec_handler(mov_d) {
  local_cpu.fpr64i[ops->fd >> 1] = local_cpu.fpr64i[ops->fs >> 1];
}

make_exec_handler(neg_s) {
  local_cpu.fpr32i[ops->fd] = local_cpu.fpr32i[ops->fs] ^ (1u << 31);
}
make_exec_handler(neg_d) {
  local_cpu.fpr64i[ops->fd >> 1] =
      local_cpu.fpr64i[ops->fs >> 1] ^ (1ull << 63);
}

make_exec_handler(trunc_w_s) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(float32_to_int32(
      make_float32(local_cpu.fpr32i[ops->fs]), &status));
}
make_exec_handler(trunc_w_d) {
  float_status status = {};
  local_cpu.fpr32i[ops->fd] = float32_val(float64_to_int32(
      make_float64(local_cpu.fpr64i[ops->fs >> 1]), &status));
}

make_exec_handler(movci) {
  if (ops->tf == 0) {
    /* movf */
    if (getFPCondCode(ops->cc2) == 0) GR_DV = GR_SV;
  } else if (ops->tf == 1) {
    /* movt */
    if (getFPCondCode(ops->cc2) == 1) GR_DV = GR_SV;
  }
}

make_exec_handler(movcf_s) {
  if (getFPCondCode(ops->cc2) == 0) {
    local_cpu.fpr32i[ops->fd] = local_cpu.fpr32i[ops->fs];
  }
}
make_exec_handler(movcf_d) {
  if (getFPCondCode(ops->cc2) == 0) {
    local_cpu.fpr64i[ops->fd64] = local_cpu.fpr32i[ops->fs64];
  }
}

make_exec_handler(movz_s) {
  if (GR_TV == 0) {
    local_cpu.fpr32i[ops->fd] = local_cpu.fpr32i[ops->fs];
  }
}

make_exec_handler(movz_d) {
  if (GR_TV == 0) {
    local_cpu.fpr64i[ops->fd64] = local_cpu.fpr64i[ops->fs64];
  }
}
make_exec_handler(movn_s) {
  if (GR_TV != 0) {
    *(float *)&local_cpu.fpr32i[ops->fd] =
        *(float *)&local_cpu.fpr32i[ops->fs];
  }
}
make_exec_handler(movn_d) {
  if (GR_TV != 0) {
    *(double *)&local_cpu.fpr32i[ops->fd & ~1] =
        *(double *)&local_cpu.fpr32i[ops->fs & ~1];
  }
}
make_exec_handler(cvt_d_s) {
  *(double *)&local_cpu.fpr32i[ops->fd & ~1] =
      *(float *)&local_cpu.fpr32i[ops->fs];
}
make_exec_handler(cvt_w_s) {
  local_cpu.fpr32i[ops->fd] =
      (int32_t) * (float *)&local_cpu.fpr32i[ops->fs];
}
make_exec_handler(cvt_s_d) {
  *(float *)&local_cpu.fpr32i[ops->fd] =
      *(double *)&local_cpu.fpr32i[ops->fs & ~1];
}
make_exec_handler(cvt_w_d) {
  local_cpu.fpr32i[ops->fd] =
      (int32_t) * (double *)&local_cpu.fpr32i[ops->fs & ~1];
}
make_exec_handler(cvt_s_w) {
  *(float *)&local_cpu.fpr32i[ops->fd] =
      (int32_t)local_cpu.fpr32i[ops->fs];
}
make_exec_handler(cvt_d_w) {
  *(double *)&local_cpu.fpr32i[ops->fd & ~1] =
      (int32_t)local_cpu.fpr32i[ops->fs];
}
make_exec_handler(c_un_s) { setFPCondCode(ops->cc1, 0); }
make_exec_handler(c_un_d) { setFPCondCode(ops->cc1, 0); }
make_exec_handler(c_eq_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] == local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_eq_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] == local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_ueq_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] == local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_ueq_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] == local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_olt_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] < local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_olt_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] < local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_ult_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] < local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_ult_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] < local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_ole_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] <= local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_ole_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] <= local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_ule_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] <= local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_ule_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] <= local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_lt_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] < local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_lt_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] < local_cpu.fpr64f[ops->ft64]);
}
make_exec_handler(c_le_s) {
  setFPCondCode(
      ops->cc1, local_cpu.fpr32f[ops->fs] <= local_cpu.fpr32f[ops->ft]);
}
make_exec_handler(c_le_d) {
  setFPCondCode(ops->cc1,
      local_cpu.fpr64f[ops->fs64] <= local_cpu.fpr64f[ops->ft64]);
}

make_exec_handler(lwc1) {
  CHECK_ALIGNED_ADDR_AdEL(4, GR_SV + I_SI);
  uint32_t rdata = vaddr_read(GR_SV + I_SI, 4);
  if (!local_cpu.has_exception) { local_cpu.fpr32i[GR_T] = rdata; }
}

make_exec_handler(swc1) {
  uint32_t waddr = GR_SV + I_SI;
  vaddr_write(waddr, 4, local_cpu.fpr32i[ops->ft]);
}

make_exec_handler(ldc1) {
  uint32_t raddr = GR_SV + I_SI;
  CHECK_ALIGNED_ADDR_AdEL(8, raddr);

  uint32_t rdata_l = vaddr_read(raddr, 4);
  uint32_t rdata_h = vaddr_read(raddr + 4, 4);
  if (!local_cpu.has_exception) {
    local_cpu.fpr32i[GR_T & ~1] = rdata_l;
    local_cpu.fpr32i[GR_T | 1] = rdata_h;
  }
}

make_exec_handler(sdc1) {
  uint32_t waddr = GR_SV + I_SI;
  CHECK_ALIGNED_ADDR_AdES(8, waddr);
  vaddr_write(waddr, 4, local_cpu.fpr32i[ops->ft & ~1]);
  vaddr_write(waddr + 4, 4, local_cpu.fpr32i[ops->ft | 1]);
}
