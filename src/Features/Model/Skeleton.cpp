#include "./Skeleton.h"

void Joint::Initialize()
{
    pAabb_.Initialize();
}

void Joint::Update()
{
    static const Vector3 jointSize(0.01f, 0.01f, 0.01f);

    Vector3& worldTranslate = jointData_.worldTranslate;

    worldTranslate = jointData_.skeletonSpaceMatrix.GetTranslation();

    // AABBの更新
    Vector3 min = worldTranslate - jointSize * 0.5f;
    Vector3 max = worldTranslate + jointSize * 0.5f;
    pAabb_.SetMinMax(min, max);
}

void Joint::DrawLine()
{
    pAabb_.Draw1F();
}

const JointData& Joint::GetJointData() const
{
    return jointData_;
}

JointData& Joint::GetJointData()
{
    return jointData_;
}

const SkeletonData& Skeleton::GetSkeletonData() const
{
    return skeletonData_;
}

SkeletonData& Skeleton::GetSkeletonData()
{
    return skeletonData_;
}

void Skeleton::Initialize()
{
    for (Joint& joint : skeletonData_.joints)
    {
        joint.Initialize();
    }

    lines_.Initialize();
}

void Skeleton::Update()
{
    if (lines_.GetLineCount() != (skeletonData_.joints.size() - 1))
    {
        // ジョイントの数に合わせて線の数を調整
        lines_.ResizeLine(skeletonData_.joints.size() - 1);
    }

    for (Joint& joint : skeletonData_.joints)
    {
        joint.Update();
    }

    uint32_t lineIndex = 0;
    for (auto& joint : skeletonData_.joints)
    {
        const JointData& jointData = joint.GetJointData();
        if (jointData.parentIndex.has_value())
        {
            const Joint& parent = skeletonData_.joints[*jointData.parentIndex];
            lines_[lineIndex++] = parent.GetJointData().worldTranslate;
            lines_[lineIndex++] = joint.GetJointData().worldTranslate;
        }
    }

    lines_.Update();
}

void Skeleton::DrawLine()
{
    for (Joint& joint : skeletonData_.joints)
    {
        joint.DrawLine();
    }

    lines_.Draw1F();
}