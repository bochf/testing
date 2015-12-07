/* if-list.c:  Demo extract network interfaces, IPs, Ethernet addresses for AIX {DRQS 54952620 <go>}
 *
 * To build:
 	/bb/util/version12-032014/usr/vac/bin/xlc -qarch=pwr6 -qtune=pwr7 -qdfp -qstrict -o if-list if-list.c
 *
 * Author:  Eugene Schulman <ESchulman6@bloomberg.net>, R&D Systems Performance (G325)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <libgen.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#define _Q(x) #x
#define QUOTE(x) _Q(x)

char *progname;


void warn_t(struct timespec * t,const char *fmt, ...)
{
	va_list ap;
	char tmbuf[32];
	strftime(tmbuf,sizeof(tmbuf),"%H:%M:%S",localtime(&(t->tv_sec)));
	fprintf(stderr,"%s.%03u %s: ",tmbuf, t->tv_nsec/1000000L, progname);
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fflush(stderr);
}


/* DIE: Exit with a string and an exit code */
/* Usage:  die(EXIT_SUCCESS,"test %d",2); */
#ifndef die
# define die(...)  _die(__FILE__,__LINE__,"die",__VA_ARGS__)
#endif
void _die(const char *file, int line, const char *fxn, int e, const char *fmt, ...)
{
	va_list ap;
	fprintf(stderr,"%s: FATAL: ",progname);
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fprintf(stderr,"\nStopped at %s:%d\n",file,line);
	fflush(stderr);
	exit(e);
}


int main(int argc, char **argv, char **envp)
{
	int s;
	int e;
	int ifbufsz;
	struct ifconf conf;
	struct ifreq *ifbuf, *n;
	struct sockaddr_in *in;
	char ntop_buf[INET6_ADDRSTRLEN];
	char *ntop_result;
	size_t ifreqsz;
	int i;
	size_t n_inc;
	uchar_t link_type;
	progname=strdup(basename(argv[0]));
	
	if ((s=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
		die (EXIT_FAILURE, "socket() failed: %s, error code %d\n", strerror(errno), errno);

	if ((e=ioctl(s, SIOCGSIZIFCONF, &ifbufsz)) < 0)
		die (EXIT_FAILURE, "Failed to get required size of interface buffer: %s, error code %d\n", strerror(errno), errno);

	if ((ifbuf=(struct ifreq *)malloc(ifbufsz)) < 0)
		die (EXIT_FAILURE, "Failed to allocate memory for interface listing: %s, error code %d\n", strerror(errno), errno);
	
	conf.ifc_len=ifbufsz;
	conf.ifc_buf=(void*)ifbuf;
	if ((e=ioctl(s, SIOCGIFCONF, &conf)) < 0)
		die (EXIT_FAILURE, "Failed to get interface listing: %s, error code %d\n", strerror(errno), errno);

//DEBUG FILE *w=fopen("./if-list.raw","w");
//DEBUG fwrite(ifbuf,ifbufsz,1,w);
//DEBUG fclose(w);

	for (i=0, n=ifbuf;  n < ifbuf + conf.ifc_len && n->ifr_name[0] && n->ifr_addr.sa_family;  ++i, (void*)n += n_inc)
	{
		//DEBUG  printf("PRINT0,  n-ifbuf=%04X\n", (size_t)((void*)n-(void*)ifbuf));
		n_inc=sizeof(struct ifreq); /* By default */
		printf("Index %d, Interface %s, address_family=%d, ", i+1, n->ifr_name, n->ifr_addr.sa_family);
		switch(n->ifr_addr.sa_family)
		{
			case AF_INET:
			ntop_result=(char*)inet_ntop(n->ifr_addr.sa_family, &(((struct sockaddr_in*)&(n->ifr_addr))->sin_addr), ntop_buf, INET_ADDRSTRLEN);
			printf ("IPv4_Addr=%s\n", (ntop_result ? ntop_buf : "IPv4_Indeterminate"));
			n_inc = sizeof(n->ifr_name) + n->ifr_addr.sa_len;
			break;

			case AF_INET6:
			ntop_result=(char*)inet_ntop(n->ifr_addr.sa_family, &(((struct sockaddr_in6*)&(n->ifr_addr))->sin6_addr), ntop_buf, INET6_ADDRSTRLEN);
			printf ("IPv6_Addr=%s\n", (ntop_result ? ntop_buf : "IPv6_Indeterminate"));
			n_inc = sizeof(n->ifr_name) + n->ifr_addr.sa_len;
			break;

			case AF_LINK:
			link_type=((struct sockaddr_dl*)&(n->ifr_addr))->sdl_type;
			if (link_type == IFT_ETHER || link_type==IFT_ISO88023)
				printf ("MAC_Addr (Ether Type %d)=%s\n", link_type, ether_ntoa((struct ether_addr*)LLADDR((struct sockaddr_dl*)&(n->ifr_addr))));
			else if (link_type == IFT_LOOP)
				printf ("MAC_Addr (Loopback)\n");
			else
				printf ("MAC_Addr (Type %d)=(Translation_TBA)\n", (int)link_type);

			n_inc = sizeof(n->ifr_name) + n->ifr_addr.sa_len;
			break;

			default:
			printf ("Addr=AF_UNKNOWN\n");
			n_inc = sizeof(n->ifr_name) + n->ifr_addr.sa_len;
			break;
		}
		if (n_inc < sizeof(struct ifreq)) n_inc=sizeof(struct ifreq);
	}
	close(s);
}
