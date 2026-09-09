#pragma once
#include <Vector2.h>

enum class FlexDirection { Row, Column, RowReverse, ColumnReverse, Count };
enum class JustifyContent { FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly, Count };
enum class AlignItems { FlexStart, FlexEnd, Center, Count };

struct FlexItem
{
    Vector2 preferredSize;
};

struct FlexBox
{
    Vector2 position;
    Vector2 size;
};

struct FlexResult
{
    Vector2 position;
    Vector2 size;
};

struct CrossAlignResult
{
    float position;
    float size;
};
