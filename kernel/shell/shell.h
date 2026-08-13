#pragma once

void shell_run(void);
void shell_run_command(const char *line);
const char *shell_get_cwd(void);
void shell_thread_entry(void);

void print_prompt(void);
