#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  define NS_POSIX_FILE_USEOLDAPI
#  define NS_POSIX_FILE_USEWINDOWSAPI
#  define NS_POSIX_FILE_NOINTERNALGETFILESTATS
#  define NS_POSIX_FILE_NOGETUSERDATAFOLDER
#  define NS_POSIX_FILE_NOGETTEMPDATAFOLDER
#  define NS_POSIX_FILE_NOGETUSERDOCUMENTSFOLDER
#  define NS_POSIX_FILE_NOGETCURRENTWORKINGDIRECTORY
#  define NS_POSIX_FILE_NOGETAPPLICATIONPATH

// For UWP we're currently using a mix of WinRT functions and Posix.
#  include <Foundation/Platform/Posix/OSFile_Posix.inl>

#  include <Foundation/Platform/UWP/Utils/UWPUtils.h>
#  include <windows.storage.h>

nsString nsOSFile::GetUserDataFolder(nsStringView sSubFolder)
{
  if (s_sUserDataPath.IsEmpty())
  {
    ComPtr<ABI::Windows::Storage::IApplicationDataStatics> appDataStatics;
    if (SUCCEEDED(ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), &appDataStatics)))
    {
      ComPtr<ABI::Windows::Storage::IApplicationData> applicationData;
      if (SUCCEEDED(appDataStatics->get_Current(&applicationData)))
      {
        ComPtr<ABI::Windows::Storage::IStorageFolder> applicationDataLocal;
        if (SUCCEEDED(applicationData->get_LocalFolder(&applicationDataLocal)))
        {
          ComPtr<ABI::Windows::Storage::IStorageItem> localFolderItem;
          if (SUCCEEDED(applicationDataLocal.As(&localFolderItem)))
          {
            HSTRING path;
            localFolderItem->get_Path(&path);
            s_sUserDataPath = nsStringUtf8(path).GetData();
          }
        }
      }
    }
  }

  nsStringBuilder s = s_sUserDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

nsString nsOSFile::GetTempDataFolder(nsStringView sSubFolder /*= nullptr*/)
{
  nsStringBuilder s;

  if (s_sTempDataPath.IsEmpty())
  {
    ComPtr<ABI::Windows::Storage::IApplicationDataStatics> appDataStatics;
    if (SUCCEEDED(ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), &appDataStatics)))
    {
      ComPtr<ABI::Windows::Storage::IApplicationData> applicationData;
      if (SUCCEEDED(appDataStatics->get_Current(&applicationData)))
      {
        ComPtr<ABI::Windows::Storage::IStorageFolder> applicationTempData;
        if (SUCCEEDED(applicationData->get_TemporaryFolder(&applicationTempData)))
        {
          ComPtr<ABI::Windows::Storage::IStorageItem> tempFolderItem;
          if (SUCCEEDED(applicationTempData.As(&tempFolderItem)))
          {
            HSTRING path;
            tempFolderItem->get_Path(&path);
            s_sTempDataPath = nsStringUtf8(path).GetData();
          }
        }
      }
    }
  }

  s = s_sTempDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

nsString nsOSFile::GetUserDocumentsFolder(nsStringView sSubFolder /*= {}*/)
{
  if (s_sUserDocumentsPath.IsEmpty())
  {
    NS_ASSERT_NOT_IMPLEMENTED;
  }

  nsStringBuilder s = s_sUserDocumentsPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

#endif
