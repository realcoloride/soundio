#pragma once

#include "../include.h"
#include "../core/AudioFormat.h"

class AudioInput {
public:
    virtual ~AudioInput() = default;
    virtual ma_data_source* dataSource() = 0;     // required
    virtual AudioFormat     format() const = 0;   // for sanity/conversion
};