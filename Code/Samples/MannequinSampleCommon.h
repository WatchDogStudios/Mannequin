#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Shared helpers for small Mannequin sample applications.

#include "../Engine/RendererCore/Utils/nsVisualTestRunner.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <string>
#include <utility>

namespace MannequinSample
{
  namespace fs = std::filesystem;

  inline std::string ToLower(std::string value)
  {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
  }

  inline bool EqualsIgnoreCase(const std::string& lhs, const std::string& rhs)
  {
    return ToLower(lhs) == ToLower(rhs);
  }

  inline bool ContainsIgnoreCase(const std::string& value, const std::string& filter)
  {
    return ToLower(value).find(ToLower(filter)) != std::string::npos;
  }

  inline std::string GetArgValue(int argc, char** argv, std::initializer_list<const char*> names, const std::string& fallback)
  {
    for (int i = 1; i < argc; ++i)
    {
      std::string current = argv[i];
      for (const char* name : names)
      {
        if (current == name && i + 1 < argc)
          return argv[i + 1];

        std::string prefix = std::string(name) + "=";
        if (current.rfind(prefix, 0) == 0)
          return current.substr(prefix.size());
      }
    }

    return fallback;
  }

  inline nsGALGraphicsAPI ParseGraphicsAPI(const std::string& name)
  {
    if (EqualsIgnoreCase(name, "DX11"))
      return nsGALGraphicsAPI::DX11;
    if (EqualsIgnoreCase(name, "Vulkan"))
      return nsGALGraphicsAPI::Vulkan;
    return nsGALGraphicsAPI::DX12;
  }

  inline const char* GetGraphicsAPIName(nsGALGraphicsAPI api)
  {
    switch (api)
    {
      case nsGALGraphicsAPI::DX11:
        return "DX11";
      case nsGALGraphicsAPI::Vulkan:
        return "Vulkan";
      case nsGALGraphicsAPI::DX12:
        return "DX12";
      default:
        return "Unknown";
    }
  }

  inline void CreateParentDirectory(const std::string& path)
  {
    fs::path parent = fs::path(path).parent_path();
    if (!parent.empty())
      fs::create_directories(parent);
  }

  inline nsCapturedImage CreateImage(uint32_t width, uint32_t height, const std::function<void(uint32_t, uint32_t, uint8_t*)>& fillPixel)
  {
    nsCapturedImage image;
    image.m_uiWidth = width;
    image.m_uiHeight = height;
    image.m_uiRowPitch = width * 4;
    image.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
    image.m_Data.resize(image.m_uiRowPitch * height);

    for (uint32_t y = 0; y < height; ++y)
    {
      for (uint32_t x = 0; x < width; ++x)
      {
        uint8_t* pixel = image.m_Data.data() + y * image.m_uiRowPitch + x * 4;
        fillPixel(x, y, pixel);
      }
    }

    return image;
  }

  inline void AddApiCallCheck(std::vector<nsApiCallCheck>& checks,
                              const std::string& category,
                              const std::string& callName,
                              bool passed,
                              const std::string& message,
                              const std::string& recommendation,
                              const std::string& severity = "Error")
  {
    nsApiCallCheck check;
    check.m_sCategory = category;
    check.m_sCallName = callName;
    check.m_bPassed = passed;
    check.m_sSeverity = passed ? "Info" : severity;
    check.m_sMessage = message;
    check.m_sRecommendation = recommendation;
    checks.push_back(std::move(check));
  }

  inline void AddValidationMessage(std::vector<nsValidationMessage>& messages,
                                   const std::string& source,
                                   const std::string& severity,
                                   const std::string& messageText,
                                   const std::string& recommendation)
  {
    nsValidationMessage message;
    message.m_sSource = source;
    message.m_sSeverity = severity;
    message.m_sMessage = messageText;
    message.m_sRecommendation = recommendation;
    messages.push_back(std::move(message));
  }

  inline void AddStateCheck(std::vector<nsResourceStateCheck>& checks,
                            const std::string& name,
                            bool passed,
                            const std::string& actual,
                            const std::string& expected,
                            const std::string& messageText,
                            const std::string& recommendation,
                            const std::string& severity = "Error")
  {
    nsResourceStateCheck check;
    check.m_sName = name;
    check.m_bPassed = passed;
    check.m_sSeverity = passed ? "Info" : severity;
    check.m_sActual = actual;
    check.m_sExpected = expected;
    check.m_sMessage = messageText;
    check.m_sRecommendation = recommendation;
    checks.push_back(std::move(check));
  }

