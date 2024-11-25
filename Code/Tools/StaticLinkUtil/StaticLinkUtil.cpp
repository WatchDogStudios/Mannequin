#include <Foundation/Application/Application.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/HTMLWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Strings/PathUtils.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Utilities/CommandLineUtils.h>

/* When statically linking libraries into an application the linker will only pull in all the functions and variables that are inside
translation units (CPP files) that somehow get referenced.

In ns a lot of stuff happens automatically (e.g. types register themselves etc.), which is accomplished through global variables
that execute code in their constructor during the applications startup phase. This only works when those global variables are actually
put into the application by the linker. If the linker does not do that, functionality will not work as intended.

Contrary to common conception, the linker is NOT ALLOWED to optimize away global variables. The only reason for not including a global
variable into the final binary, is when the entire translation unit where a variable is defined in, is never referenced and thus never
even looked at by the linker.

To fix this, this tool inserts macros into each and every file which reference each other. Afterwards every file in a library will
have reference every other file in that same library and thus once a library is used in any way in some program, the entire library
will be pulled in and will then work as intended.

These references are accomplished through empty functions that are called in one central location (where NS_STATICLINK_LIBRARY is defined),
though the code actually never really calls those functions, but it is enough to force the linker to look at all the other files.

Usage of this tool:

Call this tool with the path to the root folder of some library as the sole command line argument:

StaticLinkUtil.exe "C:\nsEngine\Trunk\Code\Engine\Foundation"

This will iterate over all files below that folder and insert the proper macros.
Also make sure that exactly one file in each library contains the text 'NS_STATICLINK_LIBRARY();'

The parameters and function body will be automatically generated and later updated, you do not need to provide more.

See the Return Codes at the end of the BeforeCoreSystemsShutdown function.
*/

class nsStaticLinkerApp : public nsApplication
{
private:
  nsString m_sSearchDir;
  bool m_bHadErrors;
  bool m_bHadSeriousWarnings;
  bool m_bHadWarnings;
  bool m_bModifiedFiles;
  bool m_bAnyFileChanged;

  struct FileContent
  {
    FileContent() { m_bFileHasChanged = false; }

    bool m_bFileHasChanged;
    nsString m_sFileContent;
  };

  nsSet<nsString> m_AllRefPoints;
  nsString m_sRefPointGroupFile;

  nsSet<nsString> m_GlobalIncludes;

  nsMap<nsString, FileContent> m_ModifiedFiles;

  nsSet<nsString> m_FilesToModify;
  nsSet<nsString> m_FilesToLink;


public:
  using SUPER = nsApplication;

  nsStaticLinkerApp()
    : nsApplication("StaticLinkerApp")
  {
    m_bHadErrors = false;
    m_bHadSeriousWarnings = false;
    m_bHadWarnings = false;
    m_bModifiedFiles = false;
    m_bAnyFileChanged = false;
  }

  /// Makes sure the apps return value reflects whether there were any errors or warnings
  static void LogInspector(const nsLoggingEventData& eventData)
  {
    nsStaticLinkerApp* app = (nsStaticLinkerApp*)nsApplication::GetApplicationInstance();

    switch (eventData.m_EventType)
    {
      case nsLogMsgType::ErrorMsg:
        app->m_bHadErrors = true;
        break;
      case nsLogMsgType::SeriousWarningMsg:
        app->m_bHadSeriousWarnings = true;
        break;
      case nsLogMsgType::WarningMsg:
        app->m_bHadWarnings = true;
        break;

      default:
        break;
    }
  }

  virtual void AfterCoreSystemsStartup() override
  {
    nsGlobalLog::AddLogWriter(nsLogWriter::Console::LogMessageHandler);
    nsGlobalLog::AddLogWriter(nsLogWriter::VisualStudio::LogMessageHandler);
    nsGlobalLog::AddLogWriter(LogInspector);

    if (GetArgumentCount() != 2)
      nsLog::Error("This tool requires exactly one command-line argument: A path to the top-level folder of a library.");

    // pass the absolute path to the directory that should be scanned as the first parameter to this application
    nsStringBuilder sSearchDir = nsOSFile::MakePathAbsoluteWithCWD(GetArgument(1));

    m_sSearchDir = sSearchDir;

    // Add the empty data directory to access files via absolute paths
    nsFileSystem::AddDataDirectory("", "App", ":", nsDataDirUsage::AllowWrites).IgnoreResult();

    // use such a path to write to an absolute file
    // ':abs/C:/some/file.txt"
  }

