/* lparstat_ext.c:  Demo to reveal sockets, chips/socket, cores/chip, etc. for a whole-system LPAR
 * {DRQS 70057029 <go>}
 *
 * To build:
 	/bb/util/version12-102014/usr/vac/bin/xlc -qarch=pwr6 -qtune=pwr7 -qdfp -qstrict -o lparstat_ext lparstat_ext.c
 *
 * Author:  Eugene Schulman <ESchulman6@bloomberg.net>, R&D Systems Performance (G325)
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/dr.h>
#include <sys/utsname.h>
#define __Q(x) #x
#define QUOTE(x) __Q(x)

/* Build with
 * /bb/util/version12-102014/usr/vac/bin/xlc -qarch=pwr6 -qtune=pwr7 -O2 -o lparstat_ext lparstat_ext.c
 */

/* Ref: 
 * http://www-01.ibm.com/support/knowledgecenter/ssw_aix_71/com.ibm.aix.basetrf1/lpar_get_info.htm?lang=en
 * Used as a starting point
 * 
 * Reference data
 ** p730 (8231-E2D)  (Total 8 cores @ 4.3GHz, 12 cores @ 4.2GHz, 16 cores @ 3.6GHz or 16 cores @ 4.2GHz)
 * Sockets 2
 * Chips   1
 * Cores   8
 *
 ** p750 (8233-E8B)
 * Sockets 4
 * Chips   1
 * Cores   8
 *
 ** p750+ (8408-E8D)  (8 cores @ 3.5GHz  or  8 cores @ 4.0GHz)
 * Sockets 4
 * Chips   2
 * Cores   4
 *
 ** p780 (9179-MHB)
 * Sockets 4
 * Chips   1
 * Cores   8
 * 
 ** p780+ (9179-MHD)  (8 cores @ 3.72GHz  or  4 cores @ 4.42GHz)
 * Sockets 8
 * Chips   1
 * Cores   4
 */

int main(int argc, char **argv, char **envp)
{
	uint64_t module_count;
	proc_module_info_t *buffer = NULL;
	struct lpar_info_format1_t  lpar_data1;
	struct utsname my_uname;
	int rc;
	int i;
	char *progname;
	char iobuf[BUFSIZ];

	setvbuf(stdout, iobuf, _IOFBF, sizeof(iobuf));

	progname=(char*)strdup((const char*)basename(argv[0]));
	if (!progname)
	{
		fprintf(stderr, "FATAL: Buffer allocation failed at (%s:%d): %s, error code %d\n",
		 __FILE__, __LINE__, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	uname(&my_uname);

	/* Retrieve the total count of modules on the system */
	rc = lpar_get_info(NUM_PROC_MODULE_TYPES, &module_count, sizeof(uint64_t));
			 
	if (rc)
	{
		fprintf(stderr, "%s: lpar_get_info(NUM_PROC_MODULES_TYPES, ...) failed at (%s:%d):  %s, error code %d\n",
		 progname, __FILE__, __LINE__, strerror(errno), errno);
		return EXIT_FAILURE;
	}
	             
	/* Allocate buffer of exact size to accomodate module information */
	buffer = malloc(module_count * sizeof(proc_module_info_t));
	     
	if (!buffer)
	{
		fprintf(stderr, "%s: Buffer allocation failed at (%s:%d): %s, error code %d\n",
		 progname, __FILE__, __LINE__, strerror(errno), errno);
		return EXIT_FAILURE;
	}
	
	rc = lpar_get_info(PROC_MODULE_INFO, buffer, (module_count * sizeof(proc_module_info_t)));
	
	if (rc)
	{
		fprintf(stderr, "%s: lpar_get_info(PROC_MODULE_INFO, ...) failed at (%s:%d):  %s, error code %d\n",
		 progname, __FILE__, __LINE__, strerror(errno), errno);
		return EXIT_FAILURE;
	}
	
	/* Retrieve the static LPAR structure */
	rc = lpar_get_info(LPAR_INFO_FORMAT1, &lpar_data1, sizeof(lpar_data1));
			 
	if (rc)
	{
		fprintf(stderr, "%s: lpar_get_info(LPAR_INFO_FORMAT1, ...) failed at (%s:%d):  %s, error code %d\n",
		 progname, __FILE__, __LINE__, strerror(errno), errno);
		return EXIT_FAILURE;
	}


	/*  If rc is 0, then buffer contains an array of proc_module_info_t
	 *  structures with module_count elements.  For an element of 
	 *  index i:
	 *       
	 *      buffer[i].nsockets is the total number of sockets
	 *      buffer[i].nchips   is the number of chips per socket		 
	 *      buffer[i].ncores   is the number of cores per chip
	 */
	 
	printf(
	  "Nodename     %-." QUOTE(SYS_NMLN) "s\n"
	  "LPAR         %-." QUOTE(LPAR_NAME_LEN1) "s\n"
	  "Machine      %-." QUOTE(SYS_NMLN) "s\n"
	  "Sysname      %-." QUOTE(SYS_NMLN) "s\n"
	  "Version      %-." QUOTE(SYS_NMLN) "s\n"
	  "Release      %-." QUOTE(SYS_NMLN) "s\n"
	  "SMT threads  %hu\n",
	  &(my_uname.nodename),
	  &(lpar_data1.lpar_name),
	  &(my_uname.machine),
	  &(my_uname.sysname),
	  &(my_uname.version),
	  &(my_uname.release),
	  lpar_data1.smt_threads);

	for (i=0 ; i < module_count ; i++)
	{
		printf(
		  "Sockets      %d\n"
		  "Chips/Socket %d\n"
		  "Cores/Chip   %d\n"
		  "Cores/Socket %d\n",
		  buffer[i].nsockets,
		  buffer[i].nchips,
		  buffer[i].ncores,
		  buffer[i].ncores * buffer[i].nchips);
	}
	fflush(stdout);
	return 0;
}
