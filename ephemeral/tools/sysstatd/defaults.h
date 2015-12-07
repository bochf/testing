#ifndef DEFAULTS_H
#define DEFAULTS_H

/* Default listening port for HTTPD server. This needs to be specified as a 
   string, not a numeric. */
#define DEFAULT_HTTPD_LISTEN_PORT "4795"

/* STUB: #define DEFAULT_CONFIG_FILE "/etc/sysstatd.conf" */
#define DEFAULT_CONFIG_FILE "sysstatd.conf"

/* The number of threads to start to handle HTTPd requests */
#define DEFAULT_HTTPD_THREADS 3

/* Size of the input and output buffers - per httpd thread */
#define DEFAULT_OUTPUT_BUFFER_SIZE 10240
#define DEFAULT_INPUT_BUFFER_SIZE 4096

/* Time we will wait trying to log access to the system */
#define DEFAULT_ACCESS_LOG_TIMEOUT 5

/* ========================================================================= */
/* The following are internal app limits and values                          */

/* Max length of a process name in process list */
#ifdef PORT_AIX
#include <procinfo.h>
#define MAX_PNAME_LEN MAXCOMLEN+1
#else
#define MAX_PNAME_LEN 32
#endif


#define DEFAULT_QC_FILE_DIR "."
#define QC_CONR_BASENAME  "qc_conr_results"

#endif
