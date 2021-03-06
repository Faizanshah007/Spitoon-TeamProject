#ifdef _ORBIS
#include "../../Plugins/PlayStation4/PS4Mesh.h"
#include "../../Plugins/PlayStation4/PS4Shader.h"
#include "../../Plugins/PlayStation4/PS4Texture.h"
#include "../../Plugins/PlayStation4/PS4UniformBuffer.h"
#include "../../Plugins/PlayStation4/PS4FrameBuffer.h"
#endif
#include "Renderer.h"

#include "../../Common/Assets.h"
#include "../../Common/SimpleFont.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/MeshGeometry.h"
#include "../CSC8503Common/GameManager.h"
#include "../CSC8503Common/RenderObject.h"


#ifdef _WIN64
#include "../../Plugins/OpenGLRendering/OGLFrameBuffer.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Plugins/OpenGLRendering/OGLUniformBuffer.h"
#endif
using namespace NCL;
using namespace Maths;
using namespace Rendering;
using namespace CSC8503;

Renderer::Renderer(GameWorld& world) : RendererBase(), gameWorld(world), drawGUI(false) {
#ifdef _WIN64

	shadowShader = new OGLShader("GameTechShadowVert.glsl", "GameTechShadowFrag.glsl");

	rendererAPI->SetDepth(true);
	rendererAPI->SetClearColour(1, 1, 1, 1);

	//Skybox!
	skyboxShader = new OGLShader("skyboxVertex.glsl", "skyboxFragment.glsl");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	uiBarMesh = new OGLMesh();


	uiCrosshairMesh = new OGLMesh();

	//ui = new RenderObject(nullptr, uiMesh, nullptr, uiShader);

	ForceValidDebugState(true);

	skyboxTex = OGLTexture::RGBATextureCubemapFromFilename(
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	);
	
	shadowFBO = new OGLFrameBuffer();
	shadowFBO->AddTexture();

	maskShader = new OGLShader("MaskVertex.glsl", "MaskFragment.glsl");

	uiBarShader = new OGLShader("UIBarVert.glsl", "UIBarFrag.glsl");


	crosshairTex = OGLTexture::RGBATextureFromFilename("crosshair.png");
	uiCrosshairShader = new OGLShader("UICrosshairVert.glsl", "UICrosshairFrag.glsl");

	normalTex = OGLTexture::RGBATextureFromFilename("noise.png");


	camBuffer = new OGLUniformBuffer(sizeof(CameraMatrix), 0);

#endif
#ifdef _ORBIS
	skyboxMesh = PS4::PS4Mesh::GenerateQuad();
	skyboxMesh->UploadToGPU(rendererAPI);

	skyboxShader = PS4::PS4Shader::GenerateShader(
		Assets::SHADERDIR + "PS4/skyboxVertex.sb",
		Assets::SHADERDIR + "PS4/skyboxPixel.sb"
	);

	skyboxTex = PS4::PS4Texture::LoadSkyboxFromFile(NCL::Assets::TEXTUREDIR + "Cubemap/cubemap.gnf");
	normalTex = PS4::PS4Texture::LoadTextureFromFile(NCL::Assets::TEXTUREDIR + "noise.gnf");

	maskShader = PS4::PS4Shader::GenerateShader(
		Assets::SHADERDIR + "PS4/maskVertex.sb",
		Assets::SHADERDIR + "PS4/maskPixel.sb"
	);

	uiBarShader = PS4::PS4Shader::GenerateShader(
		Assets::SHADERDIR + "PS4/UIBarVert.sb",
		Assets::SHADERDIR + "PS4/UIBarPixel.sb"
	);

	crosshairTex = PS4::PS4Texture::LoadTextureFromFile(NCL::Assets::TEXTUREDIR + "crosshair.gnf");
	uiCrosshairShader = PS4::PS4Shader::GenerateShader(
		Assets::SHADERDIR + "PS4/UICrosshairVert.sb",
		Assets::SHADERDIR + "PS4/UICrosshairPixel.sb"
	);
	//crosshairTex = OGLTexture::RGBATextureFromFilename("crosshair.png");
	//uiCrosshairShader = new OGLShader("UICrosshairVert.glsl", "UICrosshairFrag.glsl");

	uiBarMesh = new PS4::PS4Mesh();
	uiCrosshairMesh = new PS4::PS4Mesh();

	camBuffer = new PS4::PS4UniformBuffer(sizeof(CameraMatrix));
#endif

	uiBarMesh->SetVertexPositions({ Vector3(-0.5f, 0.2f,-0.5f), Vector3(-0.5f,0.1f,-0.5f) , Vector3(0.5f,0.1f,-0.5f) , Vector3(0.5f,0.2f,-0.5f) });
	uiBarMesh->SetVertexTextureCoords({ Vector2(0,1), Vector2(0,0), Vector2(1,0) , Vector2(1,1) });
	uiBarMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	uiBarMesh->SetVertexTangents(std::vector<Vector4>(4, Vector4()));
	uiBarMesh->SetVertexNormals(std::vector<Vector3>(4, Vector3()));
	uiBarMesh->UploadToGPU();

	uiCrosshairMesh->SetVertexPositions({ Vector3(-0.025f, 0.2f,-0.025f), Vector3(-0.025f,0.1f,-0.025f) , Vector3(0.025f,0.1f,-0.025f) , Vector3(0.025f,0.2f,-0.025f) });
	uiCrosshairMesh->SetVertexTextureCoords({ Vector2(0,1), Vector2(0,0), Vector2(1,0) , Vector2(1,1) });
	uiCrosshairMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	uiCrosshairMesh->SetVertexTangents(std::vector<Vector4>(4, Vector4()));
	uiCrosshairMesh->SetVertexNormals(std::vector<Vector3>(4, Vector3()));
	uiCrosshairMesh->UploadToGPU();

	//Set up the light properties
	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPos = Vector3(-200.0f, 200.0f, -200.0f);
}

