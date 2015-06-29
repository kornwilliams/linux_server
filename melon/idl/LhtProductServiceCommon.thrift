namespace cpp lht

struct ProductBrief {
  1: required i64 pid;
  2: required string productName;
  3: required string productDesc;
  4: optional string productThumbUrl;
  5: required double productPrice;
}
typedef list<ProductBrief> ProductBriefList
typedef map<i64, ProductBrief> ProductBriefMap

