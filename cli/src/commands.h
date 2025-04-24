#ifndef COMMANDS_H
#define COMMANDS_H

#include "config.h"

int cmd_configure(Config *config, int argc, char **argv);
int cmd_list(Config *config, int argc, char **argv);
int cmd_get(Config *config, int argc, char **argv);
int cmd_add(Config *config, int argc, char **argv);
int cmd_update(Config *config, int argc, char **argv);
int cmd_delete(Config *config, int argc, char **argv);
void print_help();

#endif
