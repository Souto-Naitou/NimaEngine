#pragma once


#include <string>
#include <Utility/JSONIO/JSONIO.h>
#include <list>
#include <mutex>


/// <summary>
/// ログ出力クラス
/// </summary>
class Logger
{
private:
    struct LogData
    {
        std::string date;
        std::string time;
        std::string status;
        std::string filename;
        std::string action;
        std::string message;
    };


public:
    Logger(Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    static Logger* GetInstance()
    {
        static Logger instance;
        return &instance;
    }

    void Initialize();
    void Save();

    void DrawUI();

    const std::string& GetLogPayload() const
    {
        return logPayload_;
    }

public:
    void LogError(
        const std::string& _filename,
        const std::string& _action,
        const std::string& _message
    );
    
    void LogWarning(
        const std::string& _filename,
        const std::string& _action,
        const std::string& _message
    );

    void LogInfo(
        const std::string& _filename,
        const std::string& _action,
        const std::string& _message
    );

    void LogFatal(
        const std::string& _filename,
        const std::string& _action,
        const std::string& _message
    );

    void LogForOutput(const std::string& _message);


private:
    void Log(
        const std::string& _status,
        const std::string& _filename,
        const std::string& _action,
        const std::string& _message
    );
    void LogJson(
        const std::string& _date,
        const std::string& _time,
        const std::string& _status,
        const std::string& _filename,
        const std::string& _action,
        const std::string& _message
    );


private:
    using json = nlohmann::json;
    json jsonValue_ = {};
    std::list<LogData> logData_ = {};
    const std::string folderPath_ = "Log/";
    std::string fileName_ = "";

    std::string logPayload_ = "";
    bool enableAutoScroll_ = true;

    bool showDate_ = true;
    bool showTime_ = true;
    bool showFilename_ = true;
    bool showAction_ = true;
    bool showMessage_ = true;

private:
    JSONIO* pJsonio_ = nullptr;

    mutable std::mutex mutex_ = {};

private:
    Logger() = default;
    ~Logger() = default;
};

#define LOG_ERROR(message)   Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, message)
#define LOG_WARNING(message) Logger::GetInstance()->LogWarning(__FILE__, __FUNCTION__, message)
#define LOG_INFO(message)    Logger::GetInstance()->LogInfo(__FILE__, __FUNCTION__, message)
#define LOG_FATAL(message)   Logger::GetInstance()->LogFatal(__FILE__, __FUNCTION__, message)