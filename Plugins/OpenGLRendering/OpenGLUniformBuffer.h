#pragma once
#include "../../Common/UniformBuffer.h"

namespace NCL {
	namespace Rendering {
		class OpenGLUniformBuffer : UniformBuffer
		{
		public:
			OpenGLUniformBuffer(uint32_t size, uint32_t binding);
			virtual ~OpenGLUniformBuffer();

			virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		protected:
			uint32_t RendererID = 0;
		};
	}
}

