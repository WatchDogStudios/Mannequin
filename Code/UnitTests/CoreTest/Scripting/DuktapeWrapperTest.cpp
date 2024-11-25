#include <CoreTest/CoreTestPCH.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

#  include <Core/Scripting/DuktapeContext.h>

#  include <Duktape/duk_module_duktape.h>
#  include <Duktape/duktape.h>
#  include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#  include <Foundation/IO/FileSystem/FileReader.h>
#  include <Foundation/IO/FileSystem/FileSystem.h>
#  include <TestFramework/Utilities/TestLogInterface.h>

static duk_ret_t ModuleSearchFunction(duk_context* pCtx);

static int CFuncPrint(duk_context* pContext)
{
  nsDuktapeFunction wrapper(pContext);
  const char* szText = wrapper.GetStringValue(0, nullptr);

  nsLog::Info("Print: '{}'", szText);
  return wrapper.ReturnVoid();
}

static int CFuncPrintVA(duk_context* pContext)
{
  nsDuktapeFunction wrapper(pContext);

  const nsUInt32 uiNumArgs = wrapper.GetNumVarArgFunctionParameters();

  nsStringBuilder s;
  s.AppendFormat("#Args: {}", uiNumArgs);

  for (nsUInt32 arg = 0; arg < uiNumArgs; ++arg)
  {
    if (wrapper.IsNumber(arg))
    {
      double val = wrapper.GetNumberValue(arg);
      s.AppendFormat(", #{}: Number = {}", arg, val);
    }
    else if (wrapper.IsBool(arg))
    {
      bool val = wrapper.GetBoolValue(arg);
      s.AppendFormat(", #{}: Bool = {}", arg, val);
    }
    else if (wrapper.IsString(arg))
    {
      const char* val = wrapper.GetStringValue(arg);
      s.AppendFormat(", #{}: String = {}", arg, val);
    }
    else if (wrapper.IsNull(arg))
    {
      s.AppendFormat(", #{}: null", arg);
    }
    else if (wrapper.IsUndefined(arg))
    {
      s.AppendFormat(", #{}: undefined", arg);
    }
    else if (wrapper.IsObject(arg))
    {
      s.AppendFormat(", #{}: object", arg);
    }
    else if (duk_check_type_mask(pContext, arg, DUK_TYPE_MASK_BUFFER))
    {
      s.AppendFormat(", #{}: buffer", arg);
    }
    else if (duk_check_type_mask(pContext, arg, DUK_TYPE_MASK_POINTER))
    {
      s.AppendFormat(", #{}: pointer", arg);
    }
    else if (duk_check_type_mask(pContext, arg, DUK_TYPE_MASK_LIGHTFUNC))
    {
      s.AppendFormat(", #{}: lightfunc", arg);
    }
    else
    {
      s.AppendFormat(", #{}: UNKNOWN TYPE", arg);
    }
  }

  nsLog::Info(s);
  return wrapper.ReturnString(s);
}

static int CFuncMagic(duk_context* pContext)
{
  nsDuktapeFunction wrapper(pContext);
  nsInt16 iMagic = wrapper.GetFunctionMagicValue();

  nsLog::Info("Magic: '{}'", iMagic);
  return wrapper.ReturnInt(iMagic);
}


