#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

int64_t strint(char **ps, bool *overflow) {
	bool is_neg = false;
	int base = 10;
	int64_t ret = 0;
	char *s = *ps;

	if(*s == '0') {
		s ++;
		if(*s == 'x' || *s == 'X') {
			s ++;
			base = 16;
		} else {
			base = 8;
		}
	} else if(*s == '-') {
		s ++;
		is_neg = true;
	}

	while(*s) {
		int ch = *s;

		switch(base) {
			case 16:
				if('0' <= ch && ch <= '9')
					ret = ret * 16 + ch - '0';
				else if('a' <= ch && ch <= 'f')
					ret = ret * 16 + ch + 10 - 'a';
				else if('A' <= ch && ch <= 'F')
					ret = ret * 16 + ch + 10 -'A';
				else
					goto exit_loop;
				break;
			case 8:
			case 10:
				if('0' <= ch && ch < '0' + base) {
					ret = ret * base + ch - '0';
				} else {
					goto exit_loop;
				}
				break;
		}

		if(ret < 0 && (ret != 0x8000000000000000ll || !is_neg) && overflow)
			*overflow = true;

		s++;
	}

exit_loop:

	if(is_neg) ret = -ret;

	*ps = s;

	return ret;
}


static int cmd_b(char *args) {
  if(args == NULL) {
	printf("usage: b addr\n");
  } else {
	char *p = args;
	uint32_t addr = strint(&p, NULL);
	while(cpu.pc != addr && nemu_state != NEMU_END) {
		cpu_exec(1);
	}
  }
  return 0;
}

static int cmd_si(char *args);
int cmd_info(char *args);
static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "b", "Break Point", cmd_b },
  { "si", "Single step", cmd_si}, 
  { "info", "Print information", cmd_info}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args)
{
  unsigned step_num;
  /*if no step number is specified, by default it execute one instruction*/
  if(args==NULL)
  {
    cpu_exec(1);
    return 0;
  }
  if(sscanf(args,"%u",&step_num)>0)
    cpu_exec(step_num);
  else
    printf("Error, you should follow the si command with a number!\n");
  return 0;
}


int cmd_info(char *args)
{
  char subcmd[5];
  if(sscanf(args, "%4s", subcmd) > 0) {
    if(strcmp(subcmd, "r")==0) {
      void print_registers();
	  print_registers();
      return 0;
    }
  }
  printf("Error, illegal subcommand!\nNote" 
      "that, only 'r' is allowed!\n");
  return 1;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
	if(str == NULL) {
		printf("exit nemu\n");
		exit(0);
	}

    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
