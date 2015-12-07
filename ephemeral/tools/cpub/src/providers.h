#ifndef PROVIDERS_H
#define PROVIDERS_H

#include "configfile.h"


/* The maximum size of a quad part. Total max size is 4x + 3 */
#define MAX_QPART_LEN 64

#include <stdint.h>

/* pitem->data_type (exclusive) flags    stdint.h                            */
#define PDT_UNKNOWN   0
#define PDT_INT8      3     /*     int8    int8_t                            */
#define PDT_UINT8     4     /*   u_int8   uint8_t                            */
#define PDT_INT16     5     /*    int16   int16_t                            */
#define PDT_UINT16    6     /*  u_int16  uint16_t                            */
#define PDT_INT32     7     /*    int32   int32_t                            */
#define PDT_UINT32    8     /*  u_int32  uint32_t                            */
#define PDT_INT64     9     /*    int64   int64_t                            */
#define PDT_UINT64    10    /*  u_int64  uint64_t                            */
#define PDT_TIMEVAL   28
#define PDT_TIMESPEC  29
#define PDT_STRING    30    /* NULL terminated array of char                 */

/* pitem->munge_flag (or-able) flags                                         */
#define MUNGE_RAW     0   /* Applies to provider and final presentation      */
#define MUNGE_DIFF    1   /* Applies to the provider                         */
#define MUNGE_PSAVG   2   /* Average per-second values (if we can)           */
#define MUNGE_FACTOR  4   /* Factor input to "human readable" form           */

/*
  Functions that the provider must provide (as pointers):
   int refresh(int interval)
       This refreshes the data for the provider - ideally only for the
       enabled items.
   int listavial(int dflag)
       This lists the availiable data items for this provider. dflag
       (display flag) determines how to format the output. dflag uses
       the DPIL_* bit field defines.
   struct pitem *get_pitem(struct qparts *qp)
       This returns a reference to a pitem based on the qpart description.
       It may allocate one, or just return the existing reference.
   int enable_pitem(struct qparts *qp)
       This tells the provider that it should enable data collection on 
       this item. This should enable the provider into action (specifically
       set the update_required value in the provider struct) as well as 
       the individual item data collection.

  A discussion on the provider->ki_* items:
    These are NOT required, and are used by the providers (should they
    opt to track these data points). The provider is required to provide
    a method to retrieve this data. (ki_count is a bit of an exception,
    it is considered "public" at this time.)
    The provider uses a "C++ syntax" (in the comments) to denote what is
    public and private.

*/

/* =========================================================================
 * Name: qparts
 * Description: A breakdown of a quad into each of the quad parts 
 * Notes: This is a much easier method of working through a quad than
 *        trying to parse a raw/linear string each time.
 */
struct qparts
{
   char pname[MAX_QPART_LEN];    /* The provider name  (ie: vmstat)        */
   char pargs[MAX_QPART_LEN];    /* Additional arguments to the provider   */
   char iname[MAX_QPART_LEN];    /* The data point name (ie: memtotal)     */
   char iargs[MAX_QPART_LEN];    /* Arguments for data handling (ie: diff) */

   struct prov_name *cfpn;       /* See the following comment block...     */

   /* (cfpn) Config file prov_name. This MAY NOT be set. It is OPTIONAL.
      This is used in the case where data is sent from the EnableDataPoint()
      through the provider enable() implementation to the NewPItem()
      function. It is only used in that case. In all other cases (such as
      when a quad (qparts) is created elsewhere) this value is undefined
      (NULL). So.... In short....
      - It WILL be there when you are in provider->enablepitem() code.
      - It will likely NOT be there any other time.
      - Always test before using it. */
};

