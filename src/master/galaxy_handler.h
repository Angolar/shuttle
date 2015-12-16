#ifndef _BAIDU_SHUTTLE_GALAXY_HANDLER_H_
#define _BAIDU_SHUTTLE_GALAXY_HANDLER_H_
#include <string>

#include <stdint.h>

#include "proto/shuttle.pb.h"
#include "galaxy.h"

namespace baidu {
namespace shuttle {

class GalaxyHandler {

public:
    GalaxyHandler(JobDescriptor& job, const std::string& job_id, int node);
    virtual ~GalaxyHandler() {
        Kill();
        if (galaxy_ != NULL) {
            delete galaxy_;
            galaxy_ = NULL;
        }
    }

    Status Start();
    Status Kill();

    Status SetPriority(const std::string& priority);
    Status SetCapacity(int capacity);

    static int additional_millicores;
    static int64_t additional_memory;

private:
    // For galaxy manangement
    ::baidu::galaxy::Galaxy* galaxy_;
    ::baidu::galaxy::JobDescription galaxy_job_;
    std::string minion_id_;

    // Minion information
    std::string minion_name_;
    JobDescriptor& job_;
    const std::string job_id_;
    int node_;
    std::string node_str_;

};

}
}

#endif

