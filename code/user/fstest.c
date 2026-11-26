#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
static char data[1024],got[1024];
static void check(int ok,char *msg){if(!ok){printf("FSTEST FAIL: %s\n",msg);exit(1);}}
int main(int argc,char **argv){
  if(argc==2 && !strcmp(argv[1],"reboot")){crash_at(0);exit(1);}
  unlink("ft");unlink("fl");unlink("big");
  int fd=open("ft",O_CREATE|O_RDWR|O_TRUNC);check(fd>=0,"create");
  check(write(fd,"abcdef",6)==6,"write");struct stat st;
  check(fstat(fd,&st)==0 && st.size==6 && st.nlink==1 && st.type==T_FILE,"stat");
  check(link("ft","fl")==0 && link("ft","fl")==-1,"link duplicate rollback");
  check(fstat(fd,&st)==0 && st.nlink==2,"nlink rollback");close(fd);
  fd=open("ft",O_RDWR);check(fd>=0,"reopen");
  check(unlink("ft")==0 && unlink("fl")==0,"unlink open inode");
  check(fstat(fd,&st)==0 && st.nlink==0,"deferred inode");
  check(read(fd,got,6)==6 && memcmp(got,"abcdef",6)==0,"read after unlink");
  check(write(fd,"!",1)==1,"write after unlink");close(fd);
  check(open("ft",O_RDONLY)==-1 && open("fl",O_RDONLY)==-1,"names gone");
  for(int round=0;round<3;round++){
    fd=open("big",O_CREATE|O_RDWR|O_TRUNC);check(fd>=0,"big open");
    for(int b=0;b<16;b++){for(int i=0;i<1024;i++)data[i]=(b*13+i)%251;check(write(fd,data,1024)==1024,"big write");}
    check(fstat(fd,&st)==0 && st.size==16384,"big stat");close(fd);fd=open("big",O_RDONLY);
    for(int b=0;b<16;b++){check(read(fd,got,1024)==1024,"big read");for(int i=0;i<1024;i++)check((uchar)got[i]==(b*13+i)%251,"big compare");}
    close(fd);check(unlink("big")==0,"indirect reclaim");
  }
  check(mkdir("ftdir")==0 && mkdir("ftdir/sub")==0,"nested mkdir");
  check(unlink("ftdir")==-1 && link("ftdir","badlink")==-1,"directory protection");
  check(chdir("ftdir/sub")==0,"chdir");fd=open("file",O_CREATE|O_RDWR);check(fd>=0,"relative create");close(fd);
  check(unlink(".")==-1 && unlink("..")==-1,"dot protection");
  check(unlink("file")==0 && chdir("/")==0,"relative cleanup");
  check(unlink("ftdir/sub")==0 && unlink("ftdir")==0,"dir cleanup");
  check(open((char*)0x3ffffff000UL,O_RDONLY)==-1 && close(99)==-1 && crash_at(6)==-1,"invalid args");
  fd=open("ft",O_CREATE|O_RDWR);check(fd>=0,"fd exhaustion setup");int fds[16],n=0,x;
  while((x=dup(fd))>=0 && n<16)fds[n++]=x;
  check(open("ft",O_RDWR)==-1,"open fd rollback");for(int i=0;i<n;i++)close(fds[i]);close(fd);unlink("ft");
  unlink("badelf");fd=open("badelf",O_CREATE|O_RDWR);check(fd>=0,"bad ELF create");
  check(write(fd,"invalid ELF",11)==11,"bad ELF write");close(fd);
  char *av[]={"badelf",0};for(int i=0;i<12;i++)check(exec("badelf",av)==-1,"ELF rollback");
  check(unlink("badelf")==0,"ELF transaction released");
  fd=open("/sh",O_RDONLY);check(fd>=0,"post error progress");close(fd);
  printf("FSTEST PASS: stat links deferred-unlink indirect-16KiB nested paths errors fd-rollback elf-rollback\n");exit(0);
}