  virtual void BeforeCoreSystemsShutdown() override
  {
    if ((m_bHadSeriousWarnings || m_bHadErrors) && m_bModifiedFiles)
      nsLog::SeriousWarning("There were issues while writing out the updated files. The source will be in an inconsistent state, please revert the changes.");
    else if (m_bHadWarnings || m_bHadSeriousWarnings || m_bHadErrors)
    {
      nsLog::Warning("There have been errors or warnings, see log for details.");
    }

    if (m_bModifiedFiles)
    {
      if (m_bHadErrors || m_bHadSeriousWarnings)
        SetReturnCode(3); // Errors or Serious Warnings, yet files modified, this is unusual, requires reverting the source from outside.
      else if (m_bHadWarnings)
        SetReturnCode(2); // Warnings, files still modified. (normal operation)
      else
        SetReturnCode(1); // No issues, files modified. (normal operation)
    }
    else
    {
      if (m_bHadErrors || m_bHadSeriousWarnings)
        SetReturnCode(-3); // Errors or serious warnings, no files modified (but might need to be), user needs to look at it.
      else if (m_bHadWarnings)
        SetReturnCode(-2); // Warnings, but no files were modified anyway, user should look at it though. (normal operation)
      else
        SetReturnCode(-1); // No issues, no file modifications, everything is up to date apparently. (normal operation)
    }

    // Return Codes:
    // All negative requires no RCS operations (no changes were made)
    // All positive require RCS operations (either commit or revert)
    // -1 and 1 are perfectly fine
    // -2 and 2 mean there were warnings that a user should look at, but nothing that prevented this tool from doing its work
    // -3 and 3 mean something went wrong
    // thus 3 means the changes it made need to be reverted from outside
    // 1 and 2 mean the changes need to be committed


    nsGlobalLog::RemoveLogWriter(LogInspector);
    nsGlobalLog::RemoveLogWriter(nsLogWriter::Console::LogMessageHandler);
    nsGlobalLog::RemoveLogWriter(nsLogWriter::VisualStudio::LogMessageHandler);
  }

  nsString GetLibraryMarkerName()
  {
    nsStringBuilder tmp;
    return nsPathUtils::GetFileName(m_sSearchDir.GetData()).GetData(tmp);
  }

  void SanitizeSourceCode(nsStringBuilder& ref_sInOut)
  {
    // this is now handled by clang-format and .editorconfig files

    // ref_sInOut.ReplaceAll("\r\n", "\n");
    // if (!ref_sInOut.EndsWith("\n"))
    //  ref_sInOut.Append("\n");

    while (ref_sInOut.EndsWith("\n\n\n\n"))
      ref_sInOut.Shrink(0, 1);
    while (ref_sInOut.EndsWith("\r\n\r\n\r\n\r\n"))
      ref_sInOut.Shrink(0, 2);
  }

