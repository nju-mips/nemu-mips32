/* clang-format off */
#if CONFIG_INSTR_PERF
#  define make_label(l) \
     l: instrperf_record(#l, sizeof(#l));
#else
#  define make_label(l) l:
#endif
/* clang-format on */

#define make_entry()
#define make_exit() make_label(exit)

#if CONFIG_DELAYSLOT
#  define make_exec_handler(name) \
    goto inst_end;                \
    make_label(name)
#  define prepare_delayslot() \
    cpu.is_delayslot = true;  \
    cpu.pc += 4;              \
    goto exit;
#else
#  define make_exec_handler(name) \
    cpu.pc += 4;                  \
    goto exit;                    \
    make_label(name)
#  define prepare_delayslot()                             \
    ON_CONFIG(DECODE_CACHE,                               \
        ds = decode_cache_fetch(cpu.pc = cpu.br_target)); \
    goto exit;
#endif

#define InstAssert_Ex(cond)      \
  do {                           \
    if (!(cond)) {               \
      cpu.cp0.badvaddr = cpu.pc; \
      raise_exception(EXC_RI);   \
      goto exit;                 \
    }                            \
  } while (0)

#define InstAssert_Ne(cond) assert(cond)

#define CHECK_ALIGNED_ADDR_Ex(AdEX, align, addr) \
  if (((addr) & (align - 1)) != 0) {             \
    cpu.cp0.badvaddr = addr;                     \
    raise_exception(EXC_##AdEX);                 \
    goto exit;                                   \
  }

#define CHECK_ALIGNED_ADDR_Ne(AdEX, align, addr)         \
  CPUAssert(((addr) & (align - 1)) == 0,                 \
      "address(0x%08x) is unaligned, pc=%08x\n", (addr), \
      cpu.pc)

#if CONFIG_EXCEPTION
#  define InstAssert InstAssert_Ex
#  define CHECK_ALIGNED_ADDR CHECK_ALIGNED_ADDR_Ex
#else
#  define InstAssert InstAssert_Ne
#  define CHECK_ALIGNED_ADDR CHECK_ALIGNED_ADDR_Ne
#endif

#define CHECK_ALIGNED_ADDR_AdEL(align, addr) \
  CHECK_ALIGNED_ADDR(AdEL, align, addr)
#define CHECK_ALIGNED_ADDR_AdES(align, addr) \
  CHECK_ALIGNED_ADDR(AdES, align, addr)

/* some macros for convenience */
#if CONFIG_DECODE_CACHE
#  define ops ds
#else
#  define ops (&inst)
#endif

#define GR_S ops->rs
#define GR_T ops->rt
#define GR_D ops->rd
#define GR_SV cpu.gpr[ops->rs]
#define GR_TV cpu.gpr[ops->rt]
#define GR_DV cpu.gpr[ops->rd]
#define I_SI ops->simm
#define I_UI ops->uimm
#define I_SA ops->shamt
