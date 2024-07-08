/**
 * @file common.h
 * @brief Nina common definitions.
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments, Australia
 */
#pragma once

#ifndef __x86_64__
#include "arm_neon.h"
#endif
#include "pluginterfaces/vst/ivstevents.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#define GPIO_TIMING_PIN_EN

namespace Steinberg {
namespace Vst {
namespace Nina {

// Constants
#ifdef __x86_64__
constexpr char NINA_WAVETABLES_DIR[] = "./";
#else
constexpr char NINA_WAVETABLES_DIR[] = "/udata/nina/wavetables/";
#endif
constexpr int NUM_LAYERS = 4;
constexpr int NUM_VOICES = 12;
constexpr int SAMPLE_RATE = 96000;
constexpr int CV_SAMPLE_RATE = 12000;
constexpr int BUFFER_SIZE = 128;
constexpr int BUFFER_RATE = SAMPLE_RATE / BUFFER_SIZE;
constexpr int CV_BUFFER_SIZE = BUFFER_SIZE / (SAMPLE_RATE / CV_SAMPLE_RATE);
constexpr int GUI_NUM_SAMPLES = 128;
constexpr int GUI_NUM_BUFFERS = 1;
constexpr int GUI_SAMPLES_SIZE = (GUI_NUM_SAMPLES * 2) * GUI_NUM_BUFFERS;
constexpr int MAX_MODULE_OUTPUTS = 2;

constexpr float PARAM_SMOOTH_COEFF = 50 / (float)BUFFER_RATE;
constexpr float DYN_COEFF = 0.02f;
constexpr float SILENCE_THRESH = 0.000001f;
constexpr float LOG_2_5TH_INTERVAL = 0.58496f;
constexpr int CV_MUX_INC = BUFFER_SIZE / CV_BUFFER_SIZE;
constexpr int VEL_MAX = 127;
constexpr uint MAX_NUM_WAVETABLE_FILES = 127;

constexpr float MIN_VEL = (1.0 / 127.0) / 2.0;
constexpr float MID_VEL = 0.5;
constexpr uint DEFAULT_MPE_PB_RANGE = 48;
constexpr uint MAX_MPE_PB_RANGE = 96;
constexpr uint MPE_MAX_NUM_CHANNELS = 15;
constexpr uint MPE_CHANNEL_SETTINGS_MULT = MPE_MAX_NUM_CHANNELS + 1;
constexpr float MORPH_THRESH = 0.001;
constexpr float MORPH_ZERO_THRESH = 0.0 + MORPH_THRESH;
constexpr float MORPH_ONE_THRESH = 1.0 - MORPH_THRESH;

constexpr float AFTERTOUCH_MAG = 2.0;

// MACRO to get the full path of a Nina VST file
#define NINA_WAVETABLE_FILE_PATH(filename) \
    (NINA_WAVETABLES_DIR + std::string(filename))

#ifdef GPIO_TIMING_PIN_EN
constexpr char MEM_DEV_NAME[] = "/dev/mem";
constexpr uint BCM2711_PI4_PERIPHERAL_BASE = 0xFE000000;
constexpr uint GPIO_REGISTER_BASE = 0x200000;
constexpr uint GPIO_SET_OFFSET = 0x1C;
constexpr uint GPIO_CLR_OFFSET = 0x28;
constexpr uint GPIO_TIMING_PIN = 16;
constexpr uint GPIO_TIMING_INITIAL_BUFFER_SKIPS = 45454;

static void *_mmap_bcm_register_base(off_t register_base) {
    uint32_t *addr = nullptr;
    int mem_fd;

    // Open the memory device
    mem_fd = ::open(MEM_DEV_NAME, O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        // Error opening the device
        return nullptr;
    }

    // Map the register base
    addr = reinterpret_cast<uint32_t *>(::mmap(NULL, 4096, (PROT_READ | PROT_WRITE),
        MAP_SHARED, mem_fd,
        (BCM2711_PI4_PERIPHERAL_BASE + register_base)));
    ::close(mem_fd);

    // Was the map successful?
    if (addr == MAP_FAILED) {
        // Error mapping the register base
        assert(false);
        return nullptr;
    }
    return addr;
}

static uint32_t *volatile gpio_set_reg;
static uint32_t *volatile gpio_clr_reg;
static uint32_t dummy_uint = 0;

static uint32_t *init_timing_gpio_pin_set() {
#ifndef __x86_64__
    // Get a pointer to the GPIO registers
    uint32_t *gpio_port = reinterpret_cast<uint32_t *>(_mmap_bcm_register_base(GPIO_REGISTER_BASE));
    assert(gpio_port != nullptr);

    if (gpio_port) {
        // Set the set/clr registers pointers
        gpio_set_reg = gpio_port + (GPIO_SET_OFFSET / sizeof(uint32_t));
        gpio_clr_reg = gpio_port + (GPIO_CLR_OFFSET / sizeof(uint32_t));
    }
    return gpio_set_reg;
#endif
    return &dummy_uint;
}

static inline uint32_t *init_timing_gpio_pin_clr() {
#ifndef __x86_64__
    // Get a pointer to the GPIO registers
    uint32_t *gpio_port = reinterpret_cast<uint32_t *>(_mmap_bcm_register_base(GPIO_REGISTER_BASE));
    assert(gpio_port != nullptr);

    if (gpio_port) {
        // Set the set/clr registers pointers
        gpio_set_reg = gpio_port + (GPIO_SET_OFFSET / sizeof(uint32_t));
        gpio_clr_reg = gpio_port + (GPIO_CLR_OFFSET / sizeof(uint32_t));
    }
    return gpio_clr_reg;
#endif
    return &dummy_uint;
}

static uint32_t *const volatile gpio_set_reg_c = init_timing_gpio_pin_set();
static uint32_t *const volatile gpio_clr_reg_c = init_timing_gpio_pin_clr();

static inline void setGPIO(bool state) {
    if (state) {
        *gpio_set_reg_c = (1 << GPIO_TIMING_PIN);
    } else {
        *gpio_clr_reg_c = (1 << GPIO_TIMING_PIN);
    }
}
#else
static inline voice setGPIO(bool state) {
    printf("\nhere %d", state);
}

#endif

// Layer State
enum LayerState : int {
    STATE_A,
    STATE_B
};

enum GlideModes : uint {
    LOG,
    LINEAR,
    PORTAMENTO_LINEAR,
    NUM_GLIDE_MODES
};

// Voice Mode
enum VoiceMode { LEGATO,
    MONO_RETRIGGER,
    POLY,
    NumVoiceModes };

enum VoiceBitMap {
    VOICE_MUTE_L,
    VOICE_MUTE_R,
    VOICE_MUTE_3,
    VOICE_MUTE_4,
    HARD_SYNC,
    DRIVE_EN_N,
    SUB_OSC_EN_N,
    MIX_MUTE_L,
    MIX_MUTE_R,
    FILTER_TYPE
};

enum MorphMode {
    DANCE,
    DJ,
    NUM_MORPH_MODES
};

// Param Change struct
struct ParamChange {
    ParamID param_id;
    float value;

