// Copyright 2016 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "src/main/cpp/util/file.h"
#include "src/main/cpp/util/file_platform.h"
#include "gtest/gtest.h"

#if !defined(COMPILER_MSVC) && !defined(__CYGWIN__)
#error("This test should only be run on Windows")
#endif  // !defined(COMPILER_MSVC) && !defined(__CYGWIN__)

namespace blaze_util {

using std::string;

void ResetMsysRootForTesting();  // defined in file_windows.cc

TEST(FileTest, TestDirname) {
  ASSERT_EQ("", Dirname(""));
  ASSERT_EQ("/", Dirname("/"));
  ASSERT_EQ("", Dirname("foo"));
  ASSERT_EQ("/", Dirname("/foo"));
  ASSERT_EQ("/foo", Dirname("/foo/"));
  ASSERT_EQ("foo", Dirname("foo/bar"));
  ASSERT_EQ("foo/bar", Dirname("foo/bar/baz"));
  ASSERT_EQ("\\", Dirname("\\foo"));
  ASSERT_EQ("\\foo", Dirname("\\foo\\"));
  ASSERT_EQ("foo", Dirname("foo\\bar"));
  ASSERT_EQ("foo\\bar", Dirname("foo\\bar\\baz"));
  ASSERT_EQ("foo\\bar/baz", Dirname("foo\\bar/baz\\qux"));
  ASSERT_EQ("c:/", Dirname("c:/"));
  ASSERT_EQ("c:\\", Dirname("c:\\"));
  ASSERT_EQ("c:/", Dirname("c:/foo"));
  ASSERT_EQ("c:\\", Dirname("c:\\foo"));
  ASSERT_EQ("\\\\?\\c:\\", Dirname("\\\\?\\c:\\"));
  ASSERT_EQ("\\\\?\\c:\\", Dirname("\\\\?\\c:\\foo"));
}

TEST(FileTest, TestBasename) {
  ASSERT_EQ("", Basename(""));
  ASSERT_EQ("", Basename("/"));
  ASSERT_EQ("foo", Basename("foo"));
  ASSERT_EQ("foo", Basename("/foo"));
  ASSERT_EQ("", Basename("/foo/"));
  ASSERT_EQ("bar", Basename("foo/bar"));
  ASSERT_EQ("baz", Basename("foo/bar/baz"));
  ASSERT_EQ("foo", Basename("\\foo"));
  ASSERT_EQ("", Basename("\\foo\\"));
  ASSERT_EQ("bar", Basename("foo\\bar"));
  ASSERT_EQ("baz", Basename("foo\\bar\\baz"));
  ASSERT_EQ("qux", Basename("foo\\bar/baz\\qux"));
  ASSERT_EQ("", Basename("c:/"));
  ASSERT_EQ("", Basename("c:\\"));
  ASSERT_EQ("foo", Basename("c:/foo"));
  ASSERT_EQ("foo", Basename("c:\\foo"));
  ASSERT_EQ("", Basename("\\\\?\\c:\\"));
  ASSERT_EQ("foo", Basename("\\\\?\\c:\\foo"));
}

TEST(FileTest, IsAbsolute) {
  ASSERT_FALSE(IsAbsolute(""));
  ASSERT_TRUE(IsAbsolute("/"));
  ASSERT_TRUE(IsAbsolute("/foo"));
  ASSERT_TRUE(IsAbsolute("\\"));
  ASSERT_TRUE(IsAbsolute("\\foo"));
  ASSERT_FALSE(IsAbsolute("c:"));
  ASSERT_TRUE(IsAbsolute("c:/"));
  ASSERT_TRUE(IsAbsolute("c:\\"));
  ASSERT_TRUE(IsAbsolute("c:\\foo"));
  ASSERT_TRUE(IsAbsolute("\\\\?\\c:\\"));
  ASSERT_TRUE(IsAbsolute("\\\\?\\c:\\foo"));
}

TEST(FileTest, IsRootDirectory) {
  ASSERT_FALSE(IsRootDirectory(""));
  ASSERT_TRUE(IsRootDirectory("/"));
  ASSERT_FALSE(IsRootDirectory("/foo"));
  ASSERT_TRUE(IsRootDirectory("\\"));
  ASSERT_FALSE(IsRootDirectory("\\foo"));
  ASSERT_FALSE(IsRootDirectory("c:"));
  ASSERT_TRUE(IsRootDirectory("c:/"));
  ASSERT_TRUE(IsRootDirectory("c:\\"));
  ASSERT_FALSE(IsRootDirectory("c:\\foo"));
  ASSERT_TRUE(IsRootDirectory("\\\\?\\c:\\"));
  ASSERT_FALSE(IsRootDirectory("\\\\?\\c:\\foo"));
}

TEST(FileTest, TestAsWindowsPath) {
  SetEnvironmentVariableA("BAZEL_SH", "c:\\msys\\some\\long\\path\\bash.exe");
  ResetMsysRootForTesting();
  std::wstring actual;

  ASSERT_TRUE(AsWindowsPath("", &actual));
  ASSERT_EQ(std::wstring(L""), actual);

  ASSERT_TRUE(AsWindowsPath("", &actual));
  ASSERT_EQ(std::wstring(L""), actual);

  ASSERT_TRUE(AsWindowsPath("foo/bar", &actual));
  ASSERT_EQ(std::wstring(L"foo\\bar"), actual);

  ASSERT_TRUE(AsWindowsPath("/c", &actual));
  ASSERT_EQ(std::wstring(L"c:\\"), actual);

  ASSERT_TRUE(AsWindowsPath("/c/", &actual));
  ASSERT_EQ(std::wstring(L"c:\\"), actual);

  ASSERT_TRUE(AsWindowsPath("/c/blah", &actual));
  ASSERT_EQ(std::wstring(L"c:\\blah"), actual);

  ASSERT_TRUE(AsWindowsPath("/d/progra~1/micros~1", &actual));
  ASSERT_EQ(std::wstring(L"d:\\progra~1\\micros~1"), actual);

  ASSERT_TRUE(AsWindowsPath("/foo", &actual));
  ASSERT_EQ(std::wstring(L"c:\\msys\\foo"), actual);

  std::wstring wlongpath(L"dummy_long_path\\");
  std::string longpath("dummy_long_path/");
  while (longpath.size() <= MAX_PATH) {
    wlongpath += wlongpath;
    longpath += longpath;
  }
  wlongpath = std::wstring(L"c:\\") + wlongpath;
  longpath = std::string("/c/") + longpath;
  ASSERT_TRUE(AsWindowsPath(longpath, &actual));
  ASSERT_EQ(wlongpath, actual);
}

TEST(FileTest, TestMsysRootRetrieval) {
  std::wstring actual;

  SetEnvironmentVariableA("BAZEL_SH", "c:/foo/msys/bar/qux.exe");
  ResetMsysRootForTesting();
  ASSERT_TRUE(AsWindowsPath("/blah", &actual));
  ASSERT_EQ(std::wstring(L"c:\\foo\\msys\\blah"), actual);

  SetEnvironmentVariableA("BAZEL_SH", "c:/foo/MSYS64/bar/qux.exe");
  ResetMsysRootForTesting();
  ASSERT_TRUE(AsWindowsPath("/blah", &actual));
  ASSERT_EQ(std::wstring(L"c:\\foo\\msys64\\blah"), actual);

  SetEnvironmentVariableA("BAZEL_SH", "c:/qux.exe");
  ResetMsysRootForTesting();
  ASSERT_FALSE(AsWindowsPath("/blah", &actual));

  SetEnvironmentVariableA("BAZEL_SH", nullptr);
  ResetMsysRootForTesting();
}

static void RunCommand(const string& cmdline) {
  STARTUPINFOA startupInfo = {sizeof(STARTUPINFO)};
  PROCESS_INFORMATION processInfo;
  // command line maximum size is 32K
  // Source (on 2017-01-04):
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
  char mutable_cmdline[0x8000];
  strncpy(mutable_cmdline, cmdline.c_str(), 0x8000);
  BOOL ok = CreateProcessA(
      /* lpApplicationName */ NULL,
      /* lpCommandLine */ mutable_cmdline,
      /* lpProcessAttributes */ NULL,
      /* lpThreadAttributes */ NULL,
      /* bInheritHandles */ TRUE,
      /* dwCreationFlags */ 0,
      /* lpEnvironment */ NULL,
      /* lpCurrentDirectory */ NULL,
      /* lpStartupInfo */ &startupInfo,
      /* lpProcessInformation */ &processInfo);
  ASSERT_TRUE(ok);

  // Wait 1 second for the process to finish.
  ASSERT_EQ(WAIT_OBJECT_0, WaitForSingleObject(processInfo.hProcess, 1000));

  DWORD exit_code = 1;
  ASSERT_TRUE(GetExitCodeProcess(processInfo.hProcess, &exit_code));
  ASSERT_EQ(0, exit_code);
}

TEST(FileTest, TestPathExistsWindows) {
  ASSERT_FALSE(PathExists(""));
  ASSERT_TRUE(PathExists("."));
  ASSERT_FALSE(PathExists("non.existent"));

  char buf[MAX_PATH] = {0};
  DWORD len = GetEnvironmentVariableA("TEST_TMPDIR", buf, MAX_PATH);
  ASSERT_GT(len, 0);
  string tmpdir(buf);
  ASSERT_TRUE(PathExists(tmpdir));

  // Create a fake msys root. We'll also use it as a junction target.
  string fake_msys_root(tmpdir + "/fake_msys");
  ASSERT_EQ(0, mkdir(fake_msys_root.c_str()));
  ASSERT_TRUE(PathExists(fake_msys_root));

  // Set the BAZEL_SH root so we can resolve MSYS paths.
  SetEnvironmentVariableA("BAZEL_SH",
                          (fake_msys_root + "/fake_bash.exe").c_str());
  ResetMsysRootForTesting();

  // Assert existence check for MSYS paths.
  ASSERT_FALSE(PathExists("/this/should/not/exist/mkay"));
  ASSERT_TRUE(PathExists("/"));

  // Create a junction pointing to an existing directory.
  RunCommand(string("cmd.exe /C mklink /J \"") + tmpdir + "/junc1\" \"" +
             fake_msys_root + "\" >NUL 2>NUL");
  ASSERT_TRUE(PathExists(fake_msys_root));
  ASSERT_TRUE(PathExists(JoinPath(tmpdir, "junc1")));

  // Create a junction pointing to a non-existent directory.
  RunCommand(string("cmd.exe /C mklink /J \"") + tmpdir + "/junc2\" \"" +
             fake_msys_root + "/i.dont.exist\" >NUL 2>NUL");
  ASSERT_FALSE(PathExists(JoinPath(fake_msys_root, "i.dont.exist")));
  ASSERT_FALSE(PathExists(JoinPath(tmpdir, "junc2")));

  // Clean up.
  ASSERT_EQ(0, rmdir(JoinPath(tmpdir, "junc1").c_str()));
  ASSERT_EQ(0, rmdir(JoinPath(tmpdir, "junc2").c_str()));
  ASSERT_EQ(0, rmdir(fake_msys_root.c_str()));
  ASSERT_FALSE(PathExists(JoinPath(tmpdir, "junc1")));
  ASSERT_FALSE(PathExists(JoinPath(tmpdir, "junc2")));
}

}  // namespace blaze_util
