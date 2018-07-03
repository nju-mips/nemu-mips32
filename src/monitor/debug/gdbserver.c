#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "protocol.h"

#include <stdint.h>
#include <malloc.h>
#include <arpa/inet.h>

uint32_t find_region(vaddr_t addr);

/*
$ message: client --> server:155:
'qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386'

$ message: server --> client:36:
'PacketSize=1000;qXfer:features:read+'


$ message: client --> server:15:
'vMustReplyEmpty'

$ message: server --> client:0:
''


$ message: client --> server:3:
'Hg0'

$ message: server --> client:2:
'OK'


$ message: client --> server:36:
'qXfer:features:read:target.xml:0,ffb'

$ message: server --> client:148:
'l<?xml version="1.0"?><!DOCTYPE target SYSTEM "gdb-target.dtd"><target><architecture>i386</architecture><xi:include href="i386-32bit.xml"/></target>'


$ message: client --> server:40:
'qXfer:features:read:i386-32bit.xml:0,ffb'

$ message: server --> client:481:
'l<?xml version="1.0"?>
<!-- Copyright (C) 2010-2017 Free Software Foundation, Inc.

     Copying and distribution of this file, with or without modification,
     are permitted in any medium without royalty provided the copyright
     notice and this notice are preserved.  -->

<!-- I386 with SSE -->

<!DOCTYPE target SYSTEM "gdb-target.dtd">
<feature name="org.gnu.gdb.i386.32bit">
  <xi:include href="i386-32bit-core.xml"/>
  <xi:include href="i386-32bit-sse.xml"/>
</feature>
'


$ message: client --> server:45:
'qXfer:features:read:i386-32bit-core.xml:0,ffb'

$ message: server --> client:2046:
'm<?xml version="1.0"?>
<!-- Copyright (C) 2010-2015 Free Software Foundation, Inc.

     Copying and distribution of this file, with or without modification,
     are permitted in any medium without royalty provided the copyright
     notice and this notice are preserved.  -->

<!DOCTYPE feature SYSTEM "gdb-target.dtd">
<feature name="org.gnu.gdb.i386.core">
  <flags id="i386_eflags" size="4">
    <field name="CF" start="0" end="0"/>
    <field name="" start="1" end="1"/>
    <field name="PF" start="2" end="2"/>
    <field name="AF" start="4" end="4"/>
    <field name="ZF" start="6" end="6"/>
    <field name="SF" start="7" end="7"/>
    <field name="TF" start="8" end="8"/>
    <field name="IF" start="9" end="9"/>
    <field name="DF" start="10" end="10"/>
    <field name="OF" start="11" end="11"/>
    <field name="NT" start="14" end="14"/>
    <field name="RF" start="16" end="16"/>
    <field name="VM" start="17" end="17"/>
    <field name="AC" start="18" end="18"/>
    <field name="VIF" start="19" end="19"/>
    <field name="VIP" start="20" end="20"/>
    <field name="ID" start="21" end="21"/>
  </flags>

  <reg name="eax" bitsize="32" type="int32"/>
  <reg name="ecx" bitsize="32" type="int32"/>
  <reg name="edx" bitsize="32" type="int32"/>
  <reg name="ebx" bitsize="32" type="int32"/>
  <reg name="esp" bitsize="32" type="data_ptr"/>
  <reg name="ebp" bitsize="32" type="data_ptr"/>
  <reg name="esi" bitsize="32" type="int32"/>
  <reg name="edi" bitsize="32" type="int32"/>

  <reg name="eip" bitsize="32" type="code_ptr"/>
  <reg name="eflags" bitsize="32" type="i386_eflags"/>
  <reg name="cs" bitsize="32" type="int32"/>
  <reg name="ss" bitsize="32" type="int32"/>
  <reg name="ds" bitsize="32" type="int32"/>
  <reg name="es" bitsize="32" type="int32"/>
  <reg name="fs" bitsize="32" type="int32"/>
  <reg name="gs" bitsize="32" type="int32"/>

  <reg name="st0" bitsize="80" type="i387_ext"/>
  <reg name="st1" bitsize="80" type="i387_ext"/>
  <reg name="st2" bitsize="80" type="i387_ext"/>
  <reg name="st3" bitsize'


$ message: client --> server:47:
'qXfer:features:read:i386-32bit-core.xml:7fd,802'

$ message: server --> client:710:
'l="80" type="i387_ext"/>
  <reg name="st4" bitsize="80" type="i387_ext"/>
  <reg name="st5" bitsize="80" type="i387_ext"/>
  <reg name="st6" bitsize="80" type="i387_ext"/>
  <reg name="st7" bitsize="80" type="i387_ext"/>

  <reg name="fctrl" bitsize="32" type="int" group="float"/>
  <reg name="fstat" bitsize="32" type="int" group="float"/>
  <reg name="ftag" bitsize="32" type="int" group="float"/>
  <reg name="fiseg" bitsize="32" type="int" group="float"/>
  <reg name="fioff" bitsize="32" type="int" group="float"/>
  <reg name="foseg" bitsize="32" type="int" group="float"/>
  <reg name="fooff" bitsize="32" type="int" group="float"/>
  <reg name="fop" bitsize="32" type="int" group="float"/>
</feature>
'


$ message: client --> server:44:
'qXfer:features:read:i386-32bit-sse.xml:0,ffb'

$ message: server --> client:2046:
'm<?xml version="1.0"?>
<!-- Copyright (C) 2010-2017 Free Software Foundation, Inc.

     Copying and distribution of this file, with or without modification,
     are permitted in any medium without royalty provided the copyright
     notice and this notice are preserved.  -->

<!DOCTYPE feature SYSTEM "gdb-target.dtd">
<feature name="org.gnu.gdb.i386.32bit.sse">
  <vector id="v4f" type="ieee_single" count="4"/>
  <vector id="v2d" type="ieee_double" count="2"/>
  <vector id="v16i8" type="int8" count="16"/>
  <vector id="v8i16" type="int16" count="8"/>
  <vector id="v4i32" type="int32" count="4"/>
  <vector id="v2i64" type="int64" count="2"/>
  <union id="vec128">
    <field name="v4_float" type="v4f"/>
    <field name="v2_double" type="v2d"/>
    <field name="v16_int8" type="v16i8"/>
    <field name="v8_int16" type="v8i16"/>
    <field name="v4_int32" type="v4i32"/>
    <field name="v2_int64" type="v2i64"/>
    <field name="uint128" type="uint128"/>
  </union>
  <flags id="i386_mxcsr" size="4">
    <field name="IE" start="0" end="0"/>
    <field name="DE" start="1" end="1"/>
    <field name="ZE" start="2" end="2"/>
    <field name="OE" start="3" end="3"/>
    <field name="UE" start="4" end="4"/>
    <field name="PE" start="5" end="5"/>
    <field name="DAZ" start="6" end="6"/>
    <field name="IM" start="7" end="7"/>
    <field name="DM" start="8" end="8"/>
    <field name="ZM" start="9" end="9"/>
    <field name="OM" start="10" end="10"/>
    <field name="UM" start="11" end="11"/>
    <field name="PM" start="12" end="12"/>
    <field name="FZ" start="15" end="15"/>
  </flags>

  <reg name="xmm0" bitsize="128" type="vec128" regnum="32"/>
  <reg name="xmm1" bitsize="128" type="vec128"/>
  <reg name="xmm2" bitsize="128" type="vec128"/>
  <reg name="xmm3" bitsize="128" type="vec128"/>
  <reg name="xmm4" bitsize="128" type="vec128"/>
  <reg name="xmm5" bitsize="128" type="vec128"/>
  <reg name="xmm6" bitsize="128" type="vec128"/>
  <reg name="xmm7" bitsize="128" type="vec128"/>

  <reg name="mxcsr" bitsize="32" typ'


$ message: client --> server:46:
'qXfer:features:read:i386-32bit-sse.xml:7fd,802'

$ message: server --> client:44:
'le="i386_mxcsr" group="vector"/>
</feature>
'


$ message: client --> server:8:
'qTStatus'

$ message: server --> client:0:
''


$ message: client --> server:1:
'?'

$ message: server --> client:13:
'T05thread:01;'


$ message: client --> server:12:
'qfThreadInfo'

$ message: server --> client:2:
'm1'


$ message: client --> server:12:
'qsThreadInfo'

$ message: server --> client:1:
'l'


$ message: client --> server:9:
'qAttached'

$ message: server --> client:1:
'1'
warning:

No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
$ message: client --> server:4:
'Hc-1'

$ message: server --> client:2:
'OK'


$ message: client --> server:1:
'g'

$ message: server --> client:616:
'0000000000000000630600000000000000000000000000000000000000000000f0ff00000200000000f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007f030000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000801f0000'


$ message: client --> server:12:
'qfThreadInfo'

$ message: server --> client:2:
'm1'


$ message: client --> server:12:
'qsThreadInfo'

$ message: server --> client:1:
'l'


$ message: client --> server:7:
'mfff0,1'

$ message: server --> client:2:
'00'


$ message: client --> server:7:
'mfff0,1'

$ message: server --> client:2:
'00'


$ message: client --> server:8:
'mffc0,40'

$ message: server --> client:128:
'00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'


$ message: client --> server:7:
'mfff0,8'

$ message: server --> client:16:
'0000000000000000'

 */


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


typedef char *(*gdb_cmd_handler_t)(char *args);


char *gdb_question(char *args) {
  return "T05thread:01;";
}

char *gdb_xfer_handler(char *args) {
  char *category = args;
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
	} else if(strcmp(file, "mips-32bit.xml") == 0) {
	  return &mips_32bit_xml[offset];
	} else if(strcmp(file, "mips-32bit-core.xml") == 0) {
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
  static char regs[32 * 8 + 25];
  int len = 0;
  for(int i = 0; i < 32; i++) {
	int value = htonl(cpu.gpr[i]);
	len += snprintf(&regs[len], sizeof(regs) - len, "%08x", value);
  }
  return regs;
}

char *gdb_write_registers(char *args) {
  return NULL;
}

char *gdb_set_thread(char *args) {
  return "OK";
}

char *gdb_step(char *args) {
  return NULL;
}

char *gdb_read_memory(char *args) {
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

char *gdb_write_memory(char *args) {
  return NULL;
}

char *gdb_read_register(char *args) {
  static char reg_value[32];

  int reg_no = 0;
  sscanf(args, "%d", &reg_no);
  snprintf(reg_value, sizeof(reg_value), "%08x", cpu.gpr[reg_no]);
  return reg_value;
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

