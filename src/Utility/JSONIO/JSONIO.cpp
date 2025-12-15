#include "./JSONIO.h"
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <ostream>

using json = nlohmann::json;

#pragma region JSONDataKeeper

    json& JSONStorage::operator[](const std::string& key)
    {
        return jsonDataMap_[ToAbsPathLower(key)];
    }

    JSONStorage::Iterator JSONStorage::begin()
    {
        return Iterator(jsonDataMap_.begin());
    }

    JSONStorage::Iterator JSONStorage::end()
    {
        return Iterator(jsonDataMap_.end());
    }

    // 成功した場合は削除した要素数を返す
    size_t JSONStorage::erase(const std::string& key)
    {
        auto it = jsonDataMap_.find(ToAbsPathLower(key));
        if (it != jsonDataMap_.end())
        {
            jsonDataMap_.erase(it);
            return 1; 
        }
        return 0;
    }

    json& JSONStorage::at(const std::string& key)
    {
        return jsonDataMap_.at(ToAbsPathLower(key));
    }

    JSONStorage::Iterator JSONStorage::find(const std::string& key)
    {
        auto it = jsonDataMap_.find(ToAbsPathLower(key));
        if (it != jsonDataMap_.end())
        {
            return Iterator(it);
        }
        return end();
    }

    std::string JSONStorage::ToAbsPathLower(const std::string& filepath) const
    {
        /// 絶対パスに変換してから小文字に変換する
        std::filesystem::path path(filepath);
        std::string lowerPath = std::filesystem::absolute(path).string();
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(static_cast<int>(c))); });
        return lowerPath;
    }

    std::pair<const std::string, json> JSONStorage::Iterator::operator*() const
    {
        return *it_;
    }

    JSONStorage::Iterator& JSONStorage::Iterator::operator++()
    {
        ++it_;
        return *this;
    }

    JSONStorage::Iterator& JSONStorage::Iterator::operator--()
    {
        --it_;
        return *this;
    }

    bool JSONStorage::Iterator::operator==(const Iterator& other) const
    {
        return it_ == other.it_;
    }

    bool JSONStorage::Iterator::operator!=(const Iterator& other) const
    {
        return it_ != other.it_;
    }

#pragma endregion


#pragma region JSONLoader
    
    const json& JSONIO::Load(const std::string& path)
    {
        if (jsonDataStorage_.find(path) != jsonDataStorage_.end())
        {
            return jsonDataStorage_[path];
        }

        std::fstream fs(path);
        if (!fs.is_open())
        {
            throw std::runtime_error("Failed to open JSON file: " + path);
        }

        json jsonData;
        fs >> jsonData;
        fs.close();

        jsonDataStorage_[path] = jsonData;
        return jsonDataStorage_[path];
    }

    void JSONIO::Save(const std::string& path, const json& jsonData)
    {
        std::filesystem::path filePath = path;
        std::filesystem::create_directories(filePath.parent_path());
        
        std::ofstream ofs(path);
        if (!ofs.is_open())
        {
            throw std::runtime_error("Failed to open JSON file for writing: " + path);
        }

        ofs << jsonData.dump(4) << std::endl;
        ofs.close();
    }

    bool JSONIO::Unload(const std::string& path)
    {
        auto countErase = jsonDataStorage_.erase(path);
        return countErase > 0;
    }

#pragma endregion