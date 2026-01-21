#pragma once
#include <Math/Transform.h>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <Matrix4x4.h>
#include <cstdint>

#include <drawable/line/Line.h>
#include <Features/Primitive/AABB.h>

// ジョイント情報
struct JointData
{
    JointData() = default;

    QuaternionTransform transform = {};
    Vector3 worldTranslate = {};
    Matrix4x4 localMatrix = {};
    Matrix4x4 skeletonSpaceMatrix = {};
    std::string name;
    std::vector<int32_t> childrenIndices; // 子ジョイントのインデックス
    int32_t index = -1;
    std::optional<int32_t> parentIndex; // 親ジョイントのインデックス。ルートジョイントの場合はstd::nullopt
};

// ジョイント
class Joint
{
public:
    Joint() = default;
    ~Joint() = default;

    void Initialize();
    void Update();
    void DrawLine();

    // Getter
    const JointData& GetJointData() const;
    JointData& GetJointData();

private:
    JointData jointData_; // ジョイントデータ
    AABB pAabb_ = {}; // ジョイントを描画するためのAABB
};

// スケルトン情報
struct SkeletonData
{
    SkeletonData() = default;

    int32_t rootIndex = -1; // RootJoint の Index
    std::unordered_map<std::string, int32_t> jointMap; // ジョイント名をキーとするジョイントのマップ
    std::vector<Joint> joints; // 所属するJointのリスト
};

// スケルトン
class Skeleton
{
public:
    Skeleton() = default;
    ~Skeleton() = default;

    void Initialize();
    void Update();
    void DrawLine();

    // Getter
    const SkeletonData& GetSkeletonData() const;
    SkeletonData& GetSkeletonData();

private:
    SkeletonData skeletonData_; // スケルトンデータ
    Line lines_ = {0}; // ジョイント同士をつなぐ線
};
