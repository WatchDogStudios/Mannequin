#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Math/Size.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Status.h>

/// \brief Window mode options.
enum class nsWindowMode
{
  WindowFixedResolution,
  WindowResizable,
  FullscreenBorderlessNativeResolution,
  FullscreenFixedResolution,
};

/// \brief Description for creating a window.
struct NS_CORE_DLL nsWindowCreationDesc
{
  nsString m_Title = "nsEngine";
  nsSizeU32 m_Resolution = nsSizeU32(1280, 720);
  nsWindowMode m_WindowMode = nsWindowMode::WindowFixedResolution;
  bool m_bClipMouseCursor = true;
  bool m_bShowMouseCursor = false;
};

/// \brief Platform-independent window base class.
class NS_CORE_DLL nsWindow
{
public:
  nsWindow();
  virtual ~nsWindow();

  nsResult Initialize(const nsWindowCreationDesc& desc);
  nsResult Destroy();

  nsResult Resize(const nsSizeU32& newSize);

  nsSizeU32 GetClientAreaSize() const;
  const nsWindowCreationDesc& GetCreationDescription() const;

  void ProcessWindowMessages();

  bool IsVisible() const;
  bool IsMinimized() const;
  bool IsFocused() const;

  virtual void OnResize(const nsSizeU32& newSize) {}
  virtual void OnFocus(bool bHasFocus) {}
  virtual void OnClickClose() {}

private:
  nsWindowCreationDesc m_CreationDescription;
  nsSizeU32 m_ClientAreaSize;
};
