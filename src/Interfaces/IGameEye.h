#pragma once

#include <Matrix4x4.h>
#include <Vector3.h>
#include <string>

class IGameEye
{
public:
    virtual ~IGameEye() = default;
    virtual void        Update() = 0;
    virtual Matrix4x4   GetViewProjectionMatrix() const = 0;
    virtual Vector3     GetPosition() const = 0;
    virtual Matrix4x4   GetWorldMatrix() const = 0;
    virtual const std::string& GetName() const = 0;
};