#pragma once
#include <list>
#include <functional>


template <typename RETURN_TYPE = void, typename... ARGS>
class TaskExecutor
{
public:
    TaskExecutor() = default;
    ~TaskExecutor() = default;

    /// <summary>
    /// 初期化します。
    /// </summary>
    inline void Initialize()
    {

    }

    /// <summary>
    /// タスクを追加します。
    /// </summary>
    /// <param name="task">追加する実行可能な関数オブジェクト</param>
    inline void AddTask(const std::function<RETURN_TYPE(ARGS)> task)
    {
        taskList_.emplace_back(task);
    }

    /// <summary>
    /// すべてを登録順に実行します。
    /// </summary>
    inline void ExecuteAllOrdered()
    {
        for (const auto& task : taskList_)
        {
            task();
        }
        taskList_.clear();
    }

    /// <summary>
    /// 登録順に実行します。
    /// </summary>
    /// <param name="numExecTask">実行するタスクの数</param>
    inline void ExecuteOrdered(uint32_t numExecTask = 1u)
    {
        for (uint32_t i = 0; i < numExecTask; ++i)
        {
            if (taskList_.empty()) break;

            const auto& task = taskList_.front();
            task();
            taskList_.pop_back();
        }
    }


private:
    std::list<std::function<RETURN_TYPE(ARGS)>> taskList_ = {};
};
