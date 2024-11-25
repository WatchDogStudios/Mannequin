#include <CoreTest/CoreTestPCH.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

#  include <Core/Scripting/DuktapeContext.h>

#  include <Duktape/duktape.h>
#  include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#  include <Foundation/IO/FileSystem/FileReader.h>
#  include <Foundation/IO/FileSystem/FileSystem.h>
#  include <Foundation/IO/FileSystem/FileWriter.h>
#  include <TestFramework/Utilities/TestLogInterface.h>

static nsResult TranspileString(const char* szSource, nsDuktapeContext& ref_script, nsStringBuilder& ref_sResult)
{
  ref_script.PushGlobalObject();                                           // [ global ]
  ref_script.PushLocalObject("ts").IgnoreResult();                         // [ global ts ]
  NS_SUCCEED_OR_RETURN(ref_script.PrepareObjectFunctionCall("transpile")); // [ global ts transpile ]
  ref_script.PushString(szSource);                                         // [ global ts transpile source ]
  NS_SUCCEED_OR_RETURN(ref_script.CallPreparedFunction());                 // [ global ts result ]
  ref_sResult = ref_script.GetStringValue(-1);                             // [ global ts result ]
  ref_script.PopStack(3);                                                  // [ ]

  return NS_SUCCESS;
}

static nsResult TranspileFile(const char* szFile, nsDuktapeContext& ref_script, nsStringBuilder& ref_sResult)
{
  nsFileReader file;
  NS_SUCCEED_OR_RETURN(file.Open(szFile));

  nsStringBuilder source;
  source.ReadAll(file);

  return TranspileString(source, ref_script, ref_sResult);
}

static nsResult TranspileFileToJS(const char* szFile, nsDuktapeContext& ref_script, nsStringBuilder& ref_sResult)
{
  NS_SUCCEED_OR_RETURN(TranspileFile(szFile, ref_script, ref_sResult));

  nsStringBuilder sFile(":TypeScriptTest/", szFile);
  sFile.ChangeFileExtension("js");

  nsFileWriter file;
  NS_SUCCEED_OR_RETURN(file.Open(sFile));

  NS_SUCCEED_OR_RETURN(file.WriteBytes(ref_sResult.GetData(), ref_sResult.GetElementCount()));
  return NS_SUCCESS;
}

static int Duk_Print(duk_context* pContext)
{
  nsDuktapeFunction duk(pContext);

  nsLog::Info(duk.GetStringValue(0));

  return duk.ReturnVoid();
}

static duk_ret_t ModuleSearchFunction2(duk_context* pCtx)
{
  nsDuktapeFunction script(pCtx);

  /* Nargs was given as 4 and we get the following stack arguments:
   *   index 0: id
   *   index 1: require
   *   index 2: exports
   *   index 3: module
   */

  nsStringBuilder id = script.GetStringValue(0);
  id.ChangeFileExtension("js");

  nsStringBuilder source;
  nsFileReader file;
  file.Open(id).IgnoreResult();
  source.ReadAll(file);

  return script.ReturnString(source);


  /* Return 'undefined' to indicate no source code. */
  // return 0;
}

NS_CREATE_SIMPLE_TEST(Scripting, TypeScript)
{
  // setup file system
  {
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);

    nsStringBuilder sTestDataDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
    sTestDataDir.AppendPath("Scripting/TypeScript");
    if (!NS_TEST_RESULT(nsFileSystem::AddDataDirectory(sTestDataDir, "TypeScriptTest")))
      return;

    const nsStringBuilder sWriteDir = nsTestFramework::GetInstance()->GetAbsOutputPath();
    if (!NS_TEST_RESULT(nsFileSystem::AddDataDirectory(sWriteDir, "TypeScriptTest", "TypeScriptTest", nsDataDirUsage::AllowWrites)))
      return;

    if (!NS_TEST_RESULT(nsFileSystem::AddDataDirectory(">sdk/Data/Tools/nsEditor", "DuktapeTest")))
      return;
  }

  nsDuktapeContext duk("DukTS");
  duk.EnableModuleSupport(ModuleSearchFunction2);

  duk.RegisterGlobalFunction("Print", Duk_Print, 1);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compile TypeScriptServices")
  {
    NS_TEST_RESULT(duk.ExecuteFile("Typescript/typescriptServices.js"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transpile Simple")
  {
    // simple way
    NS_TEST_RESULT(duk.ExecuteString("ts.transpile('class X{}');"));

    // complicated way, needed to retrieve the result
    nsStringBuilder sTranspiled;
    TranspileString("class X{}", duk, sTranspiled).IgnoreResult();

    // validate that the transpiled code can be executed by Duktape
    nsDuktapeContext duk2("duk");
    NS_TEST_RESULT(duk2.ExecuteString(sTranspiled));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transpile File")
  {
    nsStringBuilder result;
    NS_TEST_RESULT(TranspileFileToJS("Foo.ts", duk, result));

    duk.ExecuteFile("Foo.js").IgnoreResult();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Import files")
  {
    nsStringBuilder result;
    NS_TEST_RESULT(TranspileFileToJS("Bar.ts", duk, result));

    duk.ExecuteFile("Bar.js").IgnoreResult();
  }

  nsFileSystem::DeleteFile(":TypeScriptTest/Foo.js");
  nsFileSystem::DeleteFile(":TypeScriptTest/Bar.js");

  nsFileSystem::RemoveDataDirectoryGroup("DuktapeTest");
}

#endif
