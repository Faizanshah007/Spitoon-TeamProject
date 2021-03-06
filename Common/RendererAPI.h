/******************************************************************************
Class:RendererAPI
Implements:
Author:Rich Davison
Description:TODO

-_-_-_-_-_-_-_,------,
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Window.h"
#include "../Common/Vector3.h"
#include "../Common/Vector4.h"
#include "../Common/Matrix4.h"
#include "../Common/FrameBufferBase.h"
namespace NCL {
	class MeshGeometry;

	namespace Rendering {
		class ShaderBase;
		class TextureBase;

		enum class VerticalSyncState {
			VSync_ON,
			VSync_OFF,
			VSync_ADAPTIVE
		};
		class RendererAPI {
		public:
			friend class NCL::Window;
			static RendererAPI* GetInstance();
			virtual ~RendererAPI();
			virtual bool HasInitialised() const { return true; }
			virtual void Update(float dt) {}
			virtual bool SetVerticalSync(VerticalSyncState s) {
				return false;
			}

			virtual void BeginFrame() = 0;
			virtual void RenderFrame() = 0;
			virtual void EndFrame() = 0;
			virtual void SwapBuffers() = 0;

			// Renderer commands
			virtual void DrawMesh(MeshGeometry* mesh) = 0;
			virtual void DrawMeshAndSubMesh(MeshGeometry* mesh) = 0;

			virtual void BindFrameBuffer() = 0;
			virtual void BindFrameBuffer(const FrameBufferBase* fbo) = 0;

			virtual void SetDepth(bool depth) = 0;

			enum class BlendType {
				NONE,
				ONE,
				ALPHA,
				ONE_MINUS_ALPHA,
				SRC_COLOR,
				ONE_MINUS_SRC_COLOR,

			};
			virtual void SetBlend(bool blend, BlendType srcFunc = BlendType::ONE, BlendType dstFunc = BlendType::NONE) = 0;

			enum CULL_TYPE {
				FRONT,
				BACK,
			};

			virtual void SetCullFace(bool cull) = 0;
			virtual void SetCullType(CULL_TYPE type) = 0;

			virtual void SetViewportSize(int x, int y) = 0;

			virtual void ClearBuffer(bool color, bool depth, bool stencil) = 0;
			virtual void SetClearColour(float r, float g, float b, float a) = 0;
			virtual void SetColourMask(bool r, bool g, bool b, bool a) = 0;

			int GetCurrentWidth() { return currentWidth; }
			int GetCurrentHeight() { return currentHeight; }

			bool GetInitState() { return initState; }

		protected:
			RendererAPI(Window& w);
			virtual void OnWindowResize(int w, int h) = 0;
			virtual void OnWindowDetach() {}; //Most renderers won't care about this
			
			static RendererAPI* activeAPI;

			Window& hostWindow;

			int currentWidth;
			int currentHeight;

			bool initState;
		};
	}
}
