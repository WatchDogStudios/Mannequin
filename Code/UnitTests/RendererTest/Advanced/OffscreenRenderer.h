#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Communication/IpcProcessMessageProtocol.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/IO/DirectoryWatcher.h>
#include <Foundation/Logging/HTMLWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererFoundation/RendererReflection.h>
#include <RendererFoundation/Resources/Texture.h>

struct nsOffscreenTest_SharedTexture
{
  NS_DECLARE_POD_TYPE();
  nsUInt32 m_uiCurrentTextureIndex = 0;
  nsUInt64 m_uiCurrentSemaphoreValue = 0;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsOffscreenTest_SharedTexture)


class nsOffscreenTest_OpenMsg : public nsProcessMessage
{
  NS_ADD_DYNAMIC_REFLECTION(nsOffscreenTest_OpenMsg, nsProcessMessage);

public:
  nsGALTextureCreationDescription m_TextureDesc;
  nsHybridArray<nsGALPlatformSharedHandle, 2> m_TextureHandles;
};

class nsOffscreenTest_CloseMsg : public nsProcessMessage
{
  NS_ADD_DYNAMIC_REFLECTION(nsOffscreenTest_CloseMsg, nsProcessMessage);
};

class nsOffscreenTest_RenderMsg : public nsProcessMessage
{
  NS_ADD_DYNAMIC_REFLECTION(nsOffscreenTest_RenderMsg, nsProcessMessage);

public:
  nsOffscreenTest_SharedTexture m_Texture;
};

class nsOffscreenTest_RenderResponseMsg : public nsProcessMessage
{
  NS_ADD_DYNAMIC_REFLECTION(nsOffscreenTest_RenderResponseMsg, nsProcessMessage);

public:
  nsOffscreenTest_SharedTexture m_Texture;
};

class nsOffscreenRendererTest : public nsApplication
{
public:
  using SUPER = nsApplication;

  nsOffscreenRendererTest();
  ~nsOffscreenRendererTest();

  virtual void Run() override;
  void OnPresent(nsUInt32 uiCurrentTexture, nsUInt64 uiCurrentSemaphoreValue);

  virtual void AfterCoreSystemsStartup() override;
  virtual void BeforeHighLevelSystemsShutdown() override;
  virtual void BeforeCoreSystemsShutdown() override;

  void MessageFunc(const nsProcessMessage* pMsg);


private:
  nsLogWriter::HTML m_LogHTML;
  nsGALDevice* m_pDevice = nullptr;
  nsGALSwapChainHandle m_hSwapChain;
  nsShaderResourceHandle m_hScreenShader;

  nsInt64 m_iHostPID = 0;
  nsUniquePtr<nsIpcChannel> m_pChannel;
  nsUniquePtr<nsIpcProcessMessageProtocol> m_pProtocol;

  bool m_bExiting = false;
  nsHybridArray<nsOffscreenTest_RenderMsg, 2> m_RequestedFrames;
};
