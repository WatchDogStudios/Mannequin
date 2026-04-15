#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Math/Vec3.h>

/// \brief Defines a coordinate system with forward, right, and up directions.
struct NS_CORE_DLL nsCoordinateSystem
{
  nsVec3 m_vForwardDir = nsVec3(1, 0, 0);
  nsVec3 m_vRightDir = nsVec3(0, 1, 0);
  nsVec3 m_vUpDir = nsVec3(0, 0, 1);
};

/// \brief Provider interface for coordinate systems.
class NS_CORE_DLL nsCoordinateSystemProvider
{
public:
  virtual ~nsCoordinateSystemProvider() = default;
  virtual void GetCoordinateSystem(const nsVec3& vGlobalPosition, nsCoordinateSystem& out_coordinateSystem) const = 0;
};

/// \brief Converts positions and rotations between two coordinate systems.
class NS_CORE_DLL nsCoordinateSystemConversion
{
public:
  void SetConversion(const nsCoordinateSystem& source, const nsCoordinateSystem& target);

  float ConvertSourceLength(float fLength) const;
  float ConvertTargetLength(float fLength) const;

  nsVec3 ConvertSourcePosition(const nsVec3& vPos) const;
  nsVec3 ConvertTargetPosition(const nsVec3& vPos) const;

  nsQuat ConvertSourceRotation(const nsQuat& qRot) const;
  nsQuat ConvertTargetRotation(const nsQuat& qRot) const;

private:
  nsMat3 m_mSourceToTarget;
  nsMat3 m_mTargetToSource;
};
