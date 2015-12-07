#ifndef ACCLOG_H
#define ACCLOG_H

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int InitAccessLog(char *filename);

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int LogAccess(int sock, int tree, int format, int hrv);








#endif
