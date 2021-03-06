/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "../../Common/TextureBase.h"
#include "glad\glad.h"

#include <string>

namespace NCL {
	namespace Rendering {
		class OGLTexture : public TextureBase
		{
		public:
			//friend class OGLRendererAPI;
			 OGLTexture();
			 OGLTexture(GLuint texToOwn);
			~OGLTexture();

			void Bind(int slot = 0) const override;

			static TextureBase* RGBATextureFromData(char* data, int width, int height, int channels);

			static TextureBase* RGBATextureFromFilename(const std::string&name);

			static TextureBase* RGBATextureCubemapFromFilename(const std::string& side1, const std::string& side2, const std::string& side3,
				const std::string& side4, const std::string& side5, const std::string& side6);

			static TextureBase* RGBATextureEmpty(int width, int height);

			void ResetTexture() override;

			GLuint GetObjectID() const	{
				return texID;
			}
		protected:						
			GLuint texID;
		};
	}
}

