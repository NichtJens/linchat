#include <string>
#include <cstring>
#include "fakewchars.h"

const char mapping[] = 
//	"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"
	"AAAAAAACEEEEIIIIDNOOOOOx0UUUUYPsaaaaaaaceeeeiiiidnooooo:0uuuuypy";


std::string fake_wchar(int wc_closing_key)
{
	wc_closing_key -= 1<<7;
	std::string result = "_";
	if (wc_closing_key < strlen(mapping))
		result = std::string(1, mapping[wc_closing_key]);

	switch (wc_closing_key)
	{
		case 31: return "ss";  // ß
		case 36: return "ae";  // ä
		case 54: return "oe";  // ö
		case 60: return "ue";  // ü
		case  4: return "Ae";  // Ä
		case 22: return "Oe";  // Ö
		case 28: return "Ue";  // Ü
		case 38: return "ae";  // æ
		case  6: return "AE";  // Æ
		default: return result;
	}
}

