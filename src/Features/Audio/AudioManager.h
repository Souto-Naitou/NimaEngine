#pragma once

#include "Audio.h"
#include <Utility/PathResolver/PathResolver.h>
#include <map>
#include <list>
#include <string>
#include <unordered_map>
#include <memory>

/// <summary>
/// オーディオ読み込みクラス
/// </summary>
class AudioManager
{
public:
    AudioManager(AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

    static AudioManager* GetInstance()
    {
        static AudioManager instance;
        return &instance;
    }


public:
    /// <summary>
    /// オーディオシステムを初期化します。
    /// </summary>
    void Initialize();
    
    /// <summary>
    /// 状態更新（再生キューの処理など）を行います。
    /// </summary>
    void Update();
    
    /// <summary>
    /// 終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// 検索パスを追加
    /// </summary>
    /// <param name="path">パス</param>
    void AddSearchPath(const std::string& path);

    void AddSourceVoice(IXAudio2SourceVoice* sv) { sourceVoices_.push_back(sv); }
    /// <summary>
    /// 新しい Audio インスタンスを作成して返します。
    /// </summary>
    /// <param name="category">カテゴリ名。</param>
    /// <param name="filename">音声ファイル名。</param>
    /// <returns>作成された Audio。</returns>
    Audio* GetNewAudio(const std::string& category , const std::string& filename);


private:
    HRESULT_ASSERT hr_ = {};
    Microsoft::WRL::ComPtr<IXAudio2> pXAudio2_;
    IXAudio2MasteringVoice* pMasteringVoice_ = nullptr;
    std::map<std::string, SoundData> soundDataMap_;
    std::unique_ptr<PathResolver> pFilePathSearcher_ = nullptr;

    std::list<IXAudio2SourceVoice*> sourceVoices_;
    // <カテゴリ, Audioリスト>
    std::unordered_map<std::string, std::list<std::unique_ptr<Audio>>> audioMap_; // 音声ファイル名とAudioのリストのマップ

private:
    AudioManager() = default;
    ~AudioManager() = default;
    /// <summary>
    /// WAV ファイルを読み込みます。
    /// </summary>
    /// <param name="filename">ファイル名。</param>
    /// <returns>読み込んだサウンドデータ。</returns>
    SoundData& LoadWave(const char* filename);

    /// <summary>
    /// チャンクヘッダを読み込み、ターゲットに一致するまで進めます。
    /// </summary>
    /// <param name="file">入力ストリーム。</param>
    /// <param name="chunkHeader">読み込まれるチャンクヘッダ。</param>
    /// <param name="target">探索するチャンクID。</param>
    void ChunkHeaderRead(std::ifstream& file, ChunkHeader& chunkHeader, const char* target);
};