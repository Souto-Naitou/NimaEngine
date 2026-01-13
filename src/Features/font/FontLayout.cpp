#include "FontLayout.h"

std::vector<FontLayout::Result> FontLayout::Compute(std::span<GlyphInfo> glyphs)
{
    auto result = std::vector<Result>(glyphs.size());
    Vector2 penPosition = { 0.0f, 0.0f };

    for (size_t i = 0; i < glyphs.size(); ++i)
    {
        result[i].leftTop = properties_.leftTop + penPosition;
        result[i].leftTop.y -= glyphs[i].size.y * properties_.anchorPoint.y;
        penPosition.x += glyphs[i].size.x + properties_.letterSpacing;
    }

    widthOverall = penPosition.x - properties_.letterSpacing;

    for (auto& res : result)
    {
        res.leftTop.x -= widthOverall * properties_.anchorPoint.x;
    }

    return result;
}
