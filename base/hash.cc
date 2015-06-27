#include "hash.h" 

#include <iomanip> 
#include <sstream> 
#include <openssl/md5.h> 

namespace base {

std::string HashMD5(const char * data, size_t size) {
  unsigned char* hash = ::MD5((unsigned char*)data, size, NULL);

  using namespace std;
  stringstream res;
  for(size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    res << hex << setw(2) << setfill('0') << (unsigned int)hash[i];
  }

  return res.str();
}

}

