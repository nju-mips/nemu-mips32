#include <stdlib.h>
#include <SDL/SDL.h>
#include <sys/time.h>
#include <signal.h>
#include "nemu.h"
#include "monitor/monitor.h"

typedef uint32_t (*read_func) (paddr_t addr, int len);
typedef void (*write_func)(paddr_t addr, int len, uint32_t data);

static uint32_t ddr_read(paddr_t addr, int len);
static void ddr_write(paddr_t addr, int len, uint32_t data);
static uint32_t uartlite_read(paddr_t addr, int len);
static void uartlite_write(paddr_t addr, int len, uint32_t data);
static void gpio_write(paddr_t addr, int len, uint32_t data);
static uint32_t vga_read(paddr_t addr, int len);
static void vga_write(paddr_t addr, int len, uint32_t data);
static uint32_t invalid_read(paddr_t addr, int len);
static void invalid_write(paddr_t addr, int len, uint32_t data);

// the memory mapping of mips32-npc
//0x00000000 - 0x00001fff: bram
//0x10000000 - 0x1fffffff: ddr
//0x40000000 - 0x40000fff: gpio-trap
//0x40001000 - 0x40001fff: uartlite
//0x40010000 - 0x4001ffff: vga
struct mmap_region {
  uint32_t start, end;
  read_func read;
  write_func write;
} mmap_table [] = {
  {0x00000000, 0x00001fff, invalid_read, invalid_write},
  {0x10000000, 0x1fffffff, ddr_read, ddr_write},
  {0x40000000, 0x40000fff, invalid_read, gpio_write},
  {0x40001000, 0x40001fff, uartlite_read, uartlite_write},
  {0x40010000, 0x4001ffff, invalid_read, invalid_write},
  {0x50000000, 0x50100000, vga_read, vga_write},
};

#define NR_REGION (sizeof(mmap_table) / sizeof(mmap_table[0]))

