adr brk(long size);
int sys_write(int file, adr text, int bytes);
int sys_create(adr path, long mode);
int sys_open(adr path, long flags, long mode);
int sys_read(int file, adr to, int count);
int sys_close(int file);
int sys_fstat(int file, adr stat);
int sys_fork();
int sys_pause();
int sys_sleep(adr t, adr t);
int sys_time(adr t);
int sys_times(adr t);
int sys_exit(int err);
int sys_wait(int proc, adr status, int options, adr t);
int sys_pipe(adr fd);
int sys_kill(int pid, int sig);
int sys_socket(int domain, int type, int protocol);
int sys_setsockopt(int s, int level, int optname, adr optval, int optlen);