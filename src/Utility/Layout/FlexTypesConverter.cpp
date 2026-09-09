#include "FlexTypesConverter.h"



FlexItem ToFlexItem(const Sprite& sprite)
{
    return { sprite.GetSize() };
}
