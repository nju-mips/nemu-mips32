int init_cpu();
int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_cpu();

  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
