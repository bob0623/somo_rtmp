#include "filewriter.h"

#include "common/filewriter.h"
#include "common/logger.h"

#define __CLASS__ "FileWriter"

FileWriter::FileWriter(const std::string& filename)
: m_pFile(NULL) 
, m_nFrames(0)
{
    FUNLOG(Info, "file writer create, filename=%s", filename.c_str());
    m_filename = filename;
    m_pFile = fopen(filename.c_str(), "wb+");
    if (m_pFile == NULL) {
        FUNLOG(Error, "file writer  create failed, name = %s", filename.c_str());
    }
}

FileWriter::~FileWriter() {
    closeFile();
}

int FileWriter::write(const char *data, int len) {
    m_nFrames++;
    if( m_nFrames%100 == 0 ) {
        FUNLOG(Info, "writer file, frames=%u, len=%d", m_nFrames, len);
    }

    if( len == 0 ) {
        FUNLOG(Info, "writer file, len==0", NULL);
        return 0;
    }
    if (m_pFile) {
        int ret =fwrite(data, len, 1, m_pFile);
        fflush(m_pFile);
        return ret;
        
    }
    return 0;
}

void FileWriter::closeFile() {
    if (m_pFile) {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

bool FileWriter::isWriting() {
    return (m_pFile!=NULL);
}

int FileWriter::removeFile() {
    if (m_pFile) {
        closeFile();
        return remove(m_filename.c_str());
    }
    return -1;
}



