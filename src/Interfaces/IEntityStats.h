#pragma once

class IEntityStats
{
public:
    virtual ~IEntityStats() = default;
    virtual void OnCollision(const IEntityStats* status) = 0;
    virtual float GetDamage() const = 0;
};