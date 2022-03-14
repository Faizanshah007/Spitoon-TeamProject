#ifdef _ORBIS
#include "PS4UniformBuffer.h"
#include "PS4Shader.h"
#include "../../CSC8503/GameTech/RendererBase.h"
using namespace NCL::PS4;

PS4UniformBuffer::PS4UniformBuffer(uint32_t size) {
	data = (void*)((PS4RendererAPI*)RendererBase::rendererAPI)->onionAllocator->allocate(size, Gnm::kEmbeddedDataAlignment4);

	constantBuffer.initAsConstantBuffer(data, size);
	constantBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeSC);
}

PS4UniformBuffer::~PS4UniformBuffer() {

}

void PS4UniformBuffer::SetData(const void* value, uint32_t size, uint32_t offset) {
	((PS4RendererAPI*)RendererBase::rendererAPI)->currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, 0, 1, &constantBuffer);
	memcpy(data, value, size);
}
#endif