Renderer::~Renderer() {
	delete camBuffer;

	delete shadowFBO;
	delete shadowShader;

	delete skyboxShader;
	delete skyboxMesh;
	delete skyboxTex;

	delete uiBarMesh;
	delete uiBarShader;

	delete uiCrosshairMesh;
	delete uiCrosshairShader;

	delete normalTex;

	delete maskShader;
}

void Renderer::Render() {
	rendererAPI->BeginFrame();

	rendererAPI->SetCullFace(true);
	rendererAPI->SetClearColour(1, 1, 1, 1);
	BuildObjectList();

	ApplyPaintToMasks();
	RenderScene();
	rendererAPI->SetCullFace(false);

	rendererAPI->EndFrame();
	DrawDebugData();
	rendererAPI->SwapBuffers();
	frameNumber = frameNumber + 1 % 6000;
}

void Renderer::BuildObjectList() {
	activeObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				const RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.emplace_back(g);
				}
			}
		}
	);
}

void Renderer::RenderScene() {
#ifdef _WIN64
	RenderShadows();
#endif
	// Set scene uniform buffers
	const float screenAspect = (float)rendererAPI->GetCurrentWidth() / (float)rendererAPI->GetCurrentHeight();
	camMatrix.projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);
	camMatrix.viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	camBuffer->SetData(&camMatrix, sizeof(CameraMatrix));

	RenderSkybox();
	RenderObjects();
	if (drawGUI) { DrawGUI(); }

}

