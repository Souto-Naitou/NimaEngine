#include "Logger.h"
#include <chrono>
#include <format>
#include <fstream>

#include <debugapi.h>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG


void Logger::Initialize()
{
    pJsonio_ = JSONIO::GetInstance();

    /// ルートを作成
    jsonValue_ = json::object();

    /// Logs配列を作成
    jsonValue_["Logs"] = json::array();


    /// ファイル名を設定
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    fileName_ = std::format("{:%Y%m%d_%H%M%S}", now_sec);
}

void Logger::Save()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::system_clock::now();

    std::string filepath = folderPath_ + fileName_;
    pJsonio_->Save(filepath + ".json", jsonValue_);

    std::string filePayload;
    for (auto& log : logData_)
    {
        filePayload += "[" + log.status + "] ";
        filePayload += log.date + " ";
        filePayload += log.time + ", ";
        filePayload += log.filename + ", ";
        filePayload += log.action + ", ";
        filePayload += log.message + "\n";
    }

    /// 改行を削除
    filePayload.erase(filePayload.end() - 1);

    std::ofstream ofs(filepath + ".log");
    ofs << filePayload;
    ofs.close();

    logPayload_ = filePayload;
}

void Logger::DrawUI()
{
    #ifdef _DEBUG

    ImGuiWindowFlags flag = {};
    flag |= ImGuiWindowFlags_HorizontalScrollbar;
    if (enableAutoScroll_)
    {
        flag = ImGuiWindowFlags_NoScrollWithMouse;
        flag |= ImGuiWindowFlags_NoScrollbar;
    }

    if (ImGui::Begin("Log", nullptr, flag))
    {
        ImGui::Checkbox("Auto Scroll", &enableAutoScroll_);

        ImGui::Checkbox("Date", &showDate_);
        ImGui::SameLine();
        ImGui::Checkbox("Time", &showTime_);
        ImGui::SameLine();
        ImGui::Checkbox("Filename", &showFilename_);
        ImGui::SameLine();
        ImGui::Checkbox("Action", &showAction_);
        ImGui::SameLine();
        ImGui::Checkbox("Message", &showMessage_);

        ImGui::BeginChild("LogChild", ImVec2(-1, -1), ImGuiChildFlags_Border, flag);

        // ImGuiListClipperを使用して高速化
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(logData_.size()));

        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                const auto& log = *(std::next(logData_.begin(), i));
                std::string line;

                if (showDate_)     line += "[" + log.date + "] ";
                if (showTime_)     line += "[" + log.time + "] ";
                if (showFilename_) line += "[" + log.filename + "] ";
                if (showAction_)   line += "[" + log.action + "] ";

                /// ステータスに応じて色を設定
                ImVec4 color = ImVec4(1, 1, 1, 1);
                if (!log.status.empty())
                {
                    if (log.status == "Info")    color = ImVec4(0.6f, 0.8f, 1.0f, 1.0f);
                    if (log.status == "Warning") color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);
                    if (log.status == "Error")   color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("[%s]", log.status.c_str());
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                }

                if (showMessage_)
                    line += log.message;

                // 一行全体を色付きで描画
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            }
        }
        clipper.End();

        if (enableAutoScroll_)
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
    }
    ImGui::End();

    #endif // _DEBUG
}

void Logger::LogError(const std::string& filename, const std::string& action, const std::string& message)
{
    Log("Error", filename, action, message);
}

void Logger::LogWarning(const std::string& filename, const std::string& action, const std::string& message)
{
    Log("Warning", filename, action, message);
}

void Logger::LogInfo(const std::string& filename, const std::string& action, const std::string& message)
{
    Log("Info", filename, action, message);
}

void Logger::LogFatal(const std::string& filename, const std::string& action, const std::string& message)
{
    Log("Fatal", filename, action, message);
}

void Logger::LogForOutput(const std::string& message)
{
    OutputDebugStringA(message.c_str());
}

void Logger::Log(const std::string& status, const std::string& filename, const std::string& action, const std::string& message)
{
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);


    std::string date = std::format("{:%Y/%m/%d}", now_sec);
    std::string time = std::format("{:%H:%M:%S}", now_sec);

    LogData data = {};
    data.date = date;
    data.time = time;
    data.status = status;
    data.filename = filename;
    data.action = action;
    data.message = message;


    {
        std::lock_guard<std::mutex> lock(mutex_);
        logData_.emplace_back(data);
        LogJson(date, time, status, filename, action, message);
    }

    OutputDebugStringA(std::format("[{}] {} {}, {}, {}, {}\n", status, date, time, filename, action, message).c_str());

    this->Save();
}

void Logger::LogJson(const std::string& date, const std::string& time, const std::string& status, const std::string& filename, const std::string& action, const std::string& message)
{
    auto object = json::object();
    object["Date"] = date;
    object["Time"] = time;
    object["Status"] = status;
    object["ClassName"] = filename;
    object["Action"] = action;
    object["Message"] = message;

    jsonValue_["Logs"].push_back(object);
}