  inline nsResourceSnapshot CreateTextureSnapshot(const nsCapturedImage& image,
                                                  const std::string& name,
                                                  const std::string& slot,
                                                  const std::string& state,
                                                  const std::string& previewPath,
                                                  const std::string& summary)
  {
    nsResourceSnapshot snapshot;
    snapshot.m_sName = name;
    snapshot.m_sType = "Texture2D";
    snapshot.m_sSlot = slot;
    snapshot.m_sFormat = "R8G8B8A8_UNORM";
    snapshot.m_sState = state;
    snapshot.m_sSummary = summary;
    snapshot.m_sPreviewPath = previewPath;
    snapshot.m_uiWidth = image.m_uiWidth;
    snapshot.m_uiHeight = image.m_uiHeight;
    snapshot.m_uiMipLevels = 1;
    snapshot.m_uiRowPitch = image.m_uiRowPitch;
    snapshot.m_uiByteSize = static_cast<uint64_t>(image.m_Data.size());
    AddStateCheck(snapshot.m_StateChecks,
                  "Dimensions",
                  image.IsValid(),
                  std::to_string(image.m_uiWidth) + "x" + std::to_string(image.m_uiHeight),
                  "valid non-zero texture",
                  "Texture dimensions are valid for preview and comparison.",
                  "Check resource creation and capture metadata if this fails.");
    AddStateCheck(snapshot.m_StateChecks,
                  "Row pitch",
                  image.m_uiRowPitch == image.m_uiWidth * 4,
                  std::to_string(image.m_uiRowPitch),
                  std::to_string(image.m_uiWidth * 4),
                  "Row pitch matches tightly packed RGBA8 data.",
                  "Preserve backend row pitch when previewing copied resources.",
                  "Warning");
    return snapshot;
  }

  inline void SaveComparison(nsVisualTestResult& result,
                             const nsCapturedImage& testImage,
                             const nsCapturedImage& referenceImage,
                             const std::string& referenceDir,
                             const std::string& outputDir,
                             const std::string& apiName)
  {
    const std::string referencePath = referenceDir + "/" + apiName + "/" + result.m_sTestName + ".png";
    const std::string testOutputDir = outputDir + "/" + apiName + "/" + result.m_sTestName;

    CreateParentDirectory(referencePath);
    fs::create_directories(testOutputDir);

    result.m_sAPIName = apiName;
    result.m_bRenderSucceeded = testImage.IsValid();
    result.m_bReferenceExists = fs::exists(referencePath);
    if (!result.m_bReferenceExists)
      referenceImage.SavePNG(referencePath);

    nsImageComparator comparator;
    result.m_ComparisonResult = comparator.Compare(testImage, referenceImage);
    comparator.SaveComparisonReport(testImage, referenceImage, result.m_ComparisonResult, testOutputDir, result.m_sTestName);
  }

  inline void PushResult(nsVisualTestSummary& summary, nsVisualTestResult&& result)
  {
    summary.m_fTotalTimeMs += result.m_fRenderTimeMs;
    summary.m_uiTotalTests++;

    bool fatal = false;
    for (const auto& message : result.m_ValidationMessages)
      fatal = fatal || EqualsIgnoreCase(message.m_sSeverity, "Fatal") || EqualsIgnoreCase(message.m_sSeverity, "Error");

    for (const auto& snapshot : result.m_ResourceSnapshots)
    {
      for (const auto& check : snapshot.m_StateChecks)
        fatal = fatal || (!check.m_bPassed && (EqualsIgnoreCase(check.m_sSeverity, "Fatal") || EqualsIgnoreCase(check.m_sSeverity, "Error")));
    }

    if (fatal)
      result.m_ComparisonResult.m_bPassed = false;

    if (!result.m_bReferenceExists && result.m_bRenderSucceeded)
      summary.m_uiNewBaselines++;

    if (result.m_ComparisonResult.m_bPassed)
      summary.m_uiPassed++;
    else
      summary.m_uiFailed++;

    summary.m_Results.push_back(std::move(result));
  }
}
