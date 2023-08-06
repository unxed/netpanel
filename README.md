# netpanel
network plugin for far2l (currently sftp only)

Works as a wrapper against command line sftp tool, so no dependencies needed.

That works?

- connecting by key file
- connecting by password
- adding host to known hosts list
- browsing files and folders
- downloading from server files and folder
- uploading to server files and folder
- making folders
- removing files and empty folder
- tested at least on Ubuntu 22.04

That still does not (but planned in future)?

- download/upload progress indication
- viewing, editing remote files
- selection support (currently plugin is capable working with one file/folder under a cursor only)
- removing non empty folders
- support for the other similar tools (like ftp)
- help
- languages/translations

Heavily based on TmpPanel far2l plugin and still contains some unused code from it.

Checkout inside root of far2l tree, add
```
add_subdirectory (netpanel)
```
to the end of root CMakeLists.txt

C++17 supporting compiler is requred (gcc 7+, clang 5+).
