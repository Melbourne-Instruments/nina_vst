/**
 * @file GuiMsgQueue.h
 * @author Lance Williams lance@jmxaudio.com
 * @brief
 * @version 0.1
 * @date 2021-10-27
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments, Australia
 */

#pragma once

#include "common.h"
#include <cstring>
#include <mqueue.h>

namespace Steinberg {
namespace Vst {
namespace Nina {

// GUI message
struct GuiMsg {
    float samples[GUI_SAMPLES_SIZE];

    // Constructor
    GuiMsg() {
        // Set the samples buffer to all zeros
        std::memset(samples, 0, sizeof(samples));
    }
};

class GuiMsgQueue {
  public:
    GuiMsgQueue();
    ~GuiMsgQueue();

    bool open();
    bool post_msg(const GuiMsg &msg);
    void close();

  private:
    mqd_t _gui_mq_desc;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
