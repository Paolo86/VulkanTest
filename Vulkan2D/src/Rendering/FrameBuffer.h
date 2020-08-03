#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class FrameBuffer
{
public:

	FrameBuffer() {}

	void Destroy(VkDevice device)
	{
		vkDestroyFramebuffer(device, m_frameBuffer, nullptr);

	}
	void Create(VkDevice device, VkRenderPass renderPass, uint32_t width, uint32_t height, std::vector<VkImageView> attachments, uint32_t layers = 1)
	{
		m_attachments = attachments;
		this->width = width;
		this->height = height;

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = m_attachments.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = layers;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

	}
	std::vector<VkImageView>& GetAttachments() { return m_attachments; };
	VkFramebuffer GetVkFrameBuffer() { return m_frameBuffer; }

private:
	std::vector<VkImageView> m_attachments;
	uint32_t width;
	uint32_t height;
	VkFramebuffer m_frameBuffer;
};