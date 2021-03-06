#pragma once
#include <vector>
#include <string>
#include "../GameTech/RendererBase.h"
#include "../../Common/Quaternion.h"


// TODO: fix debug
namespace NCL {
	class Debug
	{
	public:
		static void Print(const std::string& text, const Vector2&pos, const float& size = 20.0f, const Vector4& colour = Vector4(1, 1, 1, 1));
		static void DebugPrint(const std::string& text, const Vector2& pos, const float& size = 20.0f, const Vector4& colour = Vector4(1, 1, 1, 1));
		static void DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);
		static void DrawArrow(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);
		static void DrawCube(const Vector3& centre, const Vector3& halfSizes, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f, const Quaternion & rotation = Quaternion(0, 0, 0, 0));
		static void DrawSphere(const Vector3& centre, const float& radius, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);
		static void DrawCircle(const Vector3& centre, const float& radius, const Quaternion& rotation = Quaternion(0,0,0,0), const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);
		static void DrawCapsule(const Vector3& centre, const float& radius, const float& height, const Quaternion& rotation = Quaternion(0,0,0,0), const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);
		static void DrawAxisLines(const Matrix4 &modelMatrix, float scaleBoost = 1.0f, float time = 0.0f);
		static void DrawTriangle(const Vector3& pos_a, const Vector3& pos_b, const Vector3& pos_c, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);
		static void DrawPlane(const Vector3& originPoint, const Vector3& normal, const float size, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);

		static void SetRenderer(RendererBase* r) {
			renderer = r;
		}

		static void FlushRenderables(float dt);


		static const Vector4 RED;
		static const Vector4 GREEN;
		static const Vector4 BLUE;

		static const Vector4 BLACK;
		static const Vector4 WHITE;

		static const Vector4 YELLOW;
		static const Vector4 MAGENTA;
		static const Vector4 CYAN;

	protected:
		struct DebugStringEntry {
			std::string	data;
			Vector2 position;
			Vector4 colour;
			float size;
		};

		struct DebugLineEntry {
			Vector3 start;
			Vector3 end;
			float	time;
			Vector4 colour;
		};

		Debug() {}
		~Debug() {}

		static std::vector<DebugStringEntry>	stringEntries;
		static std::vector<DebugLineEntry>	lineEntries;

		static RendererBase* renderer;
	};
}

