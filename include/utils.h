#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <SDL/SDL.h>

/* file */
size_t get_file_size(const char *img_file);
void *read_file(const char *filename);

/* pcap */
#define PCAP_HEADER_MAGIC 0xa1b2cd34
#define PCAP_HEADER_MAJOR 0x02
#define PCAP_HEADER_MINOR 0x04
#define PCAP_HEADER_LINKK_TYPE_ETH 0x1

typedef struct {
  uint32_t magic; /* 0x34cdb2a1 */
  uint16_t major; /* 0x0200 */
  uint16_t minor; /* 0x0400 */
  uint32_t zone;  /* 00000 */
  uint32_t sig_figs; /* 00000 */
  uint32_t snap_len; /* 65535 */
  uint32_t link_type; /* 1 for ethernet */
} pcap_header_t;

typedef struct {
  uint32_t timestamp_hi; /* seconds */
  uint32_t timestamp_lo; /* microseconds */
  uint32_t caplen;
  uint32_t len;
} pcap_packet_header_t;

typedef FILE *pcap_handler;

pcap_handler pcap_open(const char *filename);
int pcap_write(pcap_handler h, const void *data, const int len);
void pcap_flush(pcap_handler h);
int pcap_write_and_flush(pcap_handler h, const void *data, const int len);
void pcap_close(pcap_handler h);

void load_elf_symtab(const char *elf_file);
void elf_symbols_release_memory();
uint32_t find_addr_of_symbol(const char *symbol);
const char *find_symbol_by_addr(uint32_t addr);

/* nat */
void init_nat();
void net_bind_mac_addr(uint8_t mac_addr[6]);
void net_send_data(const uint8_t *data, const int len);
int net_recv_data(uint8_t *to, const int maxlen);

/* console control */
void init_console();
void disable_buffer();
void enable_buffer();
void echo_off();
void echo_on();
void set_cursor(uint32_t x, uint32_t y);
void hide_cursor();
void show_cursor();
int nchars_stdin();
void save_cursor_pos();
void load_cursor_pos();
void init_scr_wh(int *w, int *h);

uint32_t SDLKey_to_scancode(SDL_EventType type, SDLKey key);
const char *SDLKey_to_ascii(SDL_EventType type, SDLKey key);

#endif
