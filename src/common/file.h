#ifndef _BAIDU_SHUTTLE_FILE_H_
#define _BAIDU_SHUTTLE_FILE_H_

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include "proto/shuttle.pb.h"

namespace baidu {
namespace shuttle {

enum FileType {
    kLocalFs = 1,
    kInfHdfs = 2
};

enum OpenMode {
    kReadFile = 0,
    kWriteFile = 1
};

struct FileInfo {
    // 'F' stands for file and 'D' stands for directory
    char kind;
    std::string name;
    int64_t size;
};

class File {
public:
    typedef std::map<std::string, std::string> Param;
    /*
     * Create file pointer of specific type. May return NULL
     */
    static File* Create(FileType type, const Param& param);
    /*
     * Wrap the raw pointer with File class:
     *   For local file, the ptr is the FILE pointer
     *   For hdfs, the ptr should be the FS pointer
     *     and still need to open a specific file for IO
     */
    static File* Get(FileType type, void* ptr);

    // Basic file IO interfaces
    virtual bool Open(const std::string& path, OpenMode mode, const Param& param) = 0;
    virtual bool Close() = 0;
    virtual bool Seek(int64_t pos) = 0;
    virtual int32_t Read(void* buf, size_t len) = 0;
    virtual int32_t Write(void* buf, size_t len) = 0;
    virtual int64_t Tell() = 0;
    virtual int64_t GetSize() = 0;
    virtual bool Rename(const std::string& old_name, const std::string& new_name) = 0;
    virtual bool Remove(const std::string& path) = 0;
    // Use pointer here to work with boost::bind in thread pool
    virtual bool List(const std::string& dir, std::vector<FileInfo>* children) = 0;
    virtual bool Glob(const std::string& dir, std::vector<FileInfo>* children);
    virtual bool Mkdir(const std::string& dir) = 0;
    virtual bool Exist(const std::string& path) = 0;
    virtual std::string GetFileName() = 0;

    // Used to ensure that all data in buf is written or buf already has all the required data
    size_t ReadAll(void* buf, size_t len);
    bool WriteAll(const void* buf, size_t len);

    // File-related tools
    /*
     * Use information from DfsInfo to build file param
     */
    static Param BuildParam(const DfsInfo& info);
    /*
     * Extract information from a full address. Support all schema in FileType
     *   Address format: [type]://[hostname/ip]:[port][absolute path]
     *   e.g.: hdfs://localhost:9999/home/test/hdfs.file
     *         file:///home/test/local.file
     */
    static bool ParseFullAddress(const std::string& address,
            FileType* type, std::string* host, std::string* port, std::string* path);
    /*
     * Create address using above schema
     */
    static bool BuildFullAddress(const FileType type, const std::string& host,
            const std::string& port, const std::string& path, std::string& address);
    /*
     * Connect to HDFS. caller has to include hdfs sdk header for hdfsFS struct
     */
    static bool ConnectInfHdfs(const Param& param, void** fs);
    /*
     * Check if a path match with a wildcard-included pattern
     *   Support: * - any character for 0 or more times, ? - any character for 1 time
     */
    static bool PatternMatch(const std::string& origin, const std::string& pattern);

    virtual ~File() { }
};

template <class FileType>
class FileHub {
public:
    static FileHub<FileType>* GetHub();

    virtual FileType* Store(const File::Param& param, FileType* fp) = 0;
    virtual FileType* Get(const std::string& address) = 0;
    virtual FileType* Get(const std::string& host, const std::string& port) = 0;
    virtual File::Param GetParam(const std::string& address) = 0;
    virtual File::Param GetParam(const std::string& host, const std::string& port) = 0;

    virtual ~FileHub() { }
};

}
}

#endif
