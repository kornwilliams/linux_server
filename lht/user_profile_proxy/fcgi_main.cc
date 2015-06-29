#include "base/logging.h"
#include "base/config_reader.h"

#include "dump_request.h"
#include "send_register_verify_code_request.h"
#include "send_change_password_verify_code_request.h"
#include "login_request.h"
#include "register_request.h"
#include "change_password_request.h"

#include "fcgi_service/fcgi_service.h"


using namespace fcgi;
using std::string;

namespace lht {
namespace fcgi {

class LhtUserProfileRequestFactory : public RequestFactory {
public:
  LhtUserProfileRequestFactory() {}

  virtual RequestPtr Create(FCGX_Request * r) {
    char * path = FCGX_GetParam("SCRIPT_NAME", r->envp);
    RequestPtr req;

    if (path) {
      if (strcmp(path, "/user_profile/dump") == 0) {
        req = RequestPtr(new DumpRequest(r));
      } else if (strcmp(path, "/user_profile/send_change_password_verify_code") == 0) {
        req = RequestPtr(new SendChangePasswordVerifyCodeRequest(r));
      } else if (strcmp(path, "/user_profile/send_register_verify_code") == 0) {
        req = RequestPtr(new SendRegisterVerifyCodeRequest(r));
      } else if (strcmp(path, "/user_profile/change_password") == 0) {
        req = RequestPtr(new ChangePasswordRequest(r));
      } else if (strcmp(path, "/user_profile/login") == 0) {
        req = RequestPtr(new LoginRequest(r));
      } else if (strcmp(path, "/user_profile/register") == 0) {
        req = RequestPtr(new RegisterRequest(r));
      }
    }
    return req;
  }

};

} // namespace fcgi
} // namespace lht


int main(int argc, char **argv) {
  char * conf_file = "../conf/lht_user_profile_proxy.conf";
  char opt;
  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
    case 'c':
      conf_file = optarg;
      break;
    default:
      std::cerr << "Unknown option " << optopt << std::endl;
      return 1;
    }
  }

  ConfigReader cfg(conf_file);
  if (!cfg) {
    std::cerr << "Config file " << conf_file << " read error!" << std::endl;
    return 1;
  }
  LOG_INIT(cfg.Get("log4cplus", "file"), cfg.Get("log4cplus", "level"));

  std::string bind_addr = cfg.Get("fcgi_service", "bind");
  LOG_INFO("LhtUserProfileProxy starts : port=" << bind_addr);

  FcgiServer * fcgi_server = new FcgiServer(bind_addr, 128);
  fcgi_server->RegisterRequestFactory(RequestFactoryPtr(new lht::fcgi::LhtUserProfileRequestFactory()));
  fcgi_server->Start(true);
  return 0;
} // main
