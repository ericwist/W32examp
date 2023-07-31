# W32examp

Compares to directories extracting only unique files, according to peroperties below, from both directories. 
Result: Number of unique files found and the time in milliseconds to do it. 

There is a fast and slow compare, below is the logic for each:
=============================================================
Logic outline by compare type:

Fast Compare:
1. For directory one, create a FileInfoItem class with these fields. These properties taken together represent uniqueness of the file object:
  a. File name.
  b. File size.
  c. Modified date.
2. Traverse directory 1 adding only unique files to a list object of FileInfoItems, according to the three properties above. (e.g. if a file in a subdirectory is the same as one already in the list object, do not add it again).
3. Repeat steps #1 and #2 with a second list object of the same type and do it with another directory, directory 2. Run these in separate threads at the same time, in order to save time.  They are added to different list objects, so there will be no collisions.
4. Compare the two lists of objects to each other file by file and delete all files from the first unique list that exist in the second unique list.
5. Splice(combine) the list together thereby creating one unique list of files resulting from the unique files from both directories.
6. Show Number of unique files found and the time in milliseconds to do it.
7. if "Show unique files" is selected before running, the file will print out in list box to include file name, size and modified date.



Slow Compare:
1. For directory one, create a FileInfoItem class with these fields. The Hash property alone represents uniqueness of the file object:
  a. File name.
  b. Hash of the contents of the file.
2. Traverse directory 1 adding only unique files, by filename and hash, to a list object of FileInfoItems (e.g. if a file in a subdirectory is the same as one already in the list object, do not add it again).
3. Repeat steps #1 and #2 with a second list object of the same type and do it with another directory, directory 2. Run these in separate threads at the same time, in order to save time.  They are added to different list objects, so there will be no collisions.
4. Compare the two lists of objects to each other file by file and delete all files from the first unique list that exist in the second unique list.
5. Splice(combine) the list together thereby creating one unique list of files resulting from the unique files from both directories.
6. Show Number of unique files found and the time in milliseconds to do it.
7. if "Show unique files" is selected before running, the file will print out in list box to include file name, and the file Hashes (4 DWORDS X 32 = 128bit hash).

Build:
=====

To build with/wihtout threads use this define:

#define THREADED_CALLS in 'util.cpp'

Fast and Slow compare are available through the buttons on screen.

