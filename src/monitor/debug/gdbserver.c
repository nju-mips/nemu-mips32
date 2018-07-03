#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "protocol.h"

#include <stdint.h>
#include <malloc.h>
#include <arpa/inet.h>

uint32_t find_region(vaddr_t addr);
void cpu_exec(uint64_t);


static char target_xml[] =
"l<?xml version=\"1.0\"?>"
"<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"
"<target>"
"<architecture>mips</architecture>"
"<xi:include href=\"mips-32bit.xml\"/>"
"</target>";

static char mips_32bit_xml[] = \
"l<?xml version=\"1.0\"?>\n"
"<!-- Copyright (C) 2010-2017 Free Software Foundation, Inc.\n"
"\n"
"     Copying and distribution of this file, with or without modification,\n"
"     are permitted in any medium without royalty provided the copyright\n"
"     notice and this notice are preserved.  -->\n"
"\n"
"<!-- MIPS32 with CP0 -->\n"
"\n"
"<!DOCTYPE target SYSTEM \"gdb-target.dtd\">\n"
"<feature name=\"org.gnu.gdb.mips.32bit\">\n"
"  <xi:include href=\"mips-32bit-core.xml\"/>\n"
"</feature>";

static char mips_32bit_core_xml[] =
"l<?xml version=\"1.0\"?>\n"
"<!-- Copyright (C) 2010-2015 Free Software Foundation, Inc.\n"
"\n"
"     Copying and distribution of this file, with or without modification,\n"
"     are permitted in any medium without royalty provided the copyright\n"
"     notice and this notice are preserved.  -->\n"
"\n"
"<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">\n"
"<feature name=\"org.gnu.gdb.mips.core\">\n"
"  <reg name=\"zero\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"at\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"v0\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"v1\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"a0\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"a1\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"a2\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"a3\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t0\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t1\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t2\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t3\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t4\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t5\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t6\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t7\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s0\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s1\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s2\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s3\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s4\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s5\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s6\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"s7\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t8\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"t9\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"k0\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"k1\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"gp\" bitsize=\"32\" type=\"data_ptr\"/>\n"
"  <reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>\n"
"  <reg name=\"fp\" bitsize=\"32\" type=\"data_ptr\"/>\n"
"  <reg name=\"ra\" bitsize=\"32\" type=\"int32\"/>\n"

"  <reg name=\"sr\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"lo\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"hi\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"bad\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"cause\" bitsize=\"32\" type=\"int32\"/>\n"
"  <reg name=\"pc\" bitsize=\"32\" type=\"int32\"/>\n"
"</feature>\n"
;


typedef char *(*gdb_cmd_handler_t)(char *args, int arglen);


char *gdb_question(char *args, int arglen) {
  return "T05thread:01;";
}

char *gdb_xfer_handler(char *args) {
  char *category = args;
  if(!category || strcmp(category, "features") == 0) {
	char *op = strtok(NULL, ":");
	if(!op || strcmp(op, "read") != 0) return "";
	
	char *file = strtok(NULL, ":");
	char *offset_s = strtok(NULL, ":");

	int offset = 0;
	sscanf(offset_s, "%x", &offset);

	if(!file) return "";
	if(strcmp(file, "target.xml") == 0) {
	  return &target_xml[offset];
	} else if(strcmp(file, "mips-32bit.xml") == 0) {
	  return &mips_32bit_xml[offset];
	} else if(strcmp(file, "mips-32bit-core.xml") == 0) {
	  return &mips_32bit_core_xml[offset];
	} else {
	  return "";
	}
  } else {
	return "";
  }
}

char *gdb_general_query(char *args, int arglen) {
  char *kind = strtok(args, ":");
  if(strcmp(kind, "Supported") == 0) {
	return "PacketSize=1000;qXfer:features:read+";
  } else if(strcmp(kind, "MustReplyEmpty") == 0) {
	return "";
  } else if(strcmp(kind, "Xfer") == 0) {
	return gdb_xfer_handler(strtok(NULL, ":"));
  } else if(strcmp(kind, "Attached") == 0) {
	return "1";
  } else if(strcmp(kind, "fThreadInfo") == 0) {
	return "m1";
  } else if(strcmp(kind, "sThreadInfo") == 0) {
	return "l";
  } else if(strcmp(kind, "TStatus") == 0) {
	return "";
  } else {
	return "";
  }
}

char *gdb_vCont_handler(char *args) {
  if(strcmp(args, "?") == 0) {
	return "vCont;c;C;s;S";
  } else if(args[0] == ';') {
	char action = 0;
	int thread = 0;
	while(args) {
	  args ++;
	  sscanf(args, "%c:%d", &action, &thread);

	  switch(action) {
		case 'c': cpu_exec(-1); cpu.pc -= 8; break;
		case 's': cpu_exec(1); break;
	  }

	  args = strchr(args, ';');
	}
	return "T05thread:01;";
  } else {
	return "";
  }
}

char *gdb_extend_commands(char *args, int arglen) {
  if(strcmp(args, "MustReplyEmpty") == 0) {
	return "";
  } else if(strncmp(args, "Cont", 4) == 0) {
	return gdb_vCont_handler(args + 4);
  } else if(strncmp(args, "File", 4) == 0) {
	return "";
  } else {
	return "";
  }
}

char *gdb_continue(char *args, int arglen) {
  return "";
}

