#pragma once

#include <string>
#include <vulkan/vulkan.h>

namespace SylvonixNative
{
    enum class ShaderParameterType
    {
        UniformBuffer,
        StorageBuffer,

        SampledImage,
        StorageImage,

        Sampler
    };

    inline const char* ShaderParamToString(
        ShaderParameterType type)
    {
        switch(type)
        {
            case ShaderParameterType::UniformBuffer:
                return "UniformBuffer";

            case ShaderParameterType::StorageBuffer:
                return "StorageBuffer";

            case ShaderParameterType::SampledImage:
                return "SampledImage";

            case ShaderParameterType::StorageImage:
                return "StorageImage";

            case ShaderParameterType::Sampler:
                return "Sampler";

            default:
                return "Unknown";
        }
    }

    struct ShaderParameter
    {
        std::string name;

        uint32_t binding{};
        uint32_t set{};

        uint32_t size{};

        ShaderParameterType type{};
    };
}