  nsResult ReadEntireFile(nsStringView sFile, nsStringBuilder& ref_sOut)
  {
    ref_sOut.Clear();

    // if we have that file cached already, just return the cached (and possibly modified) content
    if (!m_ModifiedFiles[sFile].m_sFileContent.IsEmpty())
    {
      ref_sOut = m_ModifiedFiles[sFile].m_sFileContent.GetData();
      return NS_SUCCESS;
    }

    nsFileReader File;
    if (File.Open(sFile) == NS_FAILURE)
    {
      nsLog::Error("Could not open for reading: '{0}'", sFile);
      return NS_FAILURE;
    }

    nsDynamicArray<nsUInt8> FileContent;

    nsUInt8 Temp[1024];
    nsUInt64 uiRead = File.ReadBytes(Temp, 1024);

    while (uiRead > 0)
    {
      FileContent.PushBackRange(nsArrayPtr<nsUInt8>(Temp, (nsUInt32)uiRead));

      uiRead = File.ReadBytes(Temp, 1024);
    }

    FileContent.PushBack(0);

    if (!nsUnicodeUtils::IsValidUtf8((const char*)&FileContent[0]))
    {
      nsLog::Error("The file \"{0}\" contains characters that are not valid Utf8. This often happens when you type special characters in an editor "
                   "that does not save the file in Utf8 encoding.",
        sFile);
      return NS_FAILURE;
    }

    ref_sOut = (const char*)&FileContent[0];

    m_ModifiedFiles[sFile].m_sFileContent = ref_sOut;

    SanitizeSourceCode(ref_sOut);

    return NS_SUCCESS;
  }

  bool IsNearlyIdentical(nsStringView lhs, nsStringView rhs)
  {
    while (lhs.EndsWith("\n") || lhs.EndsWith("\r"))
      lhs.Shrink(0, 1);
    while (rhs.EndsWith("\n") || rhs.EndsWith("\r"))
      rhs.Shrink(0, 1);

    while (true)
    {
      while (lhs.StartsWith("\n") || lhs.StartsWith("\r"))
        lhs.ChopAwayFirstCharacterAscii();
      while (rhs.StartsWith("\n") || rhs.StartsWith("\r"))
        rhs.ChopAwayFirstCharacterAscii();

      if (lhs == rhs)
        return true;

      if (lhs.GetCharacter() != rhs.GetCharacter())
        return false;

      lhs.ChopAwayFirstCharacterUtf8();
      rhs.ChopAwayFirstCharacterUtf8();
    }
  }

  void OverwriteFile(nsStringView sFile, const nsStringBuilder& sFileContent)
  {
    nsStringBuilder sOut = sFileContent;
    SanitizeSourceCode(sOut);

    if (IsNearlyIdentical(m_ModifiedFiles[sFile].m_sFileContent, sOut))
      return;

    m_bAnyFileChanged = true;
    m_ModifiedFiles[sFile].m_bFileHasChanged = true;
    m_ModifiedFiles[sFile].m_sFileContent = sOut;
  }

  void OverwriteModifiedFiles()
  {
    NS_LOG_BLOCK("Overwriting modified files");

    if (m_bHadSeriousWarnings || m_bHadErrors)
    {
      nsLog::Info("There have been errors or warnings previously, no files will be modified.");
      return;
    }

    if (!m_bAnyFileChanged)
    {
      nsLog::Success("No files needed modification.");
      return;
    }

    for (auto it = m_ModifiedFiles.GetIterator(); it.IsValid(); ++it)
    {
      if (!it.Value().m_bFileHasChanged)
        continue;

      nsFileWriter FileOut;
      if (FileOut.Open(it.Key().GetData()) == NS_FAILURE)
      {
        nsLog::Error("Could not open the file for writing: '{0}'", it.Key());
        return;
      }
      else
      {
        m_bModifiedFiles = true;
        FileOut.WriteBytes(it.Value().m_sFileContent.GetData(), it.Value().m_sFileContent.GetElementCount()).IgnoreResult();

        nsLog::Success("File has been modified: '{0}'", it.Key());
      }
    }
  }

