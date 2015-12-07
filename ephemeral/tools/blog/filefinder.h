#ifndef FILEFINDER_H
#define FILEFINDER_H


/* Returns a string with just the base name */
char *GetBaseName(char *input);

/* Creates a string containing just the directory part (no trailing slash) */
char *GetDirPart(char *input);

/* Strips off any leading directory parts */
char *GetFilePart(char *input);

/* Given any string, return a new string with the relative path to the
   metadata file. Input can be basename, one of the filenames, or the
   metadata file itself. This will always give a NEW string, so it will
   need to be freed / will leak memory if called repetadetly */
char *GetMetaDataFile(char *input);

int IsCircularFile(char *input);

#endif


