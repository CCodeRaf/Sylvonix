#pragma once

#include <VulkanCommon.hpp>
#include <vector>
#include <string>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <ShaderParameter.hpp>

namespace SylvonixNative
{
    enum class ShaderType
    {
        Compute,
        Vertex,
        Fragment
    };

    class Shader
    {
    public:

        Shader(
            VkDevice device,
            slang::IGlobalSession* globalSession,
            const std::string& path,
            const std::string& name,
            ShaderType type
        );

        ~Shader();

        VkShaderModule GetModule() const
        {
            return module;
        }

        const std::vector<ShaderParameter>& GetParameters() const
        {
            return parameters;
        }

        const ShaderParameter* FindParameter(
            const std::string& name
        ) const;

    private:

        void Reflect(
            slang::ProgramLayout* layout
        );

    private:

        VkDevice device{};

        VkShaderModule module{};

        std::vector<ShaderParameter> parameters;
    };
}