    ParamChange() :
        param_id(0), value(0.0f) {}

    ParamChange(ParamID p, float v, bool e = false) :
        param_id(p), value(v) {
        if (e) {
            // Set the MS nibble to indicate this should be exported to
            // any notification listeners
            param_id |= 0x80000000;
        }
    }
};

inline static uint mask_param_id(const uint param_id) {
    return (param_id & 0x87FFFFFF);
}

inline static uint get_layers(const uint param_id) {
    return (param_id & 0x78000000) >> 27;
}

inline static uint get_mpe_param_channel(const uint param) {
    return (param & 0x78000000) >> 27;
}

// acceses the param id from the param change and returns the actual ID
inline static uint get_param_id(const ParamChange pc) {
    return (mask_param_id(pc.param_id));
}

// Returns true if the program change should be processed by the passed layer
inline static bool is_layer_param(const uint param_id, const uint layer) {
    return (param_id & (1 << (27 + layer)));
}

inline static ParamChange make_param_layer(ParamChange pc, const uint layer) {
    pc.param_id = (pc.param_id | (1 << (27 + layer)));
    return pc;
}

constexpr uint MIDI_NOTE_MAX = 127u;

// MIDI Note class
class MidiNote {
  public:
    uint pitch;
    float velocity;
    uint channel;

    MidiNote() {
        pitch = 0;
        velocity = 0;
        channel = 0;
    }

    MidiNote(NoteOnEvent note) {
        // Set the note on attibutes
        pitch = note.pitch;
        velocity = note.velocity;
        channel = note.channel;
    }

    MidiNote(NoteOffEvent note) {
        // Set the note off attibutes
        pitch = note.pitch;
        velocity = note.velocity;
        channel = note.channel;
    }

    ~MidiNote() = default;
};

/**
 * @brief Order for the muxing on cv0
 *
 */
enum Cv0Order {
    Cv0Osc1Up,
    Cv0Osc1Down,
    Cv0Osc2Up,
    Cv0Osc2Down,
    Cv0MixOsc1Sq,
    Cv0MixOsc1Tri,
    Cv0MixOsc2Sq,
    Cv0MixOsc2Tri,
};

/**
 * @brief Oder for the muxing on cv1
 *
 */
enum Cv1Order {
    Cv1MixXor,
    Cv1Drive,
    Cv1FilterCut,
    Cv1FilterQ,
    Cv1AmpL,
    Cv1AmpR,
    Cv1Unused,
    Cv1BitArray
};
} // namespace Nina
} // namespace Vst
} // namespace Steinberg