#include "SQLiteCategoryRepository.hpp"

std::vector<Category> SQLiteCategoryRepository::listByUser(
    const std::string& userId, const std::string& type) {
  return m_db->query<Category>(
    "SELECT * FROM categories WHERE user_id=? AND type=?",
    {userId, type});
}

void SQLiteCategoryRepository::create(const Category& category) {
  m_db->execute(
    "INSERT INTO categories(id, user_id, name, type) VALUES(?,?,?,?)",
    {category.id, category.userId, category.name, category.type});
}
