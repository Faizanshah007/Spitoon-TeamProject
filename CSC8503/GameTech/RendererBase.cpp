#include "../../Plugins/PlayStation4/PS4RendererAPI.h"
#include "../../Plugins/PlayStation4/PS4Shader.h"
#include "../../Plugins/PlayStation4/PS4Mesh.h"
#include "RendererBase.h"
#include "../../Plugins/OpenGLRendering/OGLRendererAPI.h"
#include "../../Plugins/OpenGLRendering/OGLFrameBuffer.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"

#include "../../Common/SimpleFont.h"
#include "../../Common/TextureLoader.h"

#include "../../Common/MeshGeometry.h"
#include "../../Common/Assets.h"

using namespace NCL;
using namespace Rendering;

RendererBase::RendererBase() {
	rendererAPI = RendererAPI::GetInstance();
#ifdef _WIN64

	TextureLoader::RegisterAPILoadFunction(OGLTexture::RGBATextureFromFilename);

	font = new SimpleFont("PressStart2P.fnt", "PressStart2P.png");

	if (rendererAPI->HasInitialised()) {
		OGLTexture* t = (OGLTexture*)font->GetTexture();

		if (t) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		}
		debugShader = new OGLShader("debugVert.glsl", "debugFrag.glsl");
	}

	debugLinesMesh = new OGLMesh();
	debugTextMesh = new OGLMesh();
#endif
#ifdef _ORBIS

	TextureLoader::RegisterAPILoadFunction(PS4::PS4Texture::LoadTextureFromFile);

	font = new SimpleFont("PressStart2P.fnt", "PressStart2P.gnf");

	debugShader = PS4::PS4Shader::GenerateShader(
		Assets::SHADERDIR + "PS4/DebugVertex.sb",
		Assets::SHADERDIR + "PS4/DebugPixel.sb"
	);

	debugLinesMesh = new PS4::PS4Mesh();
	debugTextMesh = new PS4::PS4Mesh();
#endif
	debugLinesMesh->SetVertexPositions(std::vector<Vector3>(5000, Vector3()));
	debugLinesMesh->SetVertexColours(std::vector<Vector4>(5000, Vector4()));
	debugLinesMesh->SetVertexTextureCoords(std::vector<Vector2>(5000, Vector2()));
	debugLinesMesh->SetVertexNormals(std::vector<Vector3>(5000, Vector3()));
	debugLinesMesh->SetVertexTangents(std::vector<Vector4>(5000, Vector2()));

	debugTextMesh->SetVertexPositions(std::vector<Vector3>(5000, Vector3()));
	debugTextMesh->SetVertexColours(std::vector<Vector4>(5000, Vector4()));
	debugTextMesh->SetVertexTextureCoords(std::vector<Vector2>(5000, Vector2()));
	debugTextMesh->SetVertexNormals(std::vector<Vector3>(5000, Vector2()));
	debugTextMesh->SetVertexTangents(std::vector<Vector4>(5000, Vector2()));

	debugLinesMesh->UploadToGPU(rendererAPI);
	debugTextMesh->UploadToGPU(rendererAPI);

	debugLinesMesh->SetPrimitiveType(GeometryPrimitive::Lines);
	debugTextMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
}

RendererBase::~RendererBase() {
	delete font;

	delete debugLinesMesh;
	delete debugTextMesh;

	delete debugShader;
}

void RendererBase::Render() {
	rendererAPI->BeginFrame();
	rendererAPI->SetClearColour(0.2, 0.2, 0.2, 1);
	rendererAPI->SetCullFace(false);
	rendererAPI->EndFrame();
	DrawDebugData();
	rendererAPI->SwapBuffers();
	frameNumber = frameNumber + 1 % 6000;
}

void RendererBase::DrawString(const std::string& text, const Maths::Vector2& pos, const Maths::Vector4& colour, float size) {
	DebugString s;
	s.colour = colour;
	s.pos = pos;
	s.size = size;
	s.text = text;
	debugStrings.emplace_back(s);
}

void RendererBase::DrawLine(const Maths::Vector3& start, const Maths::Vector3& end, const Maths::Vector4& colour) {
	DebugLine l;
	l.start = start;
	l.end = end;
	l.colour = colour;
	debugLines.emplace_back(l);
}

void RendererBase::DrawDebugData() {

	if (debugStrings.empty() && debugLines.empty()) {
		return;
	}
	debugShader->BindShader();

	if (forceValidDebugState) {
		rendererAPI->SetBlend(true);
		rendererAPI->SetDepth(false);
		rendererAPI->SetCullFace(false);
	}

	font->GetTexture()->Bind(0);

	if (debugLines.size() > 0) {
		const Matrix4 pMat = SetupDebugLineMatrix();
		debugShader->UpdateUniformMatrix4("viewProjMatrix", pMat);
		debugShader->UpdateUniformInt("useTexture", 0);
		DrawDebugLines();
	}

	if (debugStrings.size() > 0) {
		const Matrix4 pMat = SetupDebugStringMatrix();
		debugShader->UpdateUniformMatrix4("viewProjMatrix", pMat);
		debugShader->UpdateUniformInt("useTexture", 1);
		DrawDebugStrings();
	}

	if (forceValidDebugState) {
		rendererAPI->SetBlend(false);
		rendererAPI->SetDepth(true);
		rendererAPI->SetCullFace(true);
	}
}

void RendererBase::DrawDebugStrings() {
	vector<Vector3> vertPos;
	vector<Vector2> vertTex;
	vector<Vector4> vertColours;

	if (debugStrings.size() > 100) {
		bool a = true;
	}

	for (DebugString& s : debugStrings) {
		font->BuildVerticesForString(s.text, s.pos, s.colour, s.size, vertPos, vertTex, vertColours);
	}

	debugTextMesh->SetVertexPositions(vertPos);
	debugTextMesh->SetVertexColours(vertColours);
	debugTextMesh->SetVertexTextureCoords(vertTex);
	debugTextMesh->SetVertexNormals(std::vector<Vector3>(vertPos.size(), Vector3()));
	debugTextMesh->SetVertexTangents(std::vector<Vector4>(vertPos.size(), Vector4()));

	debugTextMesh->UpdateGPUBuffers(0, vertPos.size());

	rendererAPI->DrawMesh(debugTextMesh);

	debugStrings.clear();
}

void RendererBase::DrawDebugLines() {
	vector<Vector3> vertPos;
	vector<Vector4> vertCol;

	int indices = 0;
	for (DebugLine& s : debugLines) {
		vertPos.emplace_back(s.start);
		vertPos.emplace_back(s.end);


		vertCol.emplace_back(s.colour);
		vertCol.emplace_back(s.colour);
	}


	debugLinesMesh->SetVertexPositions(vertPos);
	debugLinesMesh->SetVertexColours(vertCol);
	debugLinesMesh->SetVertexTextureCoords(std::vector<Vector2>(vertPos.size(), Vector2()));
	debugLinesMesh->SetVertexNormals(std::vector<Vector3>(vertPos.size(), Vector3()));
	debugLinesMesh->SetVertexTangents(std::vector<Vector4>(vertPos.size(), Vector4()));

	debugLinesMesh->UpdateGPUBuffers(0, vertPos.size());

	rendererAPI->DrawMesh(debugLinesMesh);

	debugLines.clear();
}



Maths::Matrix4 RendererBase::SetupDebugLineMatrix()	const {
	return Matrix4();
}

Maths::Matrix4 RendererBase::SetupDebugStringMatrix()	const {
	return Matrix4::Orthographic(-1, 1.0f, 100, 0, 0, 100);
}

