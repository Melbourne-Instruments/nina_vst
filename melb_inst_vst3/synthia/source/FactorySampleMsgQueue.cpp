#include "FactorySampleMsgQueue.h"

// Constants
constexpr char SAMPLER_MSG_QUEUE_NAME[] = "/factory_msg_queue";
constexpr uint SAMPLER_MSG_QUEUE_SIZE = 1;

namespace Steinberg {
namespace Vst {
namespace Nina {

SamplerMsgQueue::SamplerMsgQueue() {
    // Initialise data
    _sampler_mq_desc = (mqd_t)-1;
}

SamplerMsgQueue::~SamplerMsgQueue() {
    // Close the message queue if open
    close();
}

int flag = 1;

bool SamplerMsgQueue::open() {
    mq_attr attr;
    printf("\nopen sampler queue");

    // Open the GUI Message Queue
    std::memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = SAMPLER_MSG_QUEUE_SIZE;
    attr.mq_msgsize = sizeof(SamplerMsg);
    _sampler_mq_desc = ::mq_open(
        SAMPLER_MSG_QUEUE_NAME, (O_CREAT | O_WRONLY | O_NONBLOCK),
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH), &attr);

    return _sampler_mq_desc != (mqd_t)-1;
}

bool SamplerMsgQueue::post_msg() {
    // If the GUI Message Queue is valid
    if (_sampler_mq_desc != (mqd_t)-1) {
        // Write the message to the GUI Message Queue
        int res = ::mq_send(_sampler_mq_desc, (char *)&flag, sizeof(flag), 0);
        printf("\nres: %d %d", res, errno);
        if (res == 0)
            return true;
    }
    return false;
}

void SamplerMsgQueue::close() {
    // Is the GUI Message Queue open?
    if (_sampler_mq_desc != (mqd_t)-1) {
        // Close the GUI FIFO
        ::mq_close(_sampler_mq_desc);
        ::mq_unlink(SAMPLER_MSG_QUEUE_NAME);
        _sampler_mq_desc = (mqd_t)-1;
    }
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
