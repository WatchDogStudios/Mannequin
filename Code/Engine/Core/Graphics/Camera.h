#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Vec3.h>

/// \brief Projection mode for a camera.
enum class nsCameraMode
{
  None,
  PerspectiveFixedFovX,
  PerspectiveFixedFovY,
  OrthoFixedWidth,
  OrthoFixedHeight,
  Stereo,
};

/// \brief Represents a camera with position, orientation, and projection.
class NS_CORE_DLL nsCamera
{
public:
  nsCamera();

  void LookAt(const nsVec3& vCameraPos, const nsVec3& vTargetPos, const nsVec3& vUp);

  void SetCameraMode(nsCameraMode mode, float fFovOrDim, float fNearPlane, float fFarPlane);

  void SetViewMatrix(const nsMat4& mViewMatrix, nsCameraMode mode = nsCameraMode::None);

  nsCameraMode GetCameraMode() const;
  float GetFovOrDim() const;
  float GetNearPlane() const;
  float GetFarPlane() const;

  nsVec3 GetPosition(nsCameraEye eye = nsCameraEye::Left) const;
  nsVec3 GetDirForwards(nsCameraEye eye = nsCameraEye::Left) const;
  nsVec3 GetDirRight(nsCameraEye eye = nsCameraEye::Left) const;
  nsVec3 GetDirUp(nsCameraEye eye = nsCameraEye::Left) const;

  const nsMat4& GetViewMatrix(nsCameraEye eye = nsCameraEye::Left) const;
  void GetProjectionMatrix(float fAspectRatioWidthDivHeight, nsMat4& out_mProjectionMatrix, nsCameraEye eye = nsCameraEye::Left,
    nsClipSpaceDepthRange::Enum depthRange = nsClipSpaceDepthRange::Default) const;

  bool IsPerspective() const;
  bool IsOrthographic() const;
  bool IsStereoscopic() const;
};

/// \brief Camera eye selection.
enum class nsCameraEye
{
  Left,
  Right,
};