/* =========================================================================
 * Name: pitem
 * Description: A reference to a data item (within a provider) 
 * Notes: Make careful note that some items are PUBLIC (meaning that your
 *        provider implementation MUST use these struct members. Other items 
 *        are PRIVATE and are added for convenience to the provider writer.
 *        THERE ARE TWO TYPES OF "PRIVATE" MEMBERS, "PROPOSED" AND SIMPLY
 *        THOSE MARKED "PRIVATE". This is because the framework is still 
 *        not perfectly designed and the items marked proposed *might* be
 *        used. If you use them - change the status from proposed to private!
 *        The framework can be extended, but DO NOT add lots of members to
 *        this struct that cannot be used elsewhere. If you need to track
 *        multiple items, then put it in a struct and attach that to the
 *        void *dstruct; pointer.
 *        The current argument here (that will be soon resolved) is: "Should
 *        a double reference be required to get to your data (the dstruct
 *        pointer) or should we allow for "a few" COMMON generic types to be
 *        added to the struct? One offers greater flexibility with minimal
 *        memory/storage overhead (when not used) while the other offers a
 *        faster access to the data items. End-user usage patterns have not
 *        been determined - so we just don't know at this time what is best.
 */
struct pitem
{
   /* PUBLIC (don't assume that you (whoever you are) have this to yourself) */
   char name[MAX_QPART_LEN];     /* The iname qpart (part of the quad)       */
   int data_type;                /* Flag that determines what kind of data   */
                                 /*   type that data_ptr points to. This is  */
                                 /*   used primarily to format for output.   */
                                 /*   Uses the PDT_* defines.                */
   void *data_ptr;               /* Used for access to the "final" printed   */
                                 /*   item.                                  */
   int sign_flag;                /* The sign of unsigned (and signed) data   */
   int munge_flag;               /* How the provider should represent the    */
                                 /*   data (specifically raw or diff(ed)).   */
                                 /*   Uses the MUNGE_* defines.              */
   struct pitem *next_opi;       /* output list item - This is used by the   */
                                 /*   core of the framework to walk the list */
                                 /*   of output items.                       */
   /* For the writer module ------------------------------------------------ */
   /*  >>> The provider can peek at this if it thinks it is relevant. Likely */
   /*      it will not be necessary, nor appropriate. There is a lot of      */
   /*      discussion about this in the NewPItem() code.                     */
   unsigned int fact_from;       /* Type that data before factoring          */
   unsigned int fact_to;         /* Type of representation after factoring   */
   int64_t alarm_value;
   int alarm_type;
   char header[MAX_QPART_LEN];   /* A data-item specific version of the      */
                                 /*   header string. Required for dynamic    */
                                 /*   (read: * specified) items. This is set */
                                 /*   by NewPItem().                         */
   void *wdata;                  /* Used by the writer in the event that     */
                                 /*   something needs to be saved related to */
                                 /*   the pitem.                             */

   /* PRIVATE (for the "optional" use by the provider implementation) */
   unsigned long sioffset;      /* Offset from the top of the struct of this */
                                /*   data item. Used when retrieving data    */
                                /*   from a struct. This allows the provider */
                                /*   to reference the data by location and   */
                                /*   not name.                               */
   struct pitem *next_ui;       /* Update Item - This is (optionally) used   */
                                /*    by the provider to track what needs    */
                                /*    to be updated.                         */
   void *dstruct;               /* This can be any "struct" of data. The     */
                                /*    type is indeterminant so that it is    */
                                /*    not tied to a specific kind of struct. */
};

/* =========================================================================
 * Name: provider
 * Description: A reference to all (pubilc) information on a provider
 * Notes: 
 *        
 *        
 */
struct provider
{
   /* PUBLIC */
   char name[MAX_QPART_LEN];    /* First item in the quad */
   int update_required;

   int (*refresh)(int interval);                    /* Aka: update()         */
   int (*listavail)(int dflag);                     /* Aka: list()           */
   struct pitem *(*enablepitem)(struct qparts *qp); /* Aka: enable()         */

   struct provider *next;

   /* PRIVATE */
   struct pitem *ui_list;       /* Update items - must be used for items     */
                                /*                targeted for output        */
};


/* =========================================================================
 * Name: proot
 * Description: The root of all providers (the trunk of the tree)
 * Notes: (References to) All providers sit off this struct.
 *        This is used as a handle to all provider data.
 *        
 */
struct proot
{
   struct provider *prov_list;  /* List of known (parent) providers */
   struct pitem *pi_olist;      /* List of output items             */

   int pitem_count;
};


