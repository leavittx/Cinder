#pragma once

#include <string>
#include <iostream>
#include <filesystem>

#include <Windows.h>
#include <Psapi.h>
#pragma warning(push)
#pragma warning(disable : 4091)
#include <dbghelp.h>
#pragma warning(pop)

#include <boost/format.hpp>

namespace CrashReportUtilsCinder
{
  namespace fs = std::experimental::filesystem;

  // Source: http://blogs.msdn.com/b/joshpoley/archive/2008/05/19/prolific-usage-of-minidumpwritedump-automating-crash-dump-analysis-part-0.aspx
  inline HRESULT GenerateCrashDump(const std::string& dumpFilePath, MINIDUMP_TYPE flags, EXCEPTION_POINTERS *seh = nullptr)
  {
    HRESULT error = S_OK;

    // Open the file
    HANDLE hFile = CreateFileA(dumpFilePath.c_str(), GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                               nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
      error = GetLastError();
      error = HRESULT_FROM_WIN32(error);
      return error;
    }

    // Get the process information
    HANDLE hProc = GetCurrentProcess();
    DWORD procID = GetProcessId(hProc);

    // If we have SEH info, package it up
    MINIDUMP_EXCEPTION_INFORMATION sehInfo = { 0 };
    MINIDUMP_EXCEPTION_INFORMATION *sehPtr = nullptr;
    if (seh)
    {
      sehInfo.ThreadId = GetCurrentThreadId();
      sehInfo.ExceptionPointers = seh;
      sehInfo.ClientPointers = false;
      sehPtr = &sehInfo;
    }

    // Generate the crash dump
    BOOL result = MiniDumpWriteDump(hProc, procID, hFile, flags, sehPtr, nullptr, nullptr);
    if (!result)
    {
      error = static_cast<HRESULT>(GetLastError()); // already an HRESULT
    }

    // Ñlose the file
    CloseHandle(hFile);

    return error;
  }

  enum class MiniDumpFlags
  {
    Basic = MiniDumpNormal                      | 
            MiniDumpWithHandleData              | // includes info about all handles in the process handle table. 
            MiniDumpWithUnloadedModules,          // includes info from recently unloaded modules if supported by OS. 

    Complete = MiniDumpWithFullMemory           | // the contents of every readable page in the process address space is included in the dump.      
               MiniDumpWithHandleData           | // includes info about all handles in the process handle table. 
               MiniDumpWithThreadInfo           | // includes thread times, start address and affinity. 
               MiniDumpWithProcessThreadData    | // includes contents of process and thread environment blocks. 
               MiniDumpWithFullMemoryInfo       | // includes info on virtual memory layout. 
               MiniDumpWithUnloadedModules      | // includes info from recently unloaded modules if supported by OS. 
               MiniDumpWithFullAuxiliaryState   | // requests that aux data providers include their state in the dump. 
               MiniDumpIgnoreInaccessibleMemory | // ignore memory read failures. 
               MiniDumpWithTokenInformation       // includes security token related data.
  };

  inline fs::path GetExeFilePath()
  {
    char exeFilePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exeFilePath, MAX_PATH);
    //GetModuleBaseNameA(nullptr, exeFileName, MAX_PATH);
    return fs::path(exeFilePath);
  }

  inline bool MakeDumpDirWithData(const std::string& dumpDirName)
  {
    // Current working directory
    //auto workingDirPath(fs::current_path());
    //auto fullPathStem = workingDirPath.stem();
    //auto basePath = filesystem::basepath(fullPath);

    auto dumpDirPath = /*workingDirPath / */ fs::path(dumpDirName);

    std::error_code returnedError;
    fs::create_directory(dumpDirPath, returnedError);

    if (returnedError)
    {
      return false;
    }

    fs::path exeFilePath(GetExeFilePath());
    auto baseName = exeFilePath.stem();
    fs::path exeFileName(baseName.string() + ".exe");
    fs::path pdbFileName(baseName.string() + ".pdb");

    fs::copy(exeFileName, dumpDirPath / exeFileName, returnedError);
    //if (returnedError)
    //{
    //  return false;
    //}

    fs::copy(pdbFileName, dumpDirPath / pdbFileName, returnedError);
    //if (returnedError)
    //{
    //  return false;
    //}

    return true;
  }

  inline std::string DumpGetPrefix()
  {
    // Get the time
    SYSTEMTIME sysTime = { 0 };
    //GetSystemTime(&sysTime);
    GetLocalTime(&sysTime);

    // Get the computer name
    char compName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD compNameLen = ARRAYSIZE(compName);
    GetComputerNameA(compName, &compNameLen);

    // Get running exe file name
    auto exeFilePath = GetExeFilePath();
    auto exeFileName = exeFilePath.filename().string();

    // Build the filename: APP.EXE_COMPUTERNAME_DATE_TIME.DMP
    auto prefix = boost::str(boost::format("%s_%s_%04i-%02i-%02i_%02i-%02i-%02i") %
                             exeFileName % compName % sysTime.wYear % sysTime.wMonth % sysTime.wDay %
                             sysTime.wHour % sysTime.wMinute % sysTime.wSecond);

    return prefix;
  }

  inline void DumpExc(EXCEPTION_POINTERS* pExc)
  {
    namespace fs = fs;

    // Call MiniDumpWriteDump to produce the needed dump file
    // TODO: possibly add more flags: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms680519(v=vs.85).aspx
    // http://www.debuginfo.com/articles/effminidumps.html#minidumptypes
    // http://stackoverflow.com/a/5041924

    // TODO: pause all active threads before writing commit

    auto dumpPrefix = DumpGetPrefix();
    auto dumpDir = fs::path(dumpPrefix);
    if (!MakeDumpDirWithData(dumpDir.string()))
    {
      return;
    }

    auto dumpBasicPath = dumpDir / fs::path(dumpPrefix + "_basic" + ".dmp");
    auto dumpCompletePath = dumpDir / fs::path(dumpPrefix + "_complete" + ".dmp");

    auto basicOk = GenerateCrashDump(dumpBasicPath.string(), static_cast<MINIDUMP_TYPE>(MiniDumpFlags::Basic), pExc);
    if (basicOk != S_OK)
    {
      // TODO
    }

    auto completeOk = GenerateCrashDump(dumpCompletePath.string(), static_cast<MINIDUMP_TYPE>(MiniDumpFlags::Complete), pExc);
    if (completeOk != S_OK)
    {
      // TODO
    }

    // Terminate the application, so the annoying "Program has crashed" dialog won't appear
    ExitProcess(0);
  }
}