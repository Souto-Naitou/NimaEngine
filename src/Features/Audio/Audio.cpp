#include "Audio.h"
#include <memory>

void Audio::Play(bool isLoop)
{
    /// バッファ設定
    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = pSoundData_->pBuffer.get();
    buffer.AudioBytes = pSoundData_->bufferSize;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;

    // SourceVoice作成
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    hr_ = pXAudio2_->CreateSourceVoice(&pSourceVoice, &pSoundData_->wfex);

    // 失敗時はリストに入れない
    if (!pSourceVoice) return;   

    // SourceVoiceをリストに追加
    sourceVoiceList_.emplace_back(pSourceVoice);

    /// 再生処理
    hr_ = pSourceVoice->SubmitSourceBuffer(&buffer);
    hr_ = pSourceVoice->SetVolume(volume_);
    hr_ = pSourceVoice->Start(0);
}

void Audio::Stop()
{
    for (auto& sourceVoice : sourceVoiceList_)
    {
        hr_ = sourceVoice->Stop();
        hr_ = sourceVoice->FlushSourceBuffers();
    }
}

void Audio::SetVolume(float volume)
{
    this->volume_ = volume;
    for (auto& sourceVoice : sourceVoiceList_)
    {
        sourceVoice->SetVolume(volume_);
    }
}

float Audio::GetVolume() const
{
    return this->volume_;
}

void Audio::Update()
{
    this->DestroyFinishedSourceVoice();
}

void Audio::DestroyFinishedSourceVoice()
{
    for (auto it = sourceVoiceList_.begin(); it != sourceVoiceList_.end();)
    {
        // 再生状態を取得
        XAUDIO2_VOICE_STATE state;
        (*it)->GetState(&state);

        // 再生が終了している場合はSourceVoiceを破棄
        if (state.BuffersQueued == 0)
        {
            hr_ = (*it)->Stop();
            hr_ = (*it)->FlushSourceBuffers();
            (*it)->DestroyVoice();
            it = sourceVoiceList_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
