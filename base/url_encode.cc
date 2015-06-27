#include <string>
using namespace std;

namespace base {

static string CharToHex(char c) {
	string result;
	char first;
	char second;
	
	first = (c & 0xF0) / 16;
	first += first > 9 ? 'A' - 10 : '0';
	second = c & 0x0F;
	second += second > 9 ? 'A' - 10 : '0';
	
	result.append(1, first);
	result.append(1, second);
	
	return result;
}

string UrlEncode(const string &url) {
	string result;
	string::const_iterator iter;
	
	for(iter = url.begin(); iter != url.end(); ++iter) 
	{
		switch(*iter) 
		{
		case ' ':
			result.append(1, '+');
			break;
			// alnum
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
		case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
		case 'v': case 'w': case 'x': case 'y': case 'z':
		case '0': case '1': case '2': case '3': case '4': case '5': case '6':
		case '7': case '8': case '9':
			// mark
		case '-': case '_': case '.': case '!': case '~': case '*': case '\'': 
		case '(': case ')':
			result.append(1, *iter);
			break;
			// escape
		default:
			result.append(1, '%');
			result.append(CharToHex(*iter));
			break;
		}
	}	
	return result;
}


const char HEX2DEC[256] = {
/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
/* 0 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 1 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 2 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 3 */0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

/* 4 */-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 5 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 6 */-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 7 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

/* 8 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 9 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* A */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* B */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

/* C */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* D */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* E */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* F */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

std::string UrlDecode(const std::string & src) {
  // Note from RFC1630:  "Sequences which start with a percent sign
  // but are not followed by two hexadecimal characters (0-9, A-F) are reserved
  // for future extension"

  const unsigned char * pSrc = (const unsigned char *) src.c_str();
  const int SRC_LEN = src.length();
  const unsigned char * const SRC_END = pSrc + SRC_LEN;
  const unsigned char * const SRC_LAST_DEC = SRC_END - 2; // last decodable '%'

  char * const pStart = new char[SRC_LEN];
  char * pEnd = pStart;

  while (pSrc < SRC_LAST_DEC) {
    if (*pSrc == '%') {
      char dec1, dec2;
      if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)]) && -1 != (dec2
          = HEX2DEC[*(pSrc + 2)])) {
        *pEnd++ = (dec1 << 4) + dec2;
        pSrc += 3;
        continue;
      }
    }

    *pEnd++ = *pSrc++;
  }

  // the last 2- chars
  while(pSrc < SRC_END) {
    *pEnd++ = *pSrc++;
  }

  std::string sResult(pStart, pEnd);
  delete[] pStart;
  return sResult;
}
}

