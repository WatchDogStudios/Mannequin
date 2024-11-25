#pragma once

#include <GameEngine/GameApplication/GameApplication.h>

class nsPlayerApplication : public nsGameApplication
{
public:
  using SUPER = nsGameApplication;

  nsPlayerApplication();

protected:
  virtual void Run_InputUpdate() override;
  virtual nsResult BeforeCoreSystemsStartup() override;
  virtual void AfterCoreSystemsStartup() override;

private:
  void DetermineProjectPath();
};
