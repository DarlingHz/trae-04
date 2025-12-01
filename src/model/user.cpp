#include "user.h"

void to_json(json& j, const UserWithToken& user) {
  j = json{{"user_id", user.user_id},
            {"email", user.email},
            {"token", user.token}};
}
