#include "FMSU.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#define _USE_MATH_DEFINES
#include <cmath>

#define E 2.71828182845904523536

FMSU::FMSU(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
    GetParam(kGain)->InitGain("Gain", 0., -64.0, 12.0, 0.01);
    GetParam(kDistortion)->InitPercentage("Distortion", 0., 0., 100.0, 0.01);
    GetParam(kDistortionThreshold)->InitPercentage("Distortion Threshold", 100.0, 0., 100.0, 0.01);
    GetParam(kCompression)->InitPercentage("Compression", 0., 0., 100.0, 0.01);
    GetParam(kExpansion)->InitPercentage("Expansion", 0., 0., 100.0, 0.01);
    
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };
    
    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
        pGraphics->AttachPanelBackground(COLOR_GRAY);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
        const IRECT b = pGraphics->GetBounds();
        //pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "", IText(50)));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100).GetHShifted(-200), kGain));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100).GetHShifted(-100), kDistortion));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(100).GetHShifted(-100), kDistortionThreshold));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kCompression));
        pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100).GetHShifted(100), kExpansion));
    };
#endif
}

#if IPLUG_DSP
void FMSU::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    const double gain = GetParam(kGain)->Value();
    const double distortionParam = GetParam(kDistortion)->Value();
    const double distortionThresholdParam = GetParam(kDistortionThreshold)->Value() / 100.;
    const int nChans = NOutChansConnected();
    
    const double scaledDistortionParam = distortionParam / 10.0;
    
    const double distortionAmount = pow(10, scaledDistortionParam);
    
    const double reverseDistortion = (20 * log10(distortionAmount)) / 24.0;
    
    const double reverseDistortionThreshold = 1.0 / distortionThresholdParam;
    
    printf("Distortion: %g\nThreshold: %g\nAmount: %g\nReverse: %g\nGain: %g\nReverse Thresh: %g\n", distortionParam, distortionThresholdParam, distortionAmount, reverseDistortion, gain, reverseDistortionThreshold);
    
    
    
    for (int s = 0; s < nFrames; s++) {
        for (int c = 0; c < nChans; c++) {
            //outputs[c][s] = inputs[c][s] * gain;
            
            double sample = inputs[c][s];
            
            double distorted = sample * distortionAmount;
            
            double clipped = distorted;
            
            if (clipped > distortionThresholdParam) {
                clipped = distortionThresholdParam;
            }
            
            if (clipped < -distortionThresholdParam) {
                clipped = -distortionThresholdParam;
            }
            
            double counterGained = ApplyGain(clipped, -reverseDistortion);
            
            double adjusted = counterGained * reverseDistortionThreshold;
            
            adjusted = ApplyGain(counterGained, gain);
            
            //printf("Clipped: %g\nAdjusted: %g\n", clipped, adjusted);
            
            outputs[c][s] = adjusted;
        }
    }
}

double FMSU::ApplyGain(double sample, double gainDb) {
    
    double type = 20.0;
    
    double index = gainDb / type;
    
    double power = pow(10.0, index);
    
    return sample * power;
}

#endif
