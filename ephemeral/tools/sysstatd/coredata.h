#ifndef COREDATA_H
#define COREDATA_H

/*
  How this "module" works:
    All items are initialized with InitCoreData(). There is no refresh option.
    Each data item is either static, or refreshed when the Gettor is called.
*/



/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int InitCoreData(void);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetVersion(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetUname_s(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetUname_p(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetUname_r(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetUname_n(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: The InitTimestamp() API is about compatibility with other APIs
 *        in this file. It does not actually do anything, and always
 *        returns success. As this item is dynamic, it must be thread safe,
 *        so all activity is local to GetTimestamp().
 */
long GetTimestamp(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetServed(char *status, unsigned long size);

#endif
