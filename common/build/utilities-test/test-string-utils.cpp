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

#include "Utilities/StringUtils.h"

TEST(StringUtils, NarrowWithLiteral)
{
	// Since the files are saved as UTF-8, the literal must be converted from UTF-8 as well
	std::wstring input = StringUtils::UTF8::widen("дぬдぬ");
	std::string test = StringUtils::UTF8::narrow(input);
	EXPECT_EQ("дぬдぬ", test);
}

TEST(StringUtils, WidenWithLiteral)
{
	std::string input = "дぬдぬ";
	std::wstring test = StringUtils::UTF8::widen(input);
	// Since the files are saved as UTF-8, the literal must be converted from UTF-8 as well
	std::wstring expected = StringUtils::UTF8::widen("дぬдぬ");
	EXPECT_EQ(expected, test);
}

TEST(StringUtils, NarrowWithRawBytes)
{
	wchar_t utf16_bytes[3] = {0x0434, 0x306c, 0x0}; // дぬ
	char utf8_bytes[6] = {0xD0, 0xB4, 0xE3, 0x81, 0xAC, 0x0}; // дぬ
	std::wstring utf16_input = std::wstring(utf16_bytes);
	std::string expected_utf8 = std::string(utf8_bytes);

	std::string test = StringUtils::UTF8::narrow(utf16_input);
	EXPECT_EQ(expected_utf8, test);
}

TEST(StringUtils, WidenWithRawBytes)
{
	wchar_t utf16_bytes[3] = {0x0434, 0x306c, 0x0}; // дぬ
	char utf8_bytes[6] = {0xD0, 0xB4, 0xE3, 0x81, 0xAC, 0x0}; // дぬ
	std::string utf8_input = std::string(utf8_bytes);
	std::wstring expected_utf16 = std::wstring(utf16_bytes);
	
	std::wstring test = StringUtils::UTF8::widen(utf8_input);
	EXPECT_EQ(expected_utf16, test);
}

TEST(StringUtils, wxStringToUTF8)
{
	// Since the files are saved as UTF-8, the literal must be converted from UTF-8 as well
	wxString input = wxString(StringUtils::UTF8::widen("дぬдぬ"));
	std::string expected = "дぬдぬ";
	std::string test = StringUtils::UTF8::fromWxString(input);
	EXPECT_EQ(expected, test);
}