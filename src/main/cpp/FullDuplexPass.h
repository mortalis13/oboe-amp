#ifndef SAMPLES_FULLDUPLEXPASS_H
#define SAMPLES_FULLDUPLEXPASS_H

#include "FullDuplexStream.h"

#include "logging_macros.h"


class FullDuplexPass : public FullDuplexStream {
public:
    virtual oboe::DataCallbackResult onBothStreamsReady(std::shared_ptr<oboe::AudioStream> inputStream, const void *inputData,
          int numInputFrames, std::shared_ptr<oboe::AudioStream> outputStream,
          void *outputData, int numOutputFrames)
    {
        // Copy the input samples to the output with a little arbitrary gain change.

        // This code assumes the data format for both streams is Float.
        const float *inputFloats = static_cast<const float *>(inputData);
        float *outputFloats = static_cast<float *>(outputData);

        // It also assumes the channel count for each stream is the same.
        int32_t samplesPerFrame = outputStream->getChannelCount();
        int32_t numInputSamples = numInputFrames * samplesPerFrame;
        int32_t numOutputSamples = numOutputFrames * samplesPerFrame;
        
        // It is possible that there may be fewer input than output samples.
        int32_t samplesToProcess = std::min(numInputSamples, numOutputSamples);
        
        // Write to output only the channel 2 in mono from the input (set start inputId to 0 to use only the channel 1)
        int inputId = 1;
        for (int32_t i = 0; i < samplesToProcess; i += 2) {
            auto inputSample = *(inputFloats + inputId);
            inputId += 2;
            
            *outputFloats++ = inputSample;
            *outputFloats++ = inputSample;
        }
        
        // If there are fewer input samples then clear the rest of the buffer.
        int32_t samplesLeft = numOutputSamples - numInputSamples;
        for (int32_t i = 0; i < samplesLeft; i++) {
            *outputFloats++ = 0.0; // silence
        }

        return oboe::DataCallbackResult::Continue;
    }
};
#endif //SAMPLES_FULLDUPLEXPASS_H
