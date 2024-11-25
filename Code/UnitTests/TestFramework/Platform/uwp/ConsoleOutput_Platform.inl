#include <Foundation/Logging/ETW.h>

void OutputToConsole_Platform(nsUInt8 uiColor, nsTestOutput::Enum type, nsInt32 iIndentation, const char* szMsg)
{
  printf("%*s%s\n", iIndentation, "", szMsg);

  char sz[4096];
  nsStringUtils::snprintf(sz, 4096, "%*s%s\n", iIndentation, "", szMsg);
  OutputDebugStringW(nsStringWChar(sz).GetData());

  nsLogMsgType::Enum logType = nsLogMsgType::None;
  switch (type)
  {
    case nsTestOutput::StartOutput:
    case nsTestOutput::InvalidType:
    case nsTestOutput::AllOutputTypes:
      logType = nsLogMsgType::None;
      break;
    case nsTestOutput::BeginBlock:
      logType = nsLogMsgType::BeginGroup;
      break;
    case nsTestOutput::EndBlock:
      logType = nsLogMsgType::EndGroup;
      break;
    case nsTestOutput::ImportantInfo:
    case nsTestOutput::Details:
    case nsTestOutput::Message:
    case nsTestOutput::Duration:
    case nsTestOutput::FinalResult:
      logType = nsLogMsgType::InfoMsg;
      break;
    case nsTestOutput::Success:
      logType = nsLogMsgType::SuccessMsg;
      break;
    case nsTestOutput::Warning:
      logType = nsLogMsgType::WarningMsg;
      break;
    case nsTestOutput::Error:
      logType = nsLogMsgType::ErrorMsg;
      break;
    case nsTestOutput::ImageDiffFile:
      logType = nsLogMsgType::DevMsg;
      break;
    default:
      break;
  }

  if (logType != nsLogMsgType::None)
  {
    nsETW::LogMessage(logType, iIndentation, szMsg);
  }
}