void Renderer::RenderShadows() {
	rendererAPI->SetCullFace(true);
	rendererAPI->SetBlend(true);
	rendererAPI->SetDepth(true);

	rendererAPI->BindFrameBuffer(shadowFBO);

	rendererAPI->ClearBuffer(false, true, false);
	rendererAPI->SetColourMask(false, false, false, false);
	rendererAPI->SetViewportSize(4096, 4096);
	rendererAPI->SetCullType(NCL::Rendering::RendererAPI::CULL_TYPE::FRONT);

	shadowShader->BindShader();

	const Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(lightPos, Vector3(0, 0, 0), Vector3(1, 1, 1));
	const Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);
	const Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;
	const Matrix4 biasMat = Matrix4::Translation(Vector3(0.5, 0.5, 0.5)) * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));

	shadowMatrix = biasMat * mvMatrix;

	for (const auto& i : activeObjects) {
		const Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		const Matrix4 mvpMatrix = mvMatrix * modelMatrix;
		shadowShader->UpdateUniformMatrix4("mvpMatrix", mvpMatrix);

		rendererAPI->DrawMeshAndSubMesh((*i).GetMesh());
	}
	rendererAPI->SetViewportSize(rendererAPI->GetCurrentWidth(), rendererAPI->GetCurrentHeight());
	rendererAPI->SetColourMask(true, true, true, true);
	rendererAPI->BindFrameBuffer();
	rendererAPI->SetCullType(NCL::Rendering::RendererAPI::CULL_TYPE::BACK);
}

void Renderer::RenderSkybox() {
	rendererAPI->SetCullFace(false);
	rendererAPI->SetBlend(false);
	rendererAPI->SetDepth(false);

	skyboxShader->BindShader();

	skyboxShader->UpdateUniformMatrix4("invProjMatrix", camMatrix.projMatrix.Inverse());

	skyboxTex->Bind(0);

	rendererAPI->DrawMesh(skyboxMesh);

	rendererAPI->SetCullFace(true);
	rendererAPI->SetBlend(true);
	rendererAPI->SetDepth(true);
}

void Renderer::RenderObjects() {
	ShaderBase* activeShader = nullptr;

	for (const auto& i : activeObjects) {
		ShaderBase* shader = (*i).GetShader();

		if (activeShader != shader) {
			shader->BindShader();

			shader->UpdateUniformMatrix4("viewProjMatrix", camMatrix.projMatrix * camMatrix.viewMatrix);
			shader->UpdateUniformMatrix4("invProjMatrix", camMatrix.projMatrix.Inverse());
			shader->UpdateUniformVector3("cameraPos", gameWorld.GetMainCamera()->GetPosition());

			shader->UpdateUniformVector4("lightColour", lightColour);
			shader->UpdateUniformVector3("lightPos", lightPos);
			shader->UpdateUniformFloat( "lightRadius", lightRadius);

			activeShader = shader;
		}

		if ((*i).GetDefaultTexture()) (*i).GetDefaultTexture()->Bind(0);
		shader->UpdateUniformInt("hasTexture", (*i).GetDefaultTexture() ? 1 : 0);
		if ((*i).GetNormalMap()) (*i).GetNormalMap()->Bind(3);
		shader->UpdateUniformInt("hasNormal", (*i).GetNormalMap() ? 1 : 0);
		normalTex->Bind(4);
#ifdef _WIN64
		if (shadowFBO->GetTexture()) shadowFBO->GetTexture()->Bind(1);
#endif
		if ((*i).GetPaintMask()) {
			(*i).GetPaintMask()->Bind(2);
		}
		shader->UpdateUniformInt("hasPaintMask", (*i).GetPaintMask() ? 1 : 0);


		const Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
#ifdef _WIN64
		const Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
#endif
		shader->UpdateUniformMatrix4("modelMatrix", modelMatrix);
		shader->UpdateUniformMatrix4("invModelMatrix", modelMatrix.Inverse());
#ifdef _WIN64
		shader->UpdateUniformMatrix4("shadowMatrix", fullShadowMat);
#endif
		shader->UpdateUniformVector4("objectColour", i->GetColour());
		shader->UpdateUniformInt("hasVertexColours", !(*i).GetMesh()->GetColourData().empty());

		rendererAPI->DrawMeshAndSubMesh((*i).GetMesh());
	}
}

void NCL::Rendering::Renderer::ClearPaint()
{
	GameObjectIterator start;
	GameObjectIterator end;
	gameWorld.GetPaintableObjectIterators(start, end);

	for (auto it = start; it != end; ++it){
		if (!(*it)->GetRenderObject()) return;

		if (!(*it)->GetRenderObject()->GetPaintMask()) return;

		(*it)->GetRenderObject()->GetPaintMask()->ResetTexture();
	}
}

