#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Strings/String.h>

/// \brief Provides debug drawing functionality (lines, boxes, spheres, text).
class NS_RENDERERCORE_DLL nsDebugRenderer
{
public:
  static void DrawLineSphere(const nsVec3& vCenter, float fRadius, const nsColor& color);
  static void DrawLineBox(const nsVec3& vMin, const nsVec3& vMax, const nsColor& color);
  static void DrawLine(const nsVec3& vStart, const nsVec3& vEnd, const nsColor& color);
  static void Draw3DText(const nsVec3& vPos, nsStringView sText, const nsColor& color = nsColor::White);
  static void Draw2DText(nsStringView sText, const nsVec2I32& vTopLeft, const nsColor& color = nsColor::White, nsUInt32 uiFontSize = 16);
};
