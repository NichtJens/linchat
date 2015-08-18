#include <string>
#include "fakewchars.h"

std::string fake_wchar(int wc_closing_key)
{
	switch (wc_closing_key)
	{
		case 159: return "ss";
		case 164: return "ae";
		case 182: return "oe";
		case 188: return "ue";
		case 132: return "Ae";
		case 150: return "Oe";
		case 156: return "Ue";
		default:  return "";
	}
}

