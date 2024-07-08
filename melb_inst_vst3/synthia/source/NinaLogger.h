#include "AudioFile.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#pragma once

class Logger {
    static const int size2 = 2;
    std::array<std::vector<std::pair<float, int64_t>>, size2> data;

    int counter = 0;
    int size = 10 * 12000;

    Logger() {
        data[0].reserve(size);
        data[1].reserve(size);
    }

    ~Logger() {
    }

    static Logger *instance;

  public:
    static Logger *getInstance() {
        if (instance == 0) {
            printf("\n\nnew logger");
            instance = new Logger();
        }

        return instance;
    }

    void writeChannel1(float sample, int64_t cnt) {
        data[0].push_back({sample, cnt});
    }

    void writeChannel2(float sample, int64_t cnt) {
        data[1].push_back({sample, cnt});
    }

    void tic() { counter++; }

    AudioFile<float> audio_data;

    void writeFile() {
        std::fstream file_handler;
        audio_data.setNumChannels(1);
        audio_data.setNumSamplesPerChannel(10 * 96000);

        std::stringstream path;
        file_handler.open("/home/root/log", std::ios::out | std::ios::trunc);
        if (file_handler.is_open()) {
            for (int i = 0; i < size2; i++) {
                int cont = 0;

                auto it_1 = data[i].begin();
                while (it_1 != data[i].end()) {
                    file_handler << it_1->first << ", ";
                    if (cont < 96000 * 10) {
                        audio_data.samples[0][cont++] = it_1->first;
                    }
                    it_1++;
                }
                file_handler << std::endl;
                it_1 = data[i].begin();
                while (it_1 != data[i].end()) {
                    file_handler << it_1->second << ", ";
                    it_1++;
                }
                file_handler << std::endl;
            }
            printf("\nlog done");
            file_handler.close();
            audio_data.setSampleRate(96000);
            audio_data.save("/udata/nina/tuning/log_wave.wav");
            audio_data.setBitDepth(24);

        } else {
            printf("\nlog failed");
        }
    }
};
