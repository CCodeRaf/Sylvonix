#include <ComputePipeline.hpp>

#include <DebugChecks.hpp>

namespace SylvonixNative
{
    ComputePipeline::ComputePipeline(
        VkDevice device,
        const ComputePipelineDescriptor& descriptor)
    {
        this->device = device;
        this->shader = descriptor.shader;

        BuildDescriptorSetLayout();

        AllocateDescriptors();

        VkPipelineLayoutCreateInfo layoutCI{
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts =
                &descriptorSetLayout
        };

        chk(vkCreatePipelineLayout(
            device,
            &layoutCI,
            nullptr,
            &pipelineLayout
        ));

        const auto& spirv =
            shader->GetSpirv(
                "main",
                SVShaderStageType::Compute
            );

        VkShaderModuleCreateInfo moduleCI{
            .sType =
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize =
                spirv.size()
                    * sizeof(uint32_t),
            .pCode =
                spirv.data()
        };

        chk(vkCreateShaderModule(
            device,
            &moduleCI,
            nullptr,
            &shaderModule
        ));

        VkPipelineShaderStageCreateInfo stageCI{
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage =
                VK_SHADER_STAGE_COMPUTE_BIT,
            .module =
                shaderModule,
            .pName =
                "main"
        };

        VkComputePipelineCreateInfo pipelineCI{
            .sType =
                VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage =
                stageCI,
            .layout =
                pipelineLayout
        };

        chk(vkCreateComputePipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineCI,
            nullptr,
            &pipeline
        ));
    }

    ComputePipeline::~ComputePipeline()
    {
        if (pipeline)
        {
            vkDestroyPipeline(
                device,
                pipeline,
                nullptr
            );
        }

        if (pipelineLayout)
        {
            vkDestroyPipelineLayout(
                device,
                pipelineLayout,
                nullptr
            );
        }

        if (descriptorPool)
        {
            vkDestroyDescriptorPool(
                device,
                descriptorPool,
                nullptr
            );
        }

        if (descriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout(
                device,
                descriptorSetLayout,
                nullptr
            );
        }
    }

    void ComputePipeline::Set(
        const std::string& name,
        Image* image)
    {
        resources[name].image = image;
    }

    void ComputePipeline::Set(
        const std::string& name,
        Buffer* buffer)
    {
        resources[name].buffer = buffer;
    }

    void ComputePipeline::Dispatch(
        VkCommandBuffer cb,
        uint32_t x,
        uint32_t y,
        uint32_t z)
    {
        UpdateDescriptors();

        vkCmdBindPipeline(
            cb,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipeline
        );

        vkCmdBindDescriptorSets(
            cb,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout,
            0,
            1,
            &descriptorSet,
            0,
            nullptr
        );

        vkCmdDispatch(
            cb,
            x,
            y,
            z
        );
    }

    void ComputePipeline::BuildDescriptorSetLayout()
    {
        VkDescriptorSetLayoutCreateInfo ci{
            .sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 0,
            .pBindings = nullptr
        };

        chk(vkCreateDescriptorSetLayout(
            device,
            &ci,
            nullptr,
            &descriptorSetLayout
        ));
    }

    void ComputePipeline::AllocateDescriptors()
    {
        VkDescriptorPoolCreateInfo poolCI{
            .sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = 0,
            .maxSets = 1,
            .poolSizeCount = 0,
            .pPoolSizes = nullptr
        };

        chk(vkCreateDescriptorPool(
            device,
            &poolCI,
            nullptr,
            &descriptorPool
        ));

        VkDescriptorSetAllocateInfo allocCI{
            .sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool =
                descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts =
                &descriptorSetLayout
        };

        chk(vkAllocateDescriptorSets(
            device,
            &allocCI,
            &descriptorSet
        ));
    }

    void ComputePipeline::UpdateDescriptors()
    {
        /*
            TODO:

            Rebuild descriptor reflection
            using SylvonixShader resources.

            Future code will use:

            shader->GetResources()

            and eventually:

            resource.GetType()

            once resource types are added
            to the SVShader format.
        */
    }
}