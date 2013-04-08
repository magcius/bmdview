#ifndef BMD_OPENFILE_H
#define BMD_OPENFILE_H BMD_OPENFILE_H

#include <string>
#include <cstdio>

struct OpenedFile
{
  //If the file was compressed, this is the
  //filename of the temp file with the uncompressed
  //data (needed by closeFile() to delete that file).
  //If the file was not compressed, this is ""
  std::string tempFileName;

  //this is your file descriptor
  FILE* f;
};

//opens a file for binary reading, if the
//file is yaz0-compressed an uncompressed
//file is returned
OpenedFile* openFile(const std::string& name);

//closes a file, deletes created temporary files
void closeFile(OpenedFile* f);

#endif //BMD_OPENFILE_H
