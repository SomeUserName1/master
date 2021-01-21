

/**
 *  The disk space manager shall keep and maintain a file for each record type (for now nodes and relationships)
 *
 *  Further the disk space manager
 *  - de-/allocates new slots (grows and shrinks the file),
 *  - manages the free space bitmap,
 *  - takes care of flushing the application buffer to the os buffer and flush it finally to HW
 *  - it keeps track of active cursors
 *
 */
