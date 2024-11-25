#include <RendererTest/RendererTestPCH.h>

#include "Basics.h"
#include <Core/Graphics/Camera.h>

nsResult nsRendererTestBasics::InitializeSubTest(nsInt32 iIdentifier)
{
  m_iFrame = -1;

  if (nsGraphicsTest::InitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  if (iIdentifier == SubTests::ST_ClearScreen)
  {
    return CreateWindow(320, 240);
  }

  if (CreateWindow().Failed())
    return NS_FAILURE;

  m_hSphere = CreateSphere(3, 1.0f);
  m_hSphere2 = CreateSphere(1, 0.75f);
  m_hTorus = CreateTorus(16, 0.5f, 0.75f);
  m_hLongBox = CreateBox(0.4f, 0.2f, 2.0f);
  m_hLineBox = CreateLineBox(0.4f, 0.2f, 2.0f);



  return NS_SUCCESS;
}

nsResult nsRendererTestBasics::DeInitializeSubTest(nsInt32 iIdentifier)
{
  m_hSphere.Invalidate();
  m_hSphere2.Invalidate();
  m_hTorus.Invalidate();
  m_hLongBox.Invalidate();
  m_hLineBox.Invalidate();
  m_hTexture2D.Invalidate();
  m_hTextureCube.Invalidate();

  DestroyWindow();

  if (nsGraphicsTest::DeInitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  return NS_SUCCESS;
}


nsTestAppRun nsRendererTestBasics::SubtestClearScreen()
{
  BeginFrame();
  BeginCommands("ClearScreen");
  switch (m_iFrame)
  {
    case 0:
      BeginRendering(nsColor(1, 0, 0));
      break;
    case 1:
      BeginRendering(nsColor(0, 1, 0));
      break;
    case 2:
      BeginRendering(nsColor(0, 0, 1));
      break;
    case 3:
      BeginRendering(nsColor(0.5f, 0.5f, 0.5f, 0.5f));
      break;
  }

  EndRendering();
  NS_TEST_IMAGE(m_iFrame, 1);
  EndCommands();
  EndFrame();

  return m_iFrame < 3 ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}

void nsRendererTestBasics::RenderObjects(nsBitflags<nsShaderBindFlags> ShaderBindFlags)
{
  nsCamera cam;
  cam.SetCameraMode(nsCameraMode::PerspectiveFixedFovX, 90, 0.5f, 1000.0f);
  cam.LookAt(nsVec3(0, 0, 0), nsVec3(0, 0, -1), nsVec3(0, 1, 0));
  nsMat4 mProj;
  cam.GetProjectionMatrix((float)GetResolution().width / (float)GetResolution().height, mProj);
  nsMat4 mView = cam.GetViewMatrix();

  nsMat4 mTransform, mOther, mRot;

  mRot = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(-90));

  mOther = nsMat4::MakeScaling(nsVec3(1.0f, 1.0f, 1.0f));
  mTransform = nsMat4::MakeTranslation(nsVec3(-0.3f, -0.3f, 0.0f));
  RenderObject(m_hLongBox, mProj * mView * mTransform * mOther, nsColor(1, 0, 1, 0.25f), ShaderBindFlags);

  mOther = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(80.0f));
  mTransform = nsMat4::MakeTranslation(nsVec3(0.75f, 0, -1.8f));
  RenderObject(m_hTorus, mProj * mView * mTransform * mOther * mRot, nsColor(1, 0, 0, 0.5f), ShaderBindFlags);

  mOther.SetIdentity();
  mTransform = nsMat4::MakeTranslation(nsVec3(0, 0.1f, -2.0f));
  RenderObject(m_hSphere, mProj * mView * mTransform * mOther, nsColor(0, 1, 0, 0.75f), ShaderBindFlags);

  mOther = nsMat4::MakeScaling(nsVec3(1.5f, 1.0f, 1.0f));
  mTransform = nsMat4::MakeTranslation(nsVec3(-0.6f, -0.2f, -2.2f));
  RenderObject(m_hSphere2, mProj * mView * mTransform * mOther * mRot, nsColor(0, 0, 1, 1), ShaderBindFlags);
}

void nsRendererTestBasics::RenderLineObjects(nsBitflags<nsShaderBindFlags> ShaderBindFlags)
{
  nsCamera cam;
  cam.SetCameraMode(nsCameraMode::PerspectiveFixedFovX, 90, 0.5f, 1000.0f);
  cam.LookAt(nsVec3(0, 0, 0), nsVec3(0, 0, -1), nsVec3(0, 1, 0));
  nsMat4 mProj;
  cam.GetProjectionMatrix((float)GetResolution().width / (float)GetResolution().height, mProj);
  nsMat4 mView = cam.GetViewMatrix();

  nsMat4 mTransform, mOther, mRot;

  mRot = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(-90));

  mOther = nsMat4::MakeScaling(nsVec3(1.0f, 1.0f, 1.0f));
  mTransform = nsMat4::MakeTranslation(nsVec3(-0.3f, -0.3f, 0.0f));
  RenderObject(m_hLineBox, mProj * mView * mTransform * mOther, nsColor(1, 0, 1, 0.25f), ShaderBindFlags);
}

static nsRendererTestBasics g_Test;