  void FindIncludes(nsStringBuilder& ref_sFileContent)
  {
    const char* szStartPos = ref_sFileContent.GetData();
    const nsString sLibraryName = GetLibraryMarkerName();

    while (true)
    {
      const char* szI = ref_sFileContent.FindSubString("#i", szStartPos);

      if (szI == nullptr)
        return;

      szStartPos = szI + 1;

      if (nsStringUtils::IsEqualN(szI, "#if", 3))
      {
        szStartPos = ref_sFileContent.FindSubString("#endif", szStartPos);

        if (szStartPos == nullptr)
          return;

        ++szStartPos;
        continue; // next search will be for #i again
      }

      if (nsStringUtils::IsEqualN(szI, "#include", 8))
      {
        szI += 8; // skip the "#include" string

        const char* szLineEnd = nsStringUtils::FindSubString(szI, "\n");

        nsStringView si(szI, szLineEnd ? szLineEnd : szI + nsStringUtils::GetStringElementCount(szI));

        nsStringBuilder sInclude = si;

        if (sInclude.ReplaceAll("\\", "/") > 0)
        {
          nsLog::Info("Replacing backslashes in #include path with front slashes: '{0}'", sInclude);
          ref_sFileContent.ReplaceSubString(szI, szLineEnd, sInclude.GetData());
        }

        while (sInclude.StartsWith(" ") || sInclude.StartsWith("\t") || sInclude.StartsWith("<"))
          sInclude.Shrink(1, 0);

        while (sInclude.EndsWith(" ") || sInclude.EndsWith("\t") || sInclude.EndsWith(">"))
          sInclude.Shrink(0, 1);

        // ignore relative includes, they will not work as expected from the PCH
        if (sInclude.StartsWith("\""))
          continue;

        // ignore includes into the own library
        if (sInclude.StartsWith(sLibraryName.GetData()))
          continue;

        // ignore third-party includes
        if (sInclude.FindSubString_NoCase("ThirdParty"))
        {
          nsLog::Dev("Skipping ThirdParty Include: '{0}'", sInclude);
          continue;
        }

        nsStringBuilder sCanFindInclude = m_sSearchDir.GetData();
        sCanFindInclude.PathParentDirectory();
        sCanFindInclude.AppendPath(sInclude.GetData());

        nsStringBuilder sCanFindInclude2 = m_sSearchDir.GetData();
        sCanFindInclude2.PathParentDirectory(2);
        sCanFindInclude2.AppendPath("Code/Engine");
        sCanFindInclude2.AppendPath(sInclude.GetData());

        // ignore includes to files that cannot be found (ie. they are not part of the nsEngine source tree)
        if (!nsFileSystem::ExistsFile(sCanFindInclude.GetData()) && !nsFileSystem::ExistsFile(sCanFindInclude2.GetData()))
        {
          nsLog::Dev("Skipping non-Engine Include: '{0}'", sInclude);
          continue;
        }

        // warn about includes that have 'implementation' in their path
        if (sInclude.FindSubString_NoCase("Implementation"))
        {
          nsLog::Warning("This file includes an implementation header from another library: '{0}'", sInclude);
        }

        nsLog::Dev("Found Include: '{0}'", sInclude);

        m_GlobalIncludes.Insert(sInclude);
      }
    }
  }

  bool RemoveLineWithPrefix(nsStringBuilder& ref_sFile, nsStringView sLineStart)
  {
    const char* szSkipAhead = ref_sFile.FindSubString("// <StaticLinkUtil::StartHere>");

    const char* szStart = ref_sFile.FindSubString(sLineStart, szSkipAhead);

    if (szStart == nullptr)
      return false;

    const char* szEnd = ref_sFile.FindSubString("\n", szStart);

    if (szEnd == nullptr)
      szEnd = ref_sFile.GetData() + ref_sFile.GetElementCount();

    ref_sFile.ReplaceSubString(szStart, szEnd, "");

    return true;
  }

  void RewritePrecompiledHeaderIncludes()
  {
    if (m_bHadSeriousWarnings || m_bHadErrors)
      return;

    nsStringBuilder sPCHFile = m_sSearchDir.GetData();
    sPCHFile.AppendPath("PCH.h");

    {
      nsFileReader File;
      if (File.Open(sPCHFile.GetData()) == NS_FAILURE)
      {
        nsLog::Warning("This project has no PCH file.");
        return;
      }
    }

    nsLog::Info("Rewriting PCH: '{0}'", sPCHFile);

    nsStringBuilder sFileContent;
    if (ReadEntireFile(sPCHFile.GetData(), sFileContent) == NS_FAILURE)
      return;

    while (RemoveLineWithPrefix(sFileContent, "#include"))
    {
      // do this
    }

    SanitizeSourceCode(sFileContent);

    nsStringBuilder sAllIncludes;

    for (auto it = m_GlobalIncludes.GetIterator(); it.IsValid(); ++it)
    {
      sAllIncludes.AppendFormat("#include <{0}>\n", it.Key());
    }

    sAllIncludes.ReplaceAll("\\", "/");

    sFileContent.Append(sAllIncludes.GetData());

    OverwriteFile(sPCHFile.GetData(), sFileContent);
  }

