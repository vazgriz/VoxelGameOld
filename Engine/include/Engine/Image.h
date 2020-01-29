#pragma once
#include <vk_mem_alloc.h>
#include <memory>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    class Engine;

    struct ImageState {
        Engine* engine;
        vk::Image image;
        VmaAllocation allocation;

        ImageState(Engine* engine, vk::Image&& image, VmaAllocation allocation);
        ImageState(const ImageState& other) = delete;
        ImageState& operator = (const ImageState& other) = delete;
        ImageState(ImageState&& other);
        ImageState& operator = (ImageState&& other);
        ~ImageState();
    };

    class Image {
    public:
        Image(Engine& engine, const vk::ImageCreateInfo& info, const VmaAllocationCreateInfo& allocInfo);
        ~Image();

        vk::Image& image() const { return m_imageState->image; }
        vk::Extent3D extent() const { return m_imageState->image.extent(); }

    private:
        Engine* m_engine;
        std::unique_ptr<ImageState> m_imageState;
        VmaAllocationInfo m_allocationInfo;
    };
}