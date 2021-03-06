#pragma once
#include "../../Common/FrameBufferBase.h"
#include "OGLTexture.h"
namespace NCL {
    namespace Rendering {
        class OGLFrameBuffer : public FrameBufferBase
        {
        public:
            OGLFrameBuffer();
            ~OGLFrameBuffer();

            GLuint GetBufferObject() const { return frameBuffer; }
            void AddTexture() override;
            void AddTexture(int width, int height) override;
            void AddTexture(TextureBase* text) override;
            TextureBase* GetTexture() const override;
        protected:
            GLuint frameBuffer;
            OGLTexture* texture;
        };
    }
}