  void FixFileContents(nsStringView sFile)
  {
    nsStringBuilder sFileContent;
    if (ReadEntireFile(sFile, sFileContent) == NS_FAILURE)
      return;

    if (sFile.EndsWith("PCH.h"))
      nsLog::Dev("Skipping PCH for #include search: '{0}'", sFile);
    else
      FindIncludes(sFileContent);

    // rewrite the entire file
    OverwriteFile(sFile, sFileContent);
  }

  nsString GetFileMarkerName(nsStringView sFile)
  {
    nsStringBuilder sRel = sFile;
    sRel.MakeRelativeTo(m_sSearchDir.GetData()).IgnoreResult();

    nsStringBuilder sRefPointName = nsPathUtils::GetFileName(m_sSearchDir.GetData());
    sRefPointName.Append("_");
    sRefPointName.Append(sRel.GetData());
    sRefPointName.ReplaceAll("\\", "_");
    sRefPointName.ReplaceAll("/", "_");
    sRefPointName.ReplaceAll(".cpp", "");

    return sRefPointName;
  }

  void InsertRefPoint(nsStringView sFile)
  {
    NS_LOG_BLOCK("InsertRefPoint", sFile);

    nsStringBuilder sFileContent;
    if (ReadEntireFile(sFile, sFileContent) == NS_FAILURE)
      return;

    // if we find this macro in here, we don't need to insert NS_STATICLINK_FILE in this file
    // but once we are done with all files, we want to come back to this file and rewrite the NS_STATICLINK_LIBRARY
    // part such that it will reference all the other files
    if (sFileContent.FindSubString("NS_STATICLINK_LIBRARY"))
      return;

    nsString sLibraryMarker = GetLibraryMarkerName();
    nsString sFileMarker = GetFileMarkerName(sFile);

    nsStringBuilder sNewMarker;

    if (m_FilesToLink.Contains(sFile))
    {
      m_AllRefPoints.Insert(sFileMarker.GetData());
      sNewMarker.SetFormat("NS_STATICLINK_FILE({0}, {1});", sLibraryMarker, sFileMarker);
    }

    const char* szMarker = sFileContent.FindSubString("NS_STATICLINK_FILE");

    // if the marker already exists, replace it with the updated string
    if (szMarker != nullptr)
    {
      const char* szMarkerEnd = szMarker;

      while (*szMarkerEnd != '\0' && *szMarkerEnd != '\n')
        ++szMarkerEnd;

      sFileContent.ReplaceSubString(szMarker, szMarkerEnd, sNewMarker.GetData());
    }
    else
    {
      // otherwise insert it at the end of the file
      sFileContent.AppendFormat("\n\n{0}\n\n", sNewMarker);
    }

    // rewrite the entire file
    OverwriteFile(sFile, sFileContent);
  }

