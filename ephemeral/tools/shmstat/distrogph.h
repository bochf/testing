#ifndef DISTROGPH_H
#define DISTROGPH_H

#define BA_BASE2   2
#define BA_BASE10  10

/* Bucket Array - Can be base 2, 10, ranges, etc... */
struct barray
{
   int type;        /* BA_BASE2        BA_BASE10                            */
   int bucket_N;    /* 0               0                                    */
   int bucket_0;    /* 1               1 - 9                                */
   int bucket_1;    /* 2               10 - 99                              */
   int bucket_2;    /* 3 - 4           100 - 999                            */
   int bucket_3;    /* 5 - 8           1000 - 9999                          */
   int bucket_4;    /* 9 - 16          10000 - 99999                        */
   int bucket_5;    /* 17 - 32         100000 - */
   int bucket_6;    /* 33 - 64         1000000 - */
   int bucket_7;    /* 65 - 128        10000000 - */
   int bucket_8;    /* 129 - 256       100000000 - */
   int bucket_9;    /* 257 - 512       1000000000 - */
   int bucket_A;    /* 513 - 1024      10000000000 - */
   int bucket_B;    /* 1025 - 2048     100000000000 - */
   int bucket_C;    /* 2049 - 4096     1000000000000 - */
   int bucket_D;    /* 4097 - 8192     10000000000000 - */
   int bucket_E;    /* 8193 - 16386    100000000000000 - */
   int bucket_F;    /* 16387 - 32772   1000000000000000 - */

   int bucket_Z;    /* Overflows */

   int nonzero;     /* Set to true when we have inserted a non-zero value */
};


/* =========================================================================
 * Name: InitBuckets
 * Description: Allocate and initialize array
 * Params: int (BA_BASE2 or BA_BASE10)
 *         
 * Returns: struct barray - initialized to all 0s
 * Side effects: Allocates memory
 * Notes:
 */
struct barray *InitBuckets(int type);

/* =========================================================================
 * Name: BucketValue
 * Description: Place a value into a bucket array
 * Params: barray - Where the data will be placed
 *         unsigned long long - The value to be inserted
 * Returns: zero on insert, non-zero on failure (out of range)
 * Side effects: 
 * Notes:
 */
int BucketValue(struct barray *ba, unsigned long long value);

/* Find the largest count in the bucket array.
   Used as graph_factor to DumpBucketArray() if not time data */
int GetMaxCount(struct barray *ba);

/* =========================================================================
 * Name: DumpBucketArray
 * Description: Write the "call" graph to the named file
 * Params:
 *         ba - The bucket array from InitBuckets() and BucketValue()
 *         graph_factor - This is usually a time factor. As the graph will
 *                be 60 characters across, this is how to scale the graph
 *                to fit in those 60 chars. If you ran for 2 minutes, this
 *                should be 120, and every two instances will generate a
 *                "tick" in the graph
 * Returns: 0 on success, non-zero on failure
 * Side effects: 
 * Notes: The makefile uses a compiler optimization to generate this code.
 *        See notes in the makefile about this.
 */
int DumpBucketArray(struct barray *ba,
                    long graph_factor);

/* =========================================================================
 * Name: HasNonZeroData
 * Description: Simple gettor function for if array has non-zero data
 *              This is a flag to determine if the graph is worth generating
 * Params: struct barray 
 * Returns: 0 if no data, non-0 if the array has non-zero values
 * Side effects: None
 * Notes:
 */
int HasNonZeroData(struct barray *ba);

#endif
