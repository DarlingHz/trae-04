#include "tag.h"

void to_json(json& j, const TagWithCount& tag) {
  j = json{{"tag_id", tag.tag_id},
            {"name", tag.name},
            {"card_count", tag.card_count}};
}