NS_CREATE_SIMPLE_TEST(Scripting, DuktapeWrapper)
{
  // setup file system
  {
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);

    nsStringBuilder sTestDataDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
    sTestDataDir.AppendPath("Scripting/Duktape");
    if (!NS_TEST_RESULT(nsFileSystem::AddDataDirectory(sTestDataDir, "DuktapeTest")))
      return;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basics")
  {
    nsDuktapeContext duk("DukTest");

    duk_eval_string(duk.GetContext(), "'testString'.toUpperCase()");
    nsStringBuilder sTestString = duk_get_string(duk.GetContext(), -1);
    duk_pop(duk.GetContext());
    NS_TEST_STRING(sTestString, "TESTSTRING");

    NS_TEST_RESULT(duk.ExecuteString("function MakeUpper(bla) { return bla.toUpperCase() }"));


    duk_eval_string(duk.GetContext(), "MakeUpper(\"myTest\")");
    sTestString = duk_get_string(duk.GetContext(), -1);
    duk_pop(duk.GetContext());
    NS_TEST_STRING(sTestString, "MYTEST");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExecuteString (error)")
  {
    nsDuktapeContext duk("DukTest");

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);

    log.ExpectMessage("SyntaxError: parse error (line 1)", nsLogMsgType::ErrorMsg);
    NS_TEST_BOOL(duk.ExecuteString(" == invalid code == ").Failed());

    log.ExpectMessage("ReferenceError: identifier 'Print' undefined", nsLogMsgType::ErrorMsg);
    NS_TEST_BOOL(duk.ExecuteString("Print(\"do stuff\")").Failed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExecuteFile")
  {
    nsDuktapeContext duk("DukTest");
    duk.EnableModuleSupport(nullptr);

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);

    log.ExpectMessage("Print: 'called f1'", nsLogMsgType::InfoMsg);

    duk.RegisterGlobalFunction("Print", CFuncPrint, 1);

    duk.ExecuteFile("ExecuteFile.js").IgnoreResult();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "C Function")
  {
    nsDuktapeContext duk("DukTest");

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);

    log.ExpectMessage("Hello Test", nsLogMsgType::InfoMsg);

    duk.RegisterGlobalFunction("Print", CFuncPrint, 1);

    duk.ExecuteString("Print('Hello Test')").IgnoreResult();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "VarArgs C Function")
  {
    nsDuktapeContext duk("DukTest");

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);

    log.ExpectMessage("#Args: 5, #0: String = text, #1: Number = 7, #2: Bool = true, #3: null, #4: object", nsLogMsgType::InfoMsg);

    duk.RegisterGlobalFunctionWithVarArgs("PrintVA", CFuncPrintVA);

    duk.ExecuteString("PrintVA('text', 7, true, null, {})").IgnoreResult();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Call Function")
  {
    nsDuktapeContext duk("DukTest");

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);

    log.ExpectMessage("You did it", nsLogMsgType::InfoMsg);

    duk.RegisterGlobalFunction("Print", CFuncPrint, 1);

    if (NS_TEST_RESULT(duk.PrepareGlobalFunctionCall("Print"))) // [ Print ] / [ ]
    {
      duk.PushString("You did it, Fry!");                       // [ Print String ]
      NS_TEST_RESULT(duk.CallPreparedFunction());               // [ result ]
      duk.PopStack();                                           // [ ]
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Function Magic Value")
  {
    nsDuktapeContext duk("DukTest");

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);

    log.ExpectMessage("Magic: '1'", nsLogMsgType::InfoMsg);
    log.ExpectMessage("Magic: '2'", nsLogMsgType::InfoMsg);
    log.ExpectMessage("Magic: '3'", nsLogMsgType::InfoMsg);

    duk.RegisterGlobalFunction("Magic1", CFuncMagic, 0, 1);
    duk.RegisterGlobalFunction("Magic2", CFuncMagic, 0, 2);
    duk.RegisterGlobalFunction("Magic3", CFuncMagic, 0, 3);

    if (NS_TEST_RESULT(duk.PrepareGlobalFunctionCall("Magic1"))) // [ Magic1 ]
    {
      NS_TEST_RESULT(duk.CallPreparedFunction());                // [ result ]
      duk.PopStack();                                            // [ ]
    }

    if (NS_TEST_RESULT(duk.PrepareGlobalFunctionCall("Magic2"))) // [ Magic2 ]
    {
      NS_TEST_RESULT(duk.CallPreparedFunction());                // [ result ]
      duk.PopStack();                                            // [ ]
    }

    if (NS_TEST_RESULT(duk.PrepareGlobalFunctionCall("Magic3"))) // [ Magic2 ]
    {
      NS_TEST_RESULT(duk.CallPreparedFunction());                // [ result ]
      duk.PopStack();                                            // [ ]
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Inspect Object")
  {
    nsDuktapeContext duk("DukTest");
    nsDuktapeHelper val(duk);
    NS_TEST_RESULT(duk.ExecuteFile("Object.js"));

    duk.PushGlobalObject();                     // [ global ]
    NS_TEST_RESULT(duk.PushLocalObject("obj")); // [ global obj ]

    NS_TEST_BOOL(duk.HasProperty("i"));
    NS_TEST_INT(duk.GetIntProperty("i", 0), 23);

    NS_TEST_BOOL(duk.HasProperty("f"));
    NS_TEST_FLOAT(duk.GetFloatProperty("f", 0), 4.2f, 0.01f);

    NS_TEST_BOOL(duk.HasProperty("b"));
    NS_TEST_BOOL(duk.GetBoolProperty("b", false));

    NS_TEST_BOOL(duk.HasProperty("s"));
    NS_TEST_STRING(duk.GetStringProperty("s", ""), "text");

    NS_TEST_BOOL(duk.HasProperty("n"));

    NS_TEST_BOOL(duk.HasProperty("o"));

    {
      NS_TEST_RESULT(duk.PushLocalObject("o")); // [ global obj o ]
      NS_TEST_BOOL(duk.HasProperty("sub"));
      NS_TEST_STRING(duk.GetStringProperty("sub", ""), "wub");
    }

    duk.PopStack(3); // [ ]
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "require")
  {
    nsDuktapeContext duk("DukTest");
    duk.EnableModuleSupport(ModuleSearchFunction);

    duk.RegisterGlobalFunction("Print", CFuncPrint, 1);

    nsTestLogInterface log;
    nsTestLogSystemScope logSystemScope(&log);
    log.ExpectMessage("Print: 'called f1'", nsLogMsgType::InfoMsg);
    log.ExpectMessage("Print: 'Called require.js'", nsLogMsgType::InfoMsg);
    log.ExpectMessage("Print: 'require.js: called f1'", nsLogMsgType::InfoMsg);

    NS_TEST_RESULT(duk.ExecuteFile("require.js"));
  }

  nsFileSystem::RemoveDataDirectoryGroup("DuktapeTest");
}

static duk_ret_t ModuleSearchFunction(duk_context* pCtx)
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

#endif
