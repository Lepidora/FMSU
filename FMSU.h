#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPrograms = 1;

enum EParams
{
    kGain = 0,
    kDistortionThreshold,
    kDistortion,
    kCompression,
    kExpansion,
    kNumParams
};

using namespace iplug;
using namespace igraphics;

class FMSU final : public Plugin
{
public:
    FMSU(const InstanceInfo& info);
    
#if IPLUG_DSP // http://bit.ly/2S64BDd
    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    double ApplyGain(double sample, double gainDb);
#endif
};
