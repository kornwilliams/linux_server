#include "base/logging.h"
#include "base/config_reader.h"

#include "dump_request.h"
#include "add_items_request.h"
#include "get_items_request.h"
#include "update_items_request.h"
#include "del_items_request.h"

#include "fcgi_service/fcgi_service.h"

using namespace fcgi;
using std::string;

namespace lht {
namespace fcgi {

class LhtShoppingCartRequestFactory : public RequestFactory {
public:
  LhtShoppingCartRequestFactory() {}

  virtual RequestPtr Create(FCGX_Request * r) {
    char * path = FCGX_GetParam("SCRIPT_NAME", r->envp);
    RequestPtr req;

    if (path) {
      if (strcmp(path, "/shopping_cart/dump") == 0) {
        req = RequestPtr(new DumpRequest(r));
      } else if (strcmp(path, "/shopping_cart/add_items") == 0) {
        req = RequestPtr(new AddItemsRequest(r));
      } else if (strcmp(path, "/shopping_cart/get_items") == 0) {
        req = RequestPtr(new GetItemsRequest(r));
      } else if (strcmp(path, "/shopping_cart/update_items") == 0) {
        req = RequestPtr(new UpdateItemsRequest(r));
      } else if (strcmp(path, "/shopping_cart/del_items") == 0) {
        req = RequestPtr(new DelItemsRequest(r));
      }
    }
    return req;
  }

};

} // namespace fcgi
} // namespace lht


int main(int argc, char **argv) {
  char * conf_file = "../conf/lht_shopping_cart_proxy.conf";
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
  LOG_INFO("LhtShoppingCartProxy starts : port=" << bind_addr);

  FcgiServer * fcgi_server = new FcgiServer(bind_addr, 128);
  fcgi_server->RegisterRequestFactory(RequestFactoryPtr(new lht::fcgi::LhtShoppingCartRequestFactory()));
  fcgi_server->Start(true);
  return 0;
} // main
