#ifndef OSSPECIFIC_H
#define OSSPECIFIC_H

#include "solbasic.h"
#include "process.h"

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int InitOSSCoreData(void);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int RefreshOSSCoreData(void);

/* =========================================================================
 * Name: 
 * Description: Init for anything in the "hardware" tree
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int InitOSSHardwareData(void);

/* =========================================================================
 * Name: 
 * Description: Refresh for anything in the "hardware" tree
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int RefreshOSSHardwareData(void);

#endif
