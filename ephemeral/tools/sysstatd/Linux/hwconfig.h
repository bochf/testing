#ifndef HWCONFIG_H
#define HWCONFIG_H

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int EnumerateCPUs(void);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetCPUSocketCount(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetCPUCoreCount(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetCPULogicalCount(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetCPUThreadCount(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetCPUBogomips(char *status, unsigned long size);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
long GetCPUMHz(char *status, unsigned long size);

#endif
