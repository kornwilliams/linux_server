enum Gender {
    MALE = 0,
    FEMALE = 1
}

struct UserProfile {
  1: required i64 userId;
  2: required string phone;
  3: required string nickname;
  4: required string headurl;
  5: required i64 birthday;
  6: required Gender gender;
}
typedef list<UserProfile> UserProfileList
typedef map<i64, UserProfile> UserProfileMap
