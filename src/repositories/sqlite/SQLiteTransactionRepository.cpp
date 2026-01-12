#include "SQLiteTransactionRepository.hpp"

void SQLiteTransactionRepository::create(const Transaction& tx) {
  m_db->execute(
    "INSERT INTO transactions(id,user_id,category_id,type,amount_cents,currency,date,note)"
    " VALUES(?,?,?,?,?,?,?,?)",
    {tx.id, tx.userId, tx.categoryId, tx.type,
     tx.amountCents, tx.currency, tx.date, tx.note});
}

std::vector<Transaction> SQLiteTransactionRepository::listByUser(
    const std::string& userId) {
  return m_db->query<Transaction>(
    "SELECT * FROM transactions WHERE user_id=? ORDER BY date DESC",
    {userId});
}
