#pragma once

#include <typeindex>
#include <any>
#include <unordered_map>

namespace Moon::Rendering::Data {
    
    class Describable {
    public:
        template<typename T>
        void AddDescriptor(T&& p_descriptor);

        template<typename T>
        void RemoveDescriptor();

        void ClearDescriptors();

        template<typename T>
        bool HasDescriptor() const;

        template<typename T>
        T& GetDescriptor();

    private:
        std::unordered_map<std::type_index, std::any> m_descriptors;
    };
}

#include "Describable.inl"