/* =========================================================================
 * Name: Providers
 * Description: This initializes the provider root structure
 * Access: Public - used by the framework (never by the providers)
 * Paramaters: None
 * Returns: pointer to the provider root structure, NULL on failure
 * Side Effects: Allocates memory
 * Notes: A simiple initialization function for the proot struct
 */
struct proot *Providers(void);


/* =========================================================================
 * Name: RegisterProvider
 * Description: Register "yourself" as a provider
 * Access: Private - used only by the providers
 * Paramaters: proot (to register in)
 *             name of the provider instance
 *             function pointers to the three primary functions
 * Returns: Valid pointer on success, NULL on failure
 * Side Effects: Will allocate memory and insert into the provider list. Your
 *        code, and this code should not (and in this case, will not) do
 *        anything more. Registration here is designed to be as lightweight
 *        as possible.
 * Notes: This is a method to register a provider - it requires all the 
 *        info needed to properly set up a provider. It should only be called
 *        by those "inheriting" this method.
 */
struct provider *RegisterProvider(struct proot *p,
                                  char *name,
                                  int (*refresh)(int interval),
                                  int (*listavail)(int dflag),
                                  struct pitem *(*enablepitem)(struct qparts *qp));

/* =========================================================================
 * Name: NewPItem
 * Description: Allocate and fill the public parts of a new pitem struct
 * Access: Private - used only by the providers
 * Paramaters: All the required/public parts of the pitem struct
 * Returns: Pointer to pitem or NULL on failure (to malloc)
 * Side Effects: Allocates memory, DOES NOT ADD TO LINKED LISTS (see Notes)
 * Notes: diff and psavg will be set for you.
 *        List management ===> The enablepitem() function (pointer) should
 *           manage the update item list. This is the list that the provider
 *           may use in order to update items. NewPItem() does not assume that
 *           a provider implementation will use that method. So it does not
 *           set it. Furthermore, it is possible that something could go
 *           wrong outside of NewPItem() so it does not register in the output
 *           items list either. Where list maintenance is done:
 *            - Update list: By the provider (if it uses it). A function called
 *              InsertUpdateItem() has been supplied to do this for you.
 *            - Output list: By the EnableDataPoint() function. This is the
 *              outermost wrapper on item enablement.
 */
struct pitem *NewPItem(struct qparts *qp,
                       int data_type,
                       void *data_ptr);

/* =========================================================================
 * Name: StrToQuadParts*
 * Description: Parse a string into a qparts struct
 * Access: Public
 * Paramaters: Create/Fill: string - containing a valid quad representation
 *             Fill: A pre-allocated (stack/automatic/local) qpart struct
 * Returns: Success(Create/Fill): pointer/0 ; Failure NULL/non-zero
 * Side Effects:
 * Notes: There are two variants of this function. One takes an existing
 *        qparts struct (and fills it), the other allocates a new qparts
 *        struct from the heap.
 */
struct qparts *StrToQuadPartsCreate(char *quad);
int StrToQuadPartsFill(struct qparts *qp, char *quad);

/* =========================================================================
 * Name: DumpPItemList
 * Description: Dump (stdout) a list of all quads known to the framework
 * Access: Public - used by the framework
 * Paramaters: proot - The root of the provider "tree"
 *             int   - Options on how to display the data 
 * Returns: Count of the number of items dumped
 * Side Effects: Writes (lots of) lines to stdout
 * Notes: This interface is the "public" interface called by the rest of the
 *        the framework. The Dump* functions that follow are used by the
 *        providers themselves to assist in formatting.
 */
#define DPIL_DATATYPE 0x01    /* Display the data type */

int DumpPItemList(struct proot *root, int dflag);

/* =========================================================================
 * Name: DumpQuadData
 * Description: Dump (to stdout) the leading info on the list quads output
 * Access: Private - strictly provider-only functions
 * Paramaters: See notes
 * Returns: Nothing
 * Side Effects: Writes to stdout
 * Notes: 
 */
void DumpQuadData(int dflag, int data_type);

