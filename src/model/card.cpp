#include "card.h"

void to_json(json& j, const Card& card) {
  j = json{{"card_id", card.card_id},
            {"title", card.title},
            {"content", card.content},
            {"tags", card.tags},
            {"is_pinned", card.is_pinned},
            {"created_at", card.created_at},
            {"updated_at", card.updated_at}};
}

void to_json(json& j, const CardList& card_list) {
  j = json{{"total", card_list.total},
            {"page", card_list.page},
            {"size", card_list.size},
            {"cards", card_list.cards}};
}
