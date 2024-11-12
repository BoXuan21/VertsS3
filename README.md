MyFind Program Documentation
Overview
The myfind program is a C++ utility designed to locate files by name within a specified directory. It allows for parallel file searching by using separate child processes for each filename. The program supports recursive and case-insensitive searches, making it a flexible alternative to the Linux find command.

Key Features
Parallel Search: Spawns a child process for each filename, enabling simultaneous file searches.
Recursive Search (-R): Includes subdirectories in the search.
Case-Insensitive Search (-i): Matches filenames regardless of letter case.
Output Format: Results are displayed as <pid>: <filename>: <complete-path>, where <pid> is the child process ID.
Usage
bash
Code kopieren
./myfind [-R] [-i] <searchpath> <filename1> [filename2] ... [filenameN]
Options:

-R: Enables recursive search through subdirectories.
-i: Enables case-insensitive matching.
<searchpath>: Directory to search.
<filename1> [filename2]...: One or more filenames to search for.
Example:

bash
Code kopieren
./myfind -R -i ./docs report.pdf summary.doc
This command searches for report.pdf and summary.doc in ./docs and its subdirectories, matching files regardless of case.

Code Explanation
1. Helper Function: toLowerCase
   Converts a string to lowercase for case-insensitive comparisons. This function is used if the -i option is specified.

2. File Search Function: findFile
   Performs the actual search within a directory:

Parameters: directory (start point), filename (target file), recursive (whether to include subdirectories), caseInsensitive (match file names without case sensitivity).
Directory Traversal: Opens the specified directory, reads each entry, and applies stat to determine whether it’s a file or subdirectory.
Recursive Handling: If -R is specified and an entry is a subdirectory, findFile calls itself on that subdirectory.
File Matching: For each file entry, checks if the name matches the target filename, applying case-insensitive matching if needed. Prints each match in the required format.
3. Main Function: Parallel Process Creation and Argument Parsing
   Argument Parsing: Uses getopt to process -R (recursive) and -i (case-insensitive) flags, followed by a directory path and a list of filenames.
   Process Forking: For each filename, the program forks a new child process that calls findFile to handle its search.
   Process Synchronization: The parent process waits for each child to complete, preventing "zombie" processes and ensuring all results print before the program exits.