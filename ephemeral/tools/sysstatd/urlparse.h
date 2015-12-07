#ifndef URLPARSE_H
#define URLPARSE_H

/* Commentary:
    - This is an initial naieve implementation.
    - This implementation looks for patterns as opposed to "encountering" 
      them as it walks the URL. The current implementation might be OK for
      parsing parts of the URL, but not sufficient for the finished product.
    - This should return real results that point you to the failure. Perhaps
      this could be done with a list of error strings. (Done: see parse_failure)
    - Some of the functionality will remain $tubbed until I know the complete
      specifics for RESTful params
    - The query struct is designed so that repeated malloc()s and free()s are
      not required for each query.
*/
/*                            V---- Implemented                              */
#define TREE_NONE    0
#define TREE_ROOT    1     /* Y  The default / basic / root tree             */
#define TREE_PERF    2     /* N  The performance tree                        */
#define TREE_SINF    3     /* N  The getsysinfo tree                         */
#define TREE_BBNV    4     /* Y                                              */
#define TREE_PROC    5     /* Y                                              */
#define TREE_HRDW    6     /* Y (Partially)                                  */
#define TREE_QCCR    7     /* Y  The qc_conr results tree                    */
#define TREE_DEFAULT TREE_ROOT

#define FORMAT_NONE  0
#define FORMAT_JSON  1
#define FORMAT_YAML  2
#define FORMAT_XML   3
#define FORMAT_CSV   4
#define FORMAT_DEFAULT FORMAT_YAML

#define UP_ERROR_NONE  0
#define UP_ERROR_FMT   1  /* Multiple format specs or other format error     */
#define UP_ERROR_TREE  2  /* Multiple tree specs or other tree error         */
#define UP_ERROR_SHORT 4  /* The URL was too short to parse properly         */
#define UP_ERROR_MAL   8  /* Malformed URL - Unable to parse                 */
/* Used elseshere */
#define UP_BAD_METHOD  16
#define UP_DEAD_SOCKET 32


/* This is how much data following the ? in the URI that will be copied into
   query->label_list */
#define LABEL_LIST_SIZE 1024

/* ========================================================================= */
struct query
{
   unsigned long parse_failure;
   int tree;            /* Numeric / enum representing the potential trees   */
   int format;          /* See defines for xml, json, csv, yaml              */
   char label_list[LABEL_LIST_SIZE + 1];

   int method; /* This is an ENUM that is (only) used and defined in httpd.c */
};

/* =========================================================================
 * Name: ParseURLString
 * Description: Parse the HTTPd request into a formatted result
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: The URI query strings (?name1=value&name2=value) are not "parsed"
 *        with this function. LABEL_LIST_SIZE characters of this string are
 *        copied into the query struct and can be individually retrieved with
 *        the GetLabelValue() function.
 */
struct query *ParseURLString(struct query *q, char *urlstring);


/* =========================================================================
 * Name: GetLabelValue
 * Description: Retrieve an individual name=value from the ?a=b&c=d string. 
 * Paramaters: value - string to fill (Will come back as a STRING)
 *             vlen - The length of the value string (so as to not overflow)
 *             q - The query as parsed by ParseURLString()
 *             name - The name of the name=value pair 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int GetLabelValue(char *value, unsigned int vlen, struct query *q, char *name);

#endif