/* =========================================================================
 * Name: ShouldDiff  ----- And related functions
 * Description: See if the diff flag was specified in the argument list
 * Access: Private - Used by the providers to determine diff 
 * Paramaters: qparts struct - holding a valid quad
 * Returns: 1 if to diff, 0 if not to diff
 * Side Effects: None
 * Notes: Call it thusly:
 *   if ( ShouldDiff(the_quad) )
 *      this_pitem->munge_flag |= MUNGE_DIFF;
 *
 * This is not an optimized method... Specifically we copy the string,
 * convereting to upper case, and then do a quick search. The copy costs.
 * The conversion is contrary to the users inclination to use lower case.
 * And the search is after we have already walked the string doing compares.
 * That said, we do not do this frequently and the optimization would not
 * produce significant benefit.
 */
int ShouldDiff(struct qparts *qp);    /* Looks for "diff" ----> MUNGE_DIFF  */
int ShouldPSAvg(struct qparts *qp);   /* Looks for "psavg" ---> MUNGE_PSAVG */


/* ========================================================================= */
/* These are retrieval functions to retrieve a reference to a provider based
   on either a quad string or a broken-down quad string (qparts struct).
*/
struct provider *GetProviderByNameStr(struct proot *p, char *quad);
struct provider *GetProviderByNameQP(struct proot *p, struct qparts *qp);


/* =========================================================================
 * Name: EnableDataPoint
 * Description: Enable the data point (quad) from the config file prov_name
 * Access: Public - used by the framework
 * Paramaters: The root of the provider tree (proot)
 *             The prov_name as returned from reading the config file
 * Returns: 0 on success, non-0 on failure
 * Side Effects: This will activate a provider and data item. It will likely
 *        cause the provider to actually try and get the data. It will 
 *        allocate memory.
 * Notes: Called by main(), this calls the provider enable() function
 *        implementation (funciton pointer).
 */
int EnableDataPoint(struct proot *p, struct prov_name *pn);

/* =========================================================================
 * Name:InsertUpdateItem
 * Description: Insert a pitem into the update item list
 * Access: Private - Internal ui list for the provider
 * Paramaters: The provider and the pitem to be inserted
 * Returns: 0 on success, non-zero on failure
 * Side Effects: Inserts into the TOP of the list (unordered)
 * Notes: This function is provided so that provider writers do not need
 *        to focus on how the ui list works. The code is trivial, but it
 *        reduces the liklihood of list management errors.
 */
int InsertUpdateItem(struct provider *prov, struct pitem *pi);

/* =========================================================================
 * Name: GetActiveProviderCount
 * Description: Get a list of active providers (that will be updating)
 * Access: Public - used by the framework
 * Paramaters: The root of the provider framework
 * Returns: -1 on error, count on success
 * Side Effects: None
 * Notes:
 */
int GetActiveProviderCount(struct proot *p);

/* =========================================================================
 * Name: GetNextActiveProvider
 * Description: Walk the list of providers that have been activated
 * Access: Public - used by the framework
 * Paramaters: The root of the provider framework
 *             Pointer to the last retrieved (or NULL for first)
 * Returns: Pointer to provider, NULL on last one
 * Side Effects: None
 * Notes: This is a fancy way to walk the list of (active) providers.
 *        "active" means that returned items will only be for providers that
 *        have been told to collect data (have registered quads).
 */
struct provider *GetNextActiveProvider(struct proot *p, struct provider *last);


/* =========================================================================
 * Name: CalcData
 * Description: Calculate the data for a pitem (does diff/psavg conversions...)
 * Access: Private - Used only by provider implementations for specifc needs
 * Paramaters: The pitem to be updated with latest data
 *             Pointer to the last data
 *             Pointer to the recent data
 *             The interval value (between iterations)
 *             NOTE: The pointer is to the DATA, not a structure that
 *                   references the data. For example... if you use an
 *                   sioffset, then the CALLING function must deal with that
 *                   math.
 * Returns: 0 if the calculations where handled, 1 if not
 * Side Effects: Modifies data in the pitem
 * Notes: This is only called from the update() implementaion in the provider.
 *        The design here is to simplify provider update() implementations
 *        as well as handle the unsigned value goes negative problem (with
 *        diffs on unsigned data).
 * CRITICAL NOTE: This will assert() on non-int data! So no strings, timevals,
 *        or timespecs.
 */
int CalcData(struct pitem *pi, void *last_data, void *this_data, int interval);

#endif
