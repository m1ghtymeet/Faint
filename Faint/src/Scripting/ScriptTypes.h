#pragma once
#include <string>
#include <variant>
#include <Math/Types.h>

namespace Moon { class Entity; }

enum class ExposedVarType {
    Bool,
    Int,
    Float,
    String,
    Vector2,
    Vector3,
    Vector4,
    Entity,
    Prefab,
    Unsupported
};

struct ExposedVariable {
    std::string name;
    ExposedVarType type;
    std::variant<
        bool,
        int,
        float,
        std::string,
        Vector3,
        Moon::Entity*,
        std::string
    > value;

    ExposedVariable(const std::string& n, ExposedVarType t) : name(n), type(t) {}
};