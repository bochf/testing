#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc,char **argv,char **envp)
{
	int fd;
	fd=open("/tmp/YO1", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno); if (fd>=0) close(fd);
	fd=open("/tmp/YO2", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/tmp/YO3", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/dev/null", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/bb/data/tmp/YO1", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/bb/data/tmp/YO2", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/bb/data/tmp/YO3", O_WRONLY|O_CREAT|O_TRUNC,  0770);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/tmp/YO1", O_RDONLY);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open("/bb/data/tmp/YO1", O_RDONLY);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open64("/bb/data/tmp/YO2", O_RDONLY);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open64("/bb/data/tmp/YO3", O_RDONLY);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open64("/bb/data/act.log", O_RDONLY);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	fd=open64("/bb/data/tmp", O_RDONLY);  printf("FD=%d, errno=%d\n",fd,errno);  if (fd>=0) close(fd);
	unlink("/tmp/YO1");
	unlink("/tmp/YO2");
	unlink("/tmp/YO3");
	unlink("/bb/data/tmp/YO1");
	unlink("/bb/data/tmp/YO2");
	unlink("/bb/data/tmp/YO3");
	return 0;
}
