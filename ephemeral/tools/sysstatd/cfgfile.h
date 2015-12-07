#ifndef CFGFILE_H
#define CFGFILE_H

#include "options.h"

struct cfoptions
{
   /* Currently supported */
   char *listen_addr;
   char *listen_port;
   char *httpd_server_threads;

   char *output_buffer_size;
   char *input_buffer_size;

   char *message_file;
   char *access_file;

   char *qc_file_dir;

   /* STUB: Not fully supported */
   char *max_request_min;
   char *max_request_timeo;

};

/* =========================================================================
 * Name: ReadConfigFile
 * Description: Read in the config file and place into a struct
 * Paramaters: The name of the config file
 * Returns: cfoptions struct or NULL on error
 * Side Effects: Allocates memory, reads the file
 * Notes: This pulls the config file in. It does not put configuration
 *        options in the final place. That is done by SaveCFileToOptions().
 */
struct cfoptions *ReadConfigFile(char *filename);


/* =========================================================================
 * Name: SaveCFileToOptions
 * Description: Save what is in the config file to the options struct.
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: This *could* free the cfoptions struct. It does NOT. This is
 *        primarily because assignment is done by handing over pointers in
 *        many cases. (So memory is allocated in ReadConfgFile() and then
 *        just passed as a pointer when coalescing the data into the options
 *        struct. The struct itself could be deleted... but not really worth
 *        it.
 */
int SaveCFileToOptions(struct options *o, struct cfoptions *cfo);


#endif
