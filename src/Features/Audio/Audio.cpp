#include "Audio.h"

#include "AudioManager.h"

#include <fstream>
#include <cassert>
#include <memory>

void Audio::Initialize()
{
}

void Audio::Finalize()
{

}

void Audio::Unload(SoundData* soundData)
{
    soundData->pBuffer.reset();
    soundData->bufferSize = 0;
    soundData->wfex = {};

    return;
}

void Audio::Play(bool isLoop)
{
    /// バッファ設定
    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = soundData_->pBuffer.get();
    buffer.AudioBytes = soundData_->bufferSize;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;

    /// SourceVoice作成
    hr_ = pXAudio2_->CreateSourceVoice(&pCurrentSourceVoice_, &soundData_->wfex);

    /// 再生
    hr_ = pCurrentSourceVoice_->SubmitSourceBuffer(&buffer);
    hr_ = pCurrentSourceVoice_->SetVolume(volume_);
    hr_ = pCurrentSourceVoice_->Start(0);

    AudioManager::GetInstance()->AddSourceVoice(pCurrentSourceVoice_);
}

void Audio::Stop()
{
    if (pCurrentSourceVoice_)
    {
        hr_ = pCurrentSourceVoice_->Stop();
        hr_ = pCurrentSourceVoice_->FlushSourceBuffers();
    }
}

void Audio::SetVolume(float volume)
{
    this->volume_ = volume;
    if (pCurrentSourceVoice_)
    {
        hr_ = pCurrentSourceVoice_->SetVolume(this->volume_);
    }
}

float Audio::GetVolume() const
{
    return this->volume_;
}