NCL::Maths::Vector2 Renderer::CountPaintMask(TextureBase* paintMask, NCL::Maths::Vector2 prevScores, NCL::Maths::Vector4 team1Colour, NCL::Maths::Vector4 team2Colour) {

#ifdef _ORBIS
	return prevScores;
#elif _WIN64

	const int pixelDataSize = paintMask->GetHeight() * paintMask->GetWidth() * 4;
	GLubyte* data = new GLubyte[pixelDataSize];
	glGetTextureImage(((OGLTexture*)paintMask)->GetObjectID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelDataSize * 4, data);

	float team1Score = 0;
	float team2Score = 0;

	//Read data from paint mask for scoring
	for (size_t x= 0;x < paintMask->GetWidth(); x++){
		for (size_t y = 0; y < paintMask->GetHeight(); y++) {
			float r, g, b, a;

			size_t elmes_per_line = paintMask->GetWidth() * 4;

			size_t row = y * elmes_per_line;
			size_t col = x * 4;

			r = static_cast<float>(data[row + col] / 255.0f);
			g = static_cast<float>(data[row + col+1] / 255.0f);
			b = static_cast<float>(data[row + col+2] / 255.0f);
			a = static_cast<float>(data[row + col+3] / 255.0f);
			
			if (a != 1.0f) {
				continue;
			}

			//Check if the colour is closer to team a or team b
			Vector3 curCol = Vector3(r, g, b);
			Vector3 team1Pixel = curCol - Vector3(team1Colour.x, team1Colour.y, team1Colour.z);
			Vector3 team2Pixel = curCol - Vector3(team2Colour.x, team2Colour.y, team2Colour.z);

			(team1Pixel.LengthSquared() < team2Pixel.LengthSquared()) ? (team1Score++) : (team2Score++);
		}
		
	}
	delete[] data;
	return Vector2(team1Score - prevScores.x , team2Score - prevScores.y);
#endif
}

void Renderer::Paint(const RenderObject* paintable, NCL::Maths::Vector3& barycentric, NCL::Maths::Vector3& colpos, NCL::Maths::Vector2& texUV_a, NCL::Maths::Vector2& texUV_b, NCL::Maths::Vector2& texUV_c, float radius, float hardness, float strength, NCL::Maths::Vector4 colour)
{
	PaintInstance pi;
	pi.object = paintable;
	pi.barycentric = barycentric;
	pi.colPoint = colpos;
	pi.texUV_a = texUV_a;
	pi.texUV_b = texUV_b;
	pi.texUV_c = texUV_c;
	pi.radius = radius;
	pi.hardness = hardness;
	pi.strength = strength;
	pi.colour = colour;

	paintInstances.push_back(pi);
}