char *gdb_read_registers(char *args, int arglen) {
  static char regs[32 * 8 + 25];
  int len = 0;
  for(int i = 0; i < 32; i++) {
	int value = htonl(cpu.gpr[i]);
	len += snprintf(&regs[len], sizeof(regs) - len, "%08x", value);
  }
  return regs;
}

char *gdb_write_registers(char *args, int arglen) {
  return "";
}

char *gdb_set_thread(char *args, int arglen) {
  return "OK";
}

char *gdb_step(char *args, int arglen) {
  return "";
}

char *gdb_read_memory(char *args, int arglen) {
  // m<addr>,len
  static char mem[4096];

  uint32_t addr = 0, size = 0;
  sscanf(args, "%x,%x", &addr, &size);

  int len = 0;
  for(int i = 0; i < size; i++) {
	if(find_region(addr) == -1) {
	  len += snprintf(&mem[len], sizeof(mem) - len, "00");
	} else {
	  int data = vaddr_read(addr + i, 1);
	  len += snprintf(&mem[len], sizeof(mem) - len,
		  "%02x", data & 0XFF);
	}
  }
  return mem;
}

char *gdb_write_memory(char *args, int arglen) {
  // M<addr>,len:<HEX>
  uint32_t addr = 0, size = 0;
  sscanf(args, "%x,%x:", &addr, &size);

  char *hex = strchr(args, ':');
  for(int i = 0; i < size; i++) {
	if(find_region(addr + i) == -1) {
	  // do nothing
	} else if(hex != NULL) {
	  int data = 0;
	  sscanf(hex + 1, "%02x", &data);
	  vaddr_write(addr + i, 1, data);

	  if(hex[1] == 0 || hex[2] == 0)
		hex = NULL;
	  else
		hex += 2;
	}
  }
  return "OK";
}

char *gdb_read_register(char *args, int arglen) {
  static char reg_value[32];

  int reg_no = 0;
  sscanf(args, "%x", &reg_no);
  if(reg_no < 32) {
	snprintf(reg_value, sizeof(reg_value),
		"%08x", htonl(cpu.gpr[reg_no]));
  } else {
	int value = 0;
	switch(reg_no) {
	  case 0x20: value = 0; break;
	  case 0x21: value = cpu.lo; break;
	  case 0x22: value = cpu.hi; break;
	  case 0x23: value = cpu.cp0[CP0_BADVADDR][0]; break;
	  case 0x24: value = cpu.cp0[CP0_CAUSE][0]; break;
	  case 0x25: value = cpu.pc; break;
	  default: value = 0; break;
	}
	snprintf(reg_value, sizeof(reg_value), "%08x", htonl(value));
  }
  return reg_value;
}

char *gdb_write_register(char *args, int arglen) {
  return "";
}

char *gdb_reset(char *args, int arglen) {
  return "";
}

char *gdb_single_step(char *args, int arglen) {
  return "";
}

char *gdb_detach(char *args, int arglen) {
  return "OK";
}

char *gdb_write_memory_hex(char *args, int arglen) {
  // X<addr>,len:<BIN>
  uint32_t addr = 0, size = 0;
  sscanf(args, "%x,%x:", &addr, &size);

  char *hex = strchr(args, ':');
  printf("write memory hex:%08x: '", addr);
  for(char *p = hex + 1; p < args + arglen; p ++)
	printf("%02hhx ", *p);
  printf("'\n");

  for(int i = 0; i < size; i++) {
	if(find_region(addr + i) == -1) {
	  // do nothing
	} else if(hex != NULL) {
	  vaddr_write(addr + i, 1, hex[1]);

	  if(hex > args + arglen)
		hex = NULL;
	  else
		hex += 1;
	}
  }
  return "OK";
}

char *gdb_remove_break_point(char *args, int arglen) {
  return "";
}

char *gdb_insert_break_point(char *args, int arglen) {
  int type = 0, addr = 0, kind = 0;
  sscanf(args, "%x,%x,%x", &type, &addr, &kind);
  vaddr_write(addr, 4, 0x0005000d);
  // let gdb to maintain the breakpoints, :)
  return "";
}

static gdb_cmd_handler_t handlers[128] = {
	['?'] = gdb_question,
	['c'] = gdb_continue,
	['g'] = gdb_read_registers,
	['G'] = gdb_write_registers,
	['H'] = gdb_set_thread,
	['i'] = gdb_step,
	['m'] = gdb_read_memory,
	['M'] = gdb_write_memory,
	['D'] = gdb_detach,
	['p'] = gdb_read_register,
	['P'] = gdb_write_register,
	['q'] = gdb_general_query,
	['r'] = gdb_reset,
	['R'] = gdb_reset,
	['s'] = gdb_single_step,
	['v'] = gdb_extend_commands,
	['X'] = gdb_write_memory_hex,
	['z'] = gdb_remove_break_point,
	['Z'] = gdb_insert_break_point,
};

void gdb_server_mainloop(int port) {
  struct gdb_conn *gdb = gdb_server_start(port);
  while(1) {
	size_t size = 0;
	char *data = (void*)gdb_recv(gdb, &size);

	gdb_cmd_handler_t handler = handlers[(int)data[0]];
	if(handler) {
	  char *resp = handler(&data[1], size);
	  if(resp) {
		gdb_send(gdb, (void*)resp, strlen(resp));
	  } else {
		gdb_send(gdb, (void*)"", 0);
	  }
	  free(data);
	} else {
	  gdb_send(gdb, (void*)"", 0);
	  free(data);
	}
  }
}

