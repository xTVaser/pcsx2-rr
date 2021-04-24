/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021  PCSX2 Dev Team
 * 
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include "fmt/core.h"

#include "Utilities/FileUtils.h"

TEST(FileUtils, PrintWithFmt)
{
	fs::path test_path = "hello/world.file";
	std::string test = fmt::format("{}", test_path);
	EXPECT_EQ("hello/world.file", test);
}

TEST(FileUtils, AppendToFileWithExtension)
{
	fs::path test_path = "world.file";
	fs::path test = FileUtils::appendToFilename(test_path, "-append");
	fs::path expected = "world-append.file";
	EXPECT_EQ(expected, test);
}

TEST(FileUtils, AppendToFilePathWithExtension)
{
	fs::path test_path = "hello/world.file";
	fs::path test = FileUtils::appendToFilename(test_path, "-append");
	fs::path expected = "hello/world-append.file";
	EXPECT_EQ(expected, test);
}

TEST(FileUtils, AppendToFileWithoutExtension)
{
	fs::path test_path = "world";
	fs::path test = FileUtils::appendToFilename(test_path, "-append");
	fs::path expected = "world-append";
	EXPECT_EQ(expected, test);
}

TEST(FileUtils, AppendToFilePathWithoutExtension)
{
	fs::path test_path = "hello/world";
	fs::path test = FileUtils::appendToFilename(test_path, "-append");
	fs::path expected = "hello/world-append";
	EXPECT_EQ(expected, test);
}