void Renderer::ApplyPaintToMasks() {
	rendererAPI->SetDepth(false);
	rendererAPI->SetBlend(true, RendererAPI::BlendType::ONE, RendererAPI::BlendType::ONE_MINUS_ALPHA);

	maskShader->BindShader();

	Vector2 currentSize;
	for (const auto& i : paintInstances) {
		if (i.object->GetPaintMask() == nullptr) continue;
#ifdef _WIN64
		OGLFrameBuffer maskFBO;
#elif _ORBIS
		PS4::PS4FrameBuffer maskFBO;
#endif
		maskFBO.AddTexture((i.object->GetPaintMask()));
		if (Vector2(i.object->GetPaintMask()->GetWidth(), i.object->GetPaintMask()->GetHeight()) != currentSize) {
			currentSize = Vector2(i.object->GetPaintMask()->GetWidth(), i.object->GetPaintMask()->GetHeight());
			rendererAPI->SetViewportSize(i.object->GetPaintMask()->GetWidth(), i.object->GetPaintMask()->GetHeight());
		}

		const Vector2 pos = i.texUV_a * i.barycentric.x 
			+ i.texUV_b * i.barycentric.y 
			+ i.texUV_c * i.barycentric.z;
		maskShader->UpdateUniformVector2("uvHitPoint", pos);

		maskShader->UpdateUniformVector2("viewport", Vector2(i.object->GetPaintMask()->GetWidth(), i.object->GetPaintMask()->GetHeight()));

		maskShader->UpdateUniformFloat("radius", i.radius);
		maskShader->UpdateUniformFloat("hardness", i.hardness);
		maskShader->UpdateUniformFloat("strength", i.strength);
		maskShader->UpdateUniformVector4("colour", i.colour);

		rendererAPI->BindFrameBuffer(&maskFBO);

		rendererAPI->SetCullFace(false);
		rendererAPI->DrawMesh(skyboxMesh);

#ifdef _ORBIS
		PS4::PS4Texture* ps4Tex = static_cast<PS4::PS4Texture*>(i.object->GetPaintMask());
		uint64_t textureSizeInBytes;
		Gnm::AlignmentType textureAlignment;
		GpuAddress::computeTotalTiledTextureSize(&textureSizeInBytes, &textureAlignment, &ps4Tex->GetAPITexture());
		
		((PS4::PS4RendererAPI*)NCL::Rendering::RendererBase::rendererAPI)->currentGFXContext->waitForGraphicsWrites(
			ps4Tex->GetAPITexture().getBaseAddress256ByteBlocks(),
			(textureSizeInBytes + 255) / 256,
			Gnm::kWaitTargetSlotCb1,
			Gnm::kCacheActionWriteBackAndInvalidateL1andL2,
			Gnm::kExtendedCacheActionFlushAndInvalidateCbCache,
			Gnm::kStallCommandBufferParserDisable
		);
#endif

	}
	rendererAPI->SetBlend(false, RendererAPI::BlendType::ONE, RendererAPI::BlendType::NONE);
	rendererAPI->SetDepth(true);
	rendererAPI->SetViewportSize(rendererAPI->GetCurrentWidth(), rendererAPI->GetCurrentHeight());
	rendererAPI->BindFrameBuffer();
	rendererAPI->SetCullFace(true);
	rendererAPI->ClearBuffer(true, true, true);
	
	paintInstances.clear();
}

void Renderer::DrawGUI() {

	uiBarShader->BindShader();

	rendererAPI->SetCullFace(false);
	rendererAPI->SetBlend(false);
	rendererAPI->SetDepth(false);

	uiBarShader->UpdateUniformMatrix4("viewProjMatrix", Matrix4::Translation(Vector3(0, 1, 0)) * Matrix4::Orthographic(-1, 1.0f, 1, -1, -1, 1));
	uiBarShader->UpdateUniformVector2("ratio", scores);
	uiBarShader->UpdateUniformVector4("team1Colour", GameManager::team1Colour);
	uiBarShader->UpdateUniformVector4("team2Colour", GameManager::team2Colour);
	uiBarShader->UpdateUniformVector2("screenSize", Vector2(rendererAPI->GetCurrentWidth(), rendererAPI->GetCurrentHeight()));
	rendererAPI->DrawMesh(uiBarMesh);

	rendererAPI->SetBlend(true, RendererAPI::BlendType::ALPHA, RendererAPI::BlendType::ONE_MINUS_ALPHA);

	crosshairTex->Bind(0);
	uiCrosshairShader->BindShader();
	uiCrosshairShader->UpdateUniformMatrix4("viewProjMatrix", Matrix4::Translation(Vector3(0, 0.1f, 0)) * Matrix4::Orthographic(-1, 1.0f, 1, -1, -1, 1));
	uiCrosshairShader->UpdateUniformVector4("colour", playerColour);
	rendererAPI->DrawMesh(uiCrosshairMesh);

	rendererAPI->SetBlend(false, RendererAPI::BlendType::ONE, RendererAPI::BlendType::NONE);
	rendererAPI->SetCullFace(true);
	rendererAPI->SetDepth(true);
}

Maths::Matrix4 Renderer::SetupDebugLineMatrix()	const {
	const float screenAspect = (float)rendererAPI->GetCurrentWidth() / (float)rendererAPI->GetCurrentHeight();
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

	return projMatrix * viewMatrix;
}

Maths::Matrix4 Renderer::SetupDebugStringMatrix()	const {
	return Matrix4::Orthographic(-1, 1.0f, 100, 0, 0, 100);
}
