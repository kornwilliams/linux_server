include "BaseService.thrift"
include "LhtUserProfileCommon.thrift"

namespace cpp lht

service LhtUserProfileService extends BaseService.BaseService {
  i64 Login(1: string phone, 2: string password);
  i32 SendRegisterVerifyCode(1: string phone);
  i64 Register(1: string phone, 2: string password, 3: i32 code);
  i32 SendChangePasswordVerifyCode(1: string phone);
  i32 ChangePassword(1: string phone, 2: string oldPassword, 3: string newPassword, i32 code);

  LhtUserProfileCommon.UserProfile GetUserProfile(1: i64 uid);
  i32 UpdateUserProfile(1: LhtUserProfileCommon.UserProfile profile);
}
