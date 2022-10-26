
#pragma once

enum class MultitapMode
{
	NOT_SET = 0xff,
	PAD_SUPPORT_CHECK = 0x12,
	MEMCARD_SUPPORT_CHECK = 0x13,
	SELECT_PAD = 0x21,
	SELECT_MEMCARD = 0x22,
};

class MultitapProtocol
{
private:
	void SupportCheck();
	void Select();

public:
	MultitapProtocol();
	~MultitapProtocol();

	void SoftReset();
	void FullReset();

	void SendToMultitap();
};

extern MultitapProtocol g_MultitapProtocol;
