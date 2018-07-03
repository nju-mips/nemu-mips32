#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "protocol.h"

#include <malloc.h>


static char target_xml[] =
"l<?xml version=\"1.0\"?>"
"<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"
"<target>"
"<architecture>mips</architecture>"
"<xi:include href=\"mips-32bit.xml\"/>"
"</target>";

static char mips_32bit_xml[] = \
"'l<?xml version=\"1.0\"?>"
"<!-- Copyright (C) 2010-2017 Free Software Foundation, Inc."
""
"     Copying and distribution of this file, with or without modification,"
"     are permitted in any medium without royalty provided the copyright"
"     notice and this notice are preserved.  -->"
""
"<!-- MIPS32 with CP0 -->"
""
"<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"
"<feature name=\"org.gnu.gdb.i386.32bit\">"
"  <xi:include href=\"mips-32bit-core.xml\"/>"
"</feature>";

static char mips_32bit_core_xml[] =
"'m<?xml version=\"1.0\"?>"
"<!-- Copyright (C) 2010-2015 Free Software Foundation, Inc."
""
"     Copying and distribution of this file, with or without modification,"
"     are permitted in any medium without royalty provided the copyright"
"     notice and this notice are preserved.  -->"
""
"<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">"
"<feature name=\"org.gnu.gdb.mips.core\">"
"  <reg name=\"zero\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"at\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"v0\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"v1\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"a0\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"a1\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"a2\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"a3\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t0\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t1\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t2\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t3\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t4\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t5\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t6\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t7\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s0\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s1\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s2\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s3\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s4\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s5\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s6\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"s7\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t8\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"t9\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"k0\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"k1\" bitsize=\"32\" type=\"int32\"/>"
"  <reg name=\"gp\" bitsize=\"32\" type=\"data_ptr\"/>"
"  <reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>"
"  <reg name=\"fp\" bitsize=\"32\" type=\"data_ptr\"/>"
"  <reg name=\"ra\" bitsize=\"32\" type=\"int32\"/>"
"</feature>"
;


typedef char *(*gdb_cmd_handler_t)(char *args);


char *gdb_question(char *args) {
  return "T05thread:01;";
}

char *gdb_xfer_handler(char *args) {
  char *category = strtok(args, ":");
  if(!category || strcmp(category, "features") == 0) {
	char *op = strtok(NULL, ":");
	if(!op || strcmp(op, "read") != 0) return NULL;
	
	char *file = strtok(NULL, ":");
	char *offset_s = strtok(NULL, ":");

	int offset = 0;
	sscanf(offset_s, "%x", &offset);

	if(!file) return NULL;
	if(strcmp(file, "target.xml") == 0) {
	  return &target_xml[offset];
	} else if(strcmp(file, "mips.32bit.xml") == 0) {
	  return &mips_32bit_xml[offset];
	} else if(strcmp(file, "mips.32bit.core.xml") == 0) {
	  return &mips_32bit_core_xml[offset];
	} else {
	  return NULL;
	}
  } else {
	return NULL;
  }
}

char *gdb_general_query(char *args) {
  char *kind = strtok(args, ":");
  if(strcmp(kind, "Supported") == 0) {
	return "PacketSize=1000;qXfer:features:read+";
  } else if(strcmp(kind, "MustReplyEmpty")) {
	return "";
  } else if(strcmp(kind, "Xfer")) {
	return gdb_xfer_handler(strtok(NULL, ":"));
  } else if(strcmp(kind, "Attached")) {
	return "1";
  } else if(strcmp(kind, "fThreadInfo")) {
	return "m1";
  } else if(strcmp(kind, "sThreadInfo")) {
	return "l";
  } else if(strcmp(kind, "TStatus")) {
	return "";
  } else {
	return NULL;
  }
}

char *gdb_vCont_handler(char *args) {
  if(strcmp(args, "?") == 0) {
	return "vCont;c;C;s;S";
  } else if(args[0] == ';') {
	return "T05thread:01";
  } else {
	return "";
  }
}

char *gdb_extend_commands(char *args) {
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

char *gdb_continue(char *args) {
  return NULL;
}

char *gdb_read_registers(char *args) {
  return NULL;
}

char *gdb_write_registers(char *args) {
  return NULL;
}

char *gdb_set_thread(char *args) {
  return NULL;
}

char *gdb_step(char *args) {
  return NULL;
}

char *gdb_read_memory(char *args) {
  return NULL;
}

char *gdb_write_memory(char *args) {
  return NULL;
}

char *gdb_read_register(char *args) {
  return NULL;
}

char *gdb_write_register(char *args) {
  return NULL;
}

char *gdb_reset(char *args) {
  return NULL;
}

char *gdb_single_step(char *args) {
  return NULL;
}

char *gdb_write_memory_hex(char *args) {
  return NULL;
}

char *gdb_remove_break_point(char *args) {
  return NULL;
}

char *gdb_insert_break_point(char *args) {
  return NULL;
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
	  char *resp = handler(&data[1]);
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