  void UpdateStaticLinkLibraryBlock()
  {
    if (m_bHadSeriousWarnings || m_bHadErrors)
      return;

    nsStringView sFile = m_sRefPointGroupFile;

    NS_LOG_BLOCK("RewriteRefPointGroup", sFile);

    nsLog::Info("Replacing macro NS_STATICLINK_LIBRARY in file '{0}'.", m_sRefPointGroupFile);

    nsStringBuilder sFileContent;
    if (ReadEntireFile(sFile, sFileContent) == NS_FAILURE)
      return;

    // remove all instances of NS_STATICLINK_FILE from this file, it already contains NS_STATICLINK_LIBRARY
    const char* szMarker = sFileContent.FindSubString("NS_STATICLINK_FILE");
    while (szMarker != nullptr)
    {
      nsLog::Warning("Found macro NS_STATICLINK_FILE inside the same file where NS_STATICLINK_LIBRARY is located. Removing it.");

      const char* szMarkerEnd = szMarker;

      while (*szMarkerEnd != '\0' && *szMarkerEnd != '\n')
        ++szMarkerEnd;

      // no ref point allowed in a file that has already a ref point group
      sFileContent.Remove(szMarker, szMarkerEnd);

      szMarker = sFileContent.FindSubString("NS_STATICLINK_FILE");
    }

    nsStringBuilder sNewGroupMarker;

    // generate the code that should be inserted into this file
    // this code will reference all the other files in the library
    {
      sNewGroupMarker.SetFormat("NS_STATICLINK_LIBRARY({0})\n{\n  if (bReturn)\n    return;\n\n", GetLibraryMarkerName());

      auto it = m_AllRefPoints.GetIterator();

      while (it.IsValid())
      {
        sNewGroupMarker.AppendFormat("  NS_STATICLINK_REFERENCE({0});\n", it.Key());
        ++it;
      }

      sNewGroupMarker.Append("}\n");
    }

    const char* szGroupMarker = sFileContent.FindSubString("NS_STATICLINK_LIBRARY");

    if (szGroupMarker != nullptr)
    {
      // if we could find the macro NS_STATICLINK_LIBRARY, just replace it with the new code

      const char* szMarkerEnd = szGroupMarker;

      bool bFoundOpenBraces = false;

      while (*szMarkerEnd != '\0' && *szMarkerEnd != '}')
      {
        ++szMarkerEnd;

        if (*szMarkerEnd == '{')
          bFoundOpenBraces = true;
        if (!bFoundOpenBraces && *szMarkerEnd == ';')
          break;
      }

      if (*szMarkerEnd == '}' || *szMarkerEnd == ';')
        ++szMarkerEnd;
      if (*szMarkerEnd == '\n')
        ++szMarkerEnd;

      // now replace the existing NS_STATICLINK_LIBRARY and its code block with the new block
      sFileContent.ReplaceSubString(szGroupMarker, szMarkerEnd, sNewGroupMarker.GetData());
    }
    else
    {
      // if we can't find the macro, append it to the end of the file
      // this can only happen, if we ever extend this tool such that it picks one file to auto-insert this macro
      sFileContent.AppendFormat("\n\n{0}\n\n", sNewGroupMarker);
    }

    OverwriteFile(sFile, sFileContent);
  }

  void IterateOverFiles()
  {
    if (m_bHadSeriousWarnings || m_bHadErrors)
      return;

    nsStringBuilder b, sExt;

    for (const nsString& sFile : m_FilesToModify)
    {
      if (sFile.HasExtension("h") || sFile.HasExtension("inl"))
      {
        NS_LOG_BLOCK("Header", sFile.GetFileNameAndExtension().GetStartPointer());
        FixFileContents(sFile);
        continue;
      }

      if (sFile.HasExtension("cpp"))
      {
        NS_LOG_BLOCK("Source", sFile.GetFileNameAndExtension().GetStartPointer());
        FixFileContents(sFile);

        InsertRefPoint(sFile);
        continue;
      }
    }
  }

  void MakeSureStaticLinkLibraryMacroExists()
  {
    if (m_bHadSeriousWarnings || m_bHadErrors)
      return;

    // The macro NS_STATICLINK_LIBRARY was not found in any cpp file
    // try to insert it into a PCH.cpp, if there is one
    if (!m_sRefPointGroupFile.IsEmpty())
      return;

    nsStringBuilder sFilePath;
    sFilePath.AppendPath(m_sSearchDir.GetData(), "PCH.cpp");

    auto it = m_ModifiedFiles.Find(sFilePath);

    if (it.IsValid())
    {
      nsStringBuilder sPCHcpp = it.Value().m_sFileContent.GetData();
      sPCHcpp.Append("\n\n\n\nNS_STATICLINK_LIBRARY() { }");

      OverwriteFile(sFilePath.GetData(), sPCHcpp);

      m_sRefPointGroupFile = sFilePath;

      nsLog::Warning("No NS_STATICLINK_LIBRARY found in any cpp file, inserting it into the PCH.cpp file.");
    }
    else
      nsLog::Error("The macro NS_STATICLINK_LIBRARY was not found in any cpp file in this library. It is required that it exists in exactly one "
                   "file, otherwise the generated code will not compile.");
  }