uint32_t find_region(vaddr_t addr) {
  int ret = -1;
  for(int i = 0; i < NR_REGION; i++)
    if(addr >= mmap_table[i].start && addr <= mmap_table[i].end) {
      ret = i;
      break;
    }
  Assert(ret != -1, "address(0x%08x) is out of bound", addr);
  return ret;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  int idx = find_region(addr);
  return mmap_table[idx].read(addr - mmap_table[idx].start, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  int idx = find_region(addr);
  return mmap_table[idx].write(addr - mmap_table[idx].start, len, data);
}

#define DDR_SIZE (128 * 1024 * 1024)

uint8_t ddr[DDR_SIZE];

/* Memory accessing interfaces */

#define check_ddr(addr, len) \
  Assert(addr >= 0 && addr < DDR_SIZE && addr + len <= DDR_SIZE, \
      "address(0x%08x) is out side DDR", addr);

static uint32_t ddr_read(paddr_t addr, int len) {
  check_ddr(addr, len);
  return *((uint32_t *)((uint8_t *)ddr + addr)) & (~0u >> ((4 - len) << 3));
}

static void ddr_write(paddr_t addr, int len, uint32_t data) {
  check_ddr(addr, len);
  memcpy((uint8_t *)ddr + addr, &data, len);
}

/* serial port */
#define SERIAL_PORT ((volatile char *)0x40001000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c

#define check_uartlite(addr, len) \
  Assert(addr >= 0 && addr <= STAT, \
      "address(0x%08x) is out side UARTLite", addr); \
  Assert(len == 1, \
      "UARTLite only allow byte read/write");

static uint32_t uartlite_read(paddr_t addr, int len) {
  /* CTRL not yet implemented, only allow byte read/write */
  check_uartlite(addr, len);
  switch (addr) {
    // only allow stat read
    // Rx not supported
    case STAT:
      // ready for Tx
      // no valid Rx
      return 0;
      break;
    default:
      Assert(false, "UARTLite: address(0x%08x) is not readable", addr);
      break;
  }
}

static FILE *serial = NULL;

static void uartlite_write(paddr_t addr, int len, uint32_t data) {
  check_uartlite(addr, len);
  switch (addr) {
    case Tx:
      if (!serial) {
        serial = fopen("serial", "w");
        Assert(serial, "Can not open serial file for writing");
      }
      fprintf(serial, "%c", (char)data);
      break;
    default:
      Assert(false, "UARTLite: address(0x%08x) is not writable", addr);
      break;
  }
}

/* gpio trap */

#define check_gpio(addr, len) \
  Assert(addr == 0, \
      "address(0x%08x) is out side GPIO", addr); \
  Assert(len == 1, \
      "GPIO only allow byte read/write");

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static void gpio_write(paddr_t addr, int len, uint32_t data) {
  check_gpio(addr, len);
  if ((unsigned char)data == 0) {
    printf(ANSI_COLOR_GREEN "HIT GOOD TRAP\n" ANSI_COLOR_RESET);
  }
  else
    printf(ANSI_COLOR_RED "HIT BAD TRAP code: %d\n" ANSI_COLOR_RESET, (unsigned char)data == 0);
  nemu_state = NEMU_END;
  // directly exit, so that we will not print one more commit log
  // which makes it easier for crosschecking.
  exit(0);
}

static uint32_t invalid_read(paddr_t addr, int len) {
  Assert(false, "invalid read at address(0x%08x)", addr);
}

static void invalid_write(paddr_t addr, int len, uint32_t data) {
  Assert(false, "invalid write at address(0x%08x)", addr);
}


/////////////////////////////////////////////////////////////////
//                       vga simulation                        //
/////////////////////////////////////////////////////////////////

#define SCREEN_ROW 400
#define SCREEN_COL 640
#define VMEM_SIZE (4 * SCREEN_ROW * SCREEN_COL)
#define CTR_ROW 200
#define CTR_COL 320
#define VGA_HZ 25
#define TIMER_HZ 100

static uint8_t vmem[VMEM_SIZE];
static bool vmem_dirty = false;
static bool line_dirty[CTR_ROW];

#define check_vga(addr, len) \
  Assert(addr >= 0 && addr < VMEM_SIZE && addr + len <= VMEM_SIZE, \
      "address(0x%08x) is out side DDR", addr);

static uint32_t vga_read(paddr_t addr, int len) {
  check_vga(addr, len);
  return *((uint32_t *)&vmem[addr]) & (~0u >> ((4 - len) << 3));
}

static void vga_write(paddr_t addr, int len, uint32_t data) {
  check_vga(addr, len);
  int line = addr / CTR_COL;
  if(line < CTR_ROW) {
	line_dirty[line] = true;
	vmem_dirty = true;
  }
  memcpy(&vmem[addr], &data, len);
}

static SDL_Surface *real_screen;
static SDL_Surface *screen;
static uint8_t (*pixel_buf) [SCREEN_COL];

static uint64_t jiffy = 0;
static struct itimerval it;

static inline void draw_pixel(int x, int y, uint8_t color_idx) {
	assert(x >= 0 && x < SCREEN_COL && y >= 0 && y < SCREEN_ROW);
	pixel_buf[y][x] = color_idx;
}

static void do_update_screen_graphic_mode() {
	int i, j;
	uint8_t (*vmem_ptr) [CTR_COL] = (void *)vmem;
	SDL_Rect rect;
	rect.x = 0;
	rect.w = CTR_COL * 2;
	rect.h = 2;

	for(i = 0; i < CTR_ROW; i ++) {
		if(line_dirty[i]) {
			for(j = 0; j < CTR_COL; j ++) {
				uint8_t color_idx = vmem_ptr[i][j];
				draw_pixel(2 * j, 2 * i, color_idx);
				draw_pixel(2 * j, 2 * i + 1, color_idx);
				draw_pixel(2 * j + 1, 2 * i, color_idx);
				draw_pixel(2 * j + 1, 2 * i + 1, color_idx);
			}
			rect.y = i * 2;
			SDL_BlitSurface(screen, &rect, real_screen, &rect);
		}
	}
	SDL_Flip(real_screen);
}

static void update_screen() {
	if(vmem_dirty) {
		do_update_screen_graphic_mode();
		vmem_dirty = false;
		memset(line_dirty, false, CTR_ROW);
	}
}

static void device_update(int signum) {
	jiffy ++;
	
	if(jiffy % (TIMER_HZ / VGA_HZ) == 0) {
		update_screen();
	}

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		// If a key was pressed
		if( event.type == SDL_KEYDOWN ) {
		} else if( event.type == SDL_KEYUP ) {
		}

		// If the user has Xed out the window
		if( event.type == SDL_QUIT ) {
			exit(0);
		}
	}

	int ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
	Assert(ret == 0, "Can not set timer");
}

void sdl_clear_event_queue() {
	SDL_Event event;
	while(SDL_PollEvent(&event));
}

void init_sdl() {
  int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
  Assert(ret == 0, "SDL_Init failed");

  real_screen = SDL_SetVideoMode(640, 400, 8, 
	  SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL | SDL_ASYNCBLIT);

  screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 400, 8,
	  real_screen->format->Rmask, real_screen->format->Gmask,
	  real_screen->format->Bmask, real_screen->format->Amask);
  pixel_buf = screen->pixels;

  SDL_WM_SetCaption("NEMU-MIPS32", NULL);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_handler = device_update;
  ret = sigaction(SIGVTALRM, &s, NULL);
  Assert(ret == 0, "Can not set signal handler");

  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}


/////////////////////////////////////////////////////////////////
//                       dev simulation                        //
/////////////////////////////////////////////////////////////////
void init_device() {
  init_sdl();
}
