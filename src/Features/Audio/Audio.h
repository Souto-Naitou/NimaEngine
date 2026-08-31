#pragma once

#include <xaudio2.h>

#include <list>
#include <memory>

#include <Common/HRESULT_ASSERT.h>

#pragma comment(lib, "xaudio2.lib")

struct ChunkHeader
{
    char id[4];
    DWORD size;
};

struct RiffHeader
{
    ChunkHeader chunkHeader;
    char format[4];
};

struct FormatChunk
{
    ChunkHeader chunkHeader;
    WAVEFORMATEX wfex;
};

struct SoundData
{
    WAVEFORMATEX wfex;
    std::unique_ptr<BYTE[]> pBuffer;
    DWORD bufferSize;
};

/// <summary>
/// オーディオクラス (再生用ハンドラ)
/// </summary>
class Audio
{
public:
    Audio(IXAudio2* pXAudio2, SoundData* soundData): 
        pXAudio2_(pXAudio2), 
        pSoundData_(soundData) {}

    ~Audio()
    {
        Stop();
        DestroyFinishedSourceVoice();
    }

    void Play(bool isLoop = false);
    void Stop();
    void SetVolume(float volume);
    float GetVolume() const;

private:
    // AudioManagerがAudioの内部にアクセスできるようにする
    friend class AudioManager;

    // 更新処理 (AudioManagerが呼び出す)
    void Update();

    // 再生が終了したSourceVoiceを破棄する
    void DestroyFinishedSourceVoice();

    HRESULT_ASSERT hr_ = {};
    SoundData* pSoundData_ = nullptr;
    IXAudio2* pXAudio2_ = nullptr;

    // 再生中のSourceVoiceのリスト
    std::list<IXAudio2SourceVoice*> sourceVoiceList_;

    // 音量（0.0f - 1.0f）
    float volume_ = 1.0f;
};