  void GatherInformation()
  {
    if (m_bHadSeriousWarnings || m_bHadErrors)
      return;

    NS_LOG_BLOCK("FindRefPointGroupFile");

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS) || defined(NS_DOCS)
    // get a directory iterator for the search directory
    nsFileSystemIterator it;
    it.StartSearch(m_sSearchDir.GetData(), nsFileSystemIteratorFlags::ReportFilesRecursive);

    if (it.IsValid())
    {
      nsStringBuilder sFile, sExt;

      // while there are additional files / folders
      for (; it.IsValid(); it.Next())
      {
        // build the absolute path to the current file
        sFile = it.GetCurrentPath();
        sFile.AppendPath(it.GetStats().m_sName.GetData());

        // file extensions are always converted to lower-case actually
        sExt = sFile.GetFileExtension();

        if (sExt.IsEqual_NoCase("h") || sExt.IsEqual_NoCase("inl"))
        {
          m_FilesToModify.Insert(sFile);
        }

        if (sExt.IsEqual_NoCase("cpp"))
        {
          nsStringBuilder sFileContent;
          if (ReadEntireFile(sFile.GetData(), sFileContent) == NS_FAILURE)
            return;

          // if we find this macro in here, we don't need to insert NS_STATICLINK_FILE in this file
          // but once we are done with all files, we want to come back to this file and rewrite the NS_STATICLINK_LIBRARY
          // part such that it will reference all the other files
          if (sFileContent.FindSubString("NS_STATICLINK_LIBRARY"))
          {
            m_FilesToModify.Insert(sFile);

            nsLog::Info("Found macro 'NS_STATICLINK_LIBRARY' in file '{0}'.", &sFile.GetData()[m_sSearchDir.GetElementCount() + 1]);

            if (!m_sRefPointGroupFile.IsEmpty())
              nsLog::Error("The macro 'NS_STATICLINK_LIBRARY' was already found in file '{0}' before. You cannot have this macro twice in the same library!", m_sRefPointGroupFile);
            else
              m_sRefPointGroupFile = sFile;
          }

          if (sFileContent.FindSubString("NS_STATICLINK_FILE_DISABLE"))
            continue;

          m_FilesToModify.Insert(sFile);

          bool bContainsGlobals = false;

          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("NS_STATICLINK_FORCE") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("NS_STATICLINK_LIBRARY") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("NS_BEGIN_") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("NS_PLUGIN_") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("NS_ON_GLOBAL_EVENT") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("nsCVarBool ") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("nsCVarFloat ") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("nsCVarInt ") != nullptr);
          bContainsGlobals = bContainsGlobals || (sFileContent.FindSubString("nsCVarString ") != nullptr);

          if (!bContainsGlobals)
            continue;

          m_FilesToLink.Insert(sFile);
        }
      }
    }
    else
      nsLog::Error("Could not search the directory '{0}'", m_sSearchDir);
#else
    NS_REPORT_FAILURE("No file system iterator support, StaticLinkUtil sample can't run.");
#endif
    MakeSureStaticLinkLibraryMacroExists();
  }

  virtual void Run() override
  {
    // something basic has gone wrong
    if (m_bHadSeriousWarnings || m_bHadErrors)
    {
      RequestApplicationQuit();
      return;
    }

    GatherInformation();

    IterateOverFiles();

    UpdateStaticLinkLibraryBlock();

    // RewritePrecompiledHeaderIncludes();

    OverwriteModifiedFiles();
    RequestApplicationQuit();
  }
};

NS_APPLICATION_ENTRY_POINT(nsStaticLinkerApp);
