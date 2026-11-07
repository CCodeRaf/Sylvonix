#include <Shader.hpp>
#include <DebugChecks.hpp>
#include <array>

namespace SylvonixNative
{
    Shader::Shader(
        VkDevice device,
        slang::IGlobalSession* globalSession,
        const std::string& path,
        const std::string& entryName,
        ShaderType type)
    {
        this->device = device;

        slang::TargetDesc targetDesc{};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = globalSession->findProfile("spirv_1_4");

        slang::CompilerOptionEntry compilerOption{
            slang::CompilerOptionName::EmitSpirvDirectly,
            {
                slang::CompilerOptionValueKind::Int, 1
            }
        };

        slang::SessionDesc sessionDesc{};

        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

        sessionDesc.compilerOptionEntries = &compilerOption;
        sessionDesc.compilerOptionEntryCount = 1;

        Slang::ComPtr<slang::ISession> session;
        SlangResult createResult = globalSession->createSession(sessionDesc, session.writeRef());

        if(SLANG_FAILED(createResult))
        {
            throw std::runtime_error(
                "Failed to create Slang session.");
        }

        auto* rawModule = session->loadModuleFromSource("Shader", path.c_str(), nullptr, nullptr);
        Slang::ComPtr<slang::IModule> slangModule(rawModule);

        if(!slangModule)
        {
            throw std::runtime_error(
                "Failed to load shader: " +
                path);
        }

        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        SlangResult entryPointResult = slangModule->findEntryPointByName(entryName.c_str(), entryPoint.writeRef());

        if(SLANG_FAILED(entryPointResult))
        {
            throw std::runtime_error(
                "Failed to find entry point: " +
                entryName);
        }

        std::cout
            << "Found entry point: "
            << entryName
            << "\n";

        std::array<slang::IComponentType*, 2> componentTypes = { slangModule, entryPoint };

        Slang::ComPtr<slang::IComponentType>
            composedProgram;

        SlangResult composeResult =
            session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                composedProgram.writeRef(),
                nullptr);

        if(SLANG_FAILED(composeResult))
        {
            throw std::runtime_error(
                "Failed to compose shader program.");
        }

        Reflect(composedProgram->getLayout());

    }

    Shader::~Shader()
    {
        std::cout << "exiting shader \n";
        if (module)
        {
            vkDestroyShaderModule(
                device,
                module,
                nullptr);

            module = VK_NULL_HANDLE;
        }
        std::cout << "shader destroyed\n";
    }

    const ShaderParameter*
    Shader::FindParameter(
        const std::string& name) const
    {
        for (const auto& parameter : parameters)
        {
            if (parameter.name == name)
            {
                return &parameter;
            }
        }

        return nullptr;
    }

    void Shader::Reflect(
    slang::ProgramLayout* layout)
    {
        parameters.clear();

        if(!layout)
        {
            return;
        }

        SlangInt parameterCount =
            layout->getParameterCount();

        std::cout << "===== REFLECTION =====\n";

        for(
            SlangInt i = 0;
            i < parameterCount;
            i++)
        {
            auto* parameter =
                layout->getParameterByIndex(i);

            auto* typeLayout =
                parameter->getTypeLayout();

            auto* type =
                typeLayout->getType();

            ShaderParameter p{};

            p.name = parameter->getName();

            if(parameter->getCategoryCount() > 0)
            {
                auto category =
                    parameter->getCategoryByIndex(0);

                p.binding =
                    static_cast<uint32_t>(
                        parameter->getOffset(category)
                    );

                p.set =
                    static_cast<uint32_t>(
                        parameter->getBindingSpace(category)
                    );
            }

            switch(type->getKind())
            {
                case slang::TypeReflection::Kind::ConstantBuffer:
                {
                    p.type =
                        ShaderParameterType::
                            UniformBuffer;
                    break;
                }

                case slang::TypeReflection::Kind::ShaderStorageBuffer:
                {
                    p.type =
                        ShaderParameterType::
                            StorageBuffer;
                    break;
                }

                case slang::TypeReflection::Kind::SamplerState:
                {
                    p.type =
                        ShaderParameterType::
                            Sampler;
                    break;
                }

                case slang::TypeReflection::Kind::Resource:
                {
                    auto access =
                        type->getResourceAccess();

                    p.type =
                        access ==
                        SLANG_RESOURCE_ACCESS_READ_WRITE
                        ?
                        ShaderParameterType::
                            StorageImage
                        :
                        ShaderParameterType::
                            SampledImage;

                    break;
                }

                default:
                {
                    std::cout << "Unsupported shader parameter type: " << std::string(parameter->getName()) << std::endl;
                }
            }

            std::cout
                << "Name: "
                << p.name
                << "\n";

            std::cout
                << "Kind: "
                << ShaderParamToString(p.type)
                << "\n";

            std::cout
                << "Shape: "
                << static_cast<int>(
                    type->getResourceShape())
                << "\n";

            std::cout
                << "Access: "
                << static_cast<int>(
                    type->getResourceAccess())
                << "\n";
            
            std::cout
                << "Binding: "
                << p.binding
                << "\n";

            std::cout
                << "Set: "
                << p.set
                << "\n";

            parameters.push_back(p);
        }

        std::cout << "======================\n";
    }
}