#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_pwd(void);
void cmd_mkdir(int argc, char** argv);
void cmd_ls(int argc, char** argv);
void cmd_cd(int argc, char** argv);
void cmd_touch(int argc, char** argv);
void cmd_write(int argc, char** argv);
void cmd_cat(int argc, char** argv);
void cmd_cp(int argc, char** argv);
void cmd_mv(int argc, char** argv);
void cmd_rm(int argc, char** argv);
void cmd_chmod(int argc, char** argv);
void cmd_user(int argc, char** argv);
void cmd_whoami(void);
void cmd_stat(int argc, char** argv);

#endif