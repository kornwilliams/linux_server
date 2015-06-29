#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <stdarg.h>

// TODO:
// 1. 启动超时检测
// 2. 多进程监听同一端口
// 3. 反复启动错误汇报
// 4. 修改rlimit, 例如增加NOFILE limit:
//      /* Increase number of fds */
//      struct rlimit r_fd = {65535,65535};
//      if (-1 == setrlimit(RLIMIT_NOFILE, &r_fd)) {
//        LOG_OPER("setrlimit error (setting max fd size)");
//      }
// 5. 定制signal handler
//
// 6. 如下一些option:
//      "--conf \n"
//      "--host \n"
//      "--port \n"
//      "--daemon             Run as a daemon.\n"
//      "--pidfile FILE       Write process ID into FILE.\n"
//      "--noclose            Do not close open file descriptors.\n"
//      "--nochdir            Do not change the current working directory."
//

int SetProcessTitle(int argc, char ** argv, const char * title) {
  // 需要覆盖argv中的所有内容，且不能超过argv各个参数的总长度
  char * end = argv[argc - 1] + strlen(argv[argc - 1]);
  char * p = argv[0];
  const char * q = title;
  while(p < end && *q) {
    *p++ = *q++;
  }

  int size = p - argv[0];
  while(p < end) {
    *p++ = '\0';
  }
  // printf("%s\r\n", argv[0]);
  return size;
}

int main(int argc, char ** argv) {
  void (*fun)() = NULL;
  pid_t pid;
  const int MAX_FILE_LEN = 64;

  char service_name[MAX_FILE_LEN] = "myservice";
  char libfile[MAX_FILE_LEN];

  char para_name;
  while ((para_name = getopt(argc, argv, "l:c:d")) != -1) {
    switch(para_name) {
    case 'l':
      strncpy(service_name, optarg, MAX_FILE_LEN - 1);
      break;
    case 'd':
      printf("daemonize.\r\n");
      break;
    case 'c':
      printf("config file =  %s\r\n", optarg);
      break;
    default:
      fprintf(stderr, "Illegal argument \"%c\"\r\n", para_name);
      return 1;
    }
  }

  snprintf(libfile, MAX_FILE_LEN - 1, "lib%s.so", service_name);

DO_FORK:
  pid = fork();
  if (pid < 0) {
    printf("fork error.\n");
    return -1;
  } else if (pid == 0) {
    // child process
    void * dl = dlopen(libfile, RTLD_LAZY);
    if (dl == NULL) {
      fprintf(stderr, "%s\r\n", dlerror());
      return -1;
    }

    char * err = dlerror();
    if (err != NULL)
    {
      printf("%s\n", err);
      dlclose(dl);
      return -1;
    }

    char child_process_name[MAX_FILE_LEN] = {0};
    snprintf(child_process_name, MAX_FILE_LEN - 1, "%s:worker", service_name);
    SetProcessTitle(argc, argv, child_process_name);

    fun = (void(*)())dlsym(dl, "ServiceEntry");
    if (err != NULL) {
      printf("%s\n", err);
      dlclose(dl);
      return -1;
    }

    (*fun)();
    dlclose(dl);
  } else {
    // parent process
    int status;
    pid_t cpid = waitpid(pid, &status, 0);
    printf("%s pid=%d exit, status=%d.\n", libfile, cpid, status);
    if (!WIFEXITED(status)) {
      goto DO_FORK;
    }
  }
  return 0;
}

