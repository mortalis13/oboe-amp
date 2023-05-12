#include "FullDuplexStream.h"

oboe::DataCallbackResult FullDuplexStream::onAudioReady(oboe::AudioStream *outputStream, void *audioData, int numFrames) {
    oboe::DataCallbackResult callbackResult = oboe::DataCallbackResult::Continue;
    int32_t actualFramesRead = 0;

    // Silence the output.
    int32_t numBytes = numFrames * outputStream->getBytesPerFrame();
    memset(audioData, 0 /* value */, numBytes);

    if (mCountCallbacksToDrain > 0) {
        // Drain the input.
        int32_t totalFramesRead = 0;
        do {
            oboe::ResultWithValue<int32_t> result = mInputStream->read(mInputBuffer.get(), numFrames, 0 /* timeout */);
            if (!result) {
                // Ignore errors because input stream may not be started yet.
                break;
            }
            
            actualFramesRead = result.value();
            totalFramesRead += actualFramesRead;
        }
        while (actualFramesRead > 0);
        
        // Only counts if we actually got some data.
        if (totalFramesRead > 0) {
            mCountCallbacksToDrain--;
        }
    }
    else if (mCountInputBurstsCushion > 0) {
        // Let the input fill up a bit so we are not so close to the write pointer.
        mCountInputBurstsCushion--;
    }
    else if (mCountCallbacksToDiscard > 0) {
        // Ignore. Allow the input to reach to equilibrium with the output.
        oboe::ResultWithValue<int32_t> result = mInputStream->read(mInputBuffer.get(), numFrames, 0 /* timeout */);
        if (!result) {
            callbackResult = oboe::DataCallbackResult::Stop;
        }
        mCountCallbacksToDiscard--;
    }
    else {
        // Read data into input buffer.
        oboe::ResultWithValue<int32_t> result = mInputStream->read(mInputBuffer.get(), numFrames, 0 /* timeout */);
        if (!result) {
            callbackResult = oboe::DataCallbackResult::Stop;
        }
        else {
            int32_t framesRead = result.value();
            callbackResult = onBothStreamsReady(mInputStream, mInputBuffer.get(), framesRead, mOutputStream, audioData, numFrames);
        }
    }

    if (callbackResult == oboe::DataCallbackResult::Stop) {
        mInputStream->requestStop();
    }

    return callbackResult;
}

oboe::Result FullDuplexStream::start() {
    mCountCallbacksToDrain = kNumCallbacksToDrain;
    mCountInputBurstsCushion = mNumInputBurstsCushion;
    mCountCallbacksToDiscard = kNumCallbacksToDiscard;

    // Determine maximum size that could possibly be called.
    int32_t bufferSize = mOutputStream->getBufferCapacityInFrames() * mOutputStream->getChannelCount();
    if (bufferSize > mBufferSize) {
        mInputBuffer = std::make_unique<float[]>(bufferSize);
        mBufferSize = bufferSize;
    }
    
    oboe::Result result = mInputStream->requestStart();
    if (result != oboe::Result::OK) {
        return result;
    }
    
    return mOutputStream->requestStart();
}

oboe::Result FullDuplexStream::stop() {
    oboe::Result outputResult = oboe::Result::OK;
    oboe::Result inputResult = oboe::Result::OK;
    
    if (mOutputStream) {
        outputResult = mOutputStream->requestStop();
    }
    
    if (mInputStream) {
        inputResult = mInputStream->requestStop();
    }
    
    if (outputResult != oboe::Result::OK) {
        return outputResult;
    } else {
        return inputResult;
    }
}

int32_t FullDuplexStream::getNumInputBurstsCushion() const {
    return mNumInputBurstsCushion;
}

void FullDuplexStream::setNumInputBurstsCushion(int32_t numBursts) {
    FullDuplexStream::mNumInputBurstsCushion = numBursts;
}
