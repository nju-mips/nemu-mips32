#ifndef IMPL_CPU
#error "this file can only be included by cpu.c"
#endif

#if CONFIG_MMU_CACHE_PERF
uint64_t mmu_cache_hit = 0;
uint64_t mmu_cache_miss = 0;
#endif

#define MMU_BITS 12

struct mmu_cache_t {
  uint32_t id;
  uint8_t *ptr;
  bool can_write;
};

static struct mmu_cache_t mmu_cache[1 << MMU_BITS];

static inline void clear_mmu_cache() {
  for (int i = 0; i < sizeof(mmu_cache) / sizeof(*mmu_cache); i++) {
    mmu_cache[i].id = 0xFFFFFFFF;
    mmu_cache[i].ptr = NULL;
  }
}

static ALWAYS_INLINE uint32_t mmu_cache_index(vaddr_t vaddr) {
  return (vaddr >> 12) & ((1 << MMU_BITS) - 1);
}

static ALWAYS_INLINE uint32_t mmu_cache_id(vaddr_t vaddr) {
  return (vaddr >> (12 + MMU_BITS));
}

static ALWAYS_INLINE void update_mmu_cache(
    vaddr_t vaddr, paddr_t paddr, device_t *dev, bool can_write) {
#if CONFIG_MMU_CACHE_PERF
  mmu_cache_miss++;
#endif
  if (cpu.has_exception) return;
  if (dev->map) {
    uint32_t idx = mmu_cache_index(vaddr);
    mmu_cache[idx].id = mmu_cache_id(vaddr);
    mmu_cache[idx].ptr = dev->map((paddr & ~0xFFF) - dev->start, 0);
    mmu_cache[idx].can_write = can_write;
    assert(mmu_cache[idx].ptr);
  }
}

