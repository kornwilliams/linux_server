#
# Official dependency number 0: CMake.
# Obviously, we must have it if we are here but test version.
#
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 FATAL_ERROR )

#
# Enter some information about the project
# This is the only configurable part of the build system
#
# project name
PROJECT(BigWorld)

SET(CMAKE_VERBOSE_MAKEFILE "1")

SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_FLAGS "-fPIC -g -Wall -DHAVE_NETINET_IN_H")
MESSAGE (STATUS "**************** Debug ****************")

INCLUDE( "../cmake/main.cmake" )

BW_ADD_EXECUTABLE(lht/push_service lht_push_service thrift_LhtPushService melon_service base curl boost_thread uuid)
BW_ADD_EXECUTABLE(lht/bingo_service lht_bingo_service thrift_LhtBingoService melon_service redis_executor base boost_thread)

BW_ADD_EXECUTABLE(lht/shopping_cart_proxy lht_shopping_cart_proxy thrift_LhtShoppingCartService thrift_BaseService fcgi_service melon_client base curl boost_thread uuid)
BW_ADD_EXECUTABLE(lht/shopping_cart_service lht_shopping_cart_service thrift_LhtShoppingCartService melon_service redis_executor base boost_thread)

BW_ADD_EXECUTABLE(lht/user_profile_service lht_user_profile_service
thrift_LhtUserProfileService thrift_LhtUserProfileCommon thrift_BaseService
melon_service redis_executor base database boost_thread)

BW_ADD_EXECUTABLE(lht/user_profile_proxy lht_user_profile_proxy thrift_LhtUserProfileService thrift_BaseService fcgi_service melon_client base curl boost_thread)
