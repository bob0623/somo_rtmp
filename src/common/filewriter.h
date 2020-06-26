#pragma once

#include <stdio.h>
#include <string>

class FileWriter {
public:
    FileWriter(const std::string& filename);
    ~FileWriter();

public:
    int     write(const char *data, int len);
    void    closeFile();
    bool    isWriting();
    int     removeFile();

private:
    std::string m_filename;
    FILE*       m_pFile;
    int         m_nFrames;
};
