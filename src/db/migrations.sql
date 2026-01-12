PRAGMA foreign_keys = ON;

--------------------------------------------------
-- USERS (future authentication support)
--------------------------------------------------
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

--------------------------------------------------
-- CATEGORIES (shared for income & expense)
--------------------------------------------------
CREATE TABLE IF NOT EXISTS categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    user_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

--------------------------------------------------
-- TRANSACTIONS (income + expense)
--------------------------------------------------
CREATE TABLE IF NOT EXISTS transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    category_id INTEGER,
    amount REAL NOT NULL CHECK (amount > 0),
    currency TEXT NOT NULL DEFAULT 'CAD',
    date TEXT NOT NULL,
    type TEXT NOT NULL CHECK (type IN ('INCOME', 'EXPENSE')),
    note TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

--------------------------------------------------
-- PROJECTS (issue-tracker system)
--------------------------------------------------
CREATE TABLE IF NOT EXISTS projects (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

--------------------------------------------------
-- ISSUES (issue-tracker system)
--------------------------------------------------
CREATE TABLE IF NOT EXISTS issues (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    project_id INTEGER,
    title TEXT NOT NULL,
    description TEXT,
    priority TEXT DEFAULT 'MEDIUM',
    status TEXT DEFAULT 'OPEN',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (project_id) REFERENCES projects(id)
);

--------------------------------------------------
-- INDEXES (performance polish)
--------------------------------------------------
CREATE INDEX IF NOT EXISTS idx_transactions_user
    ON transactions(user_id);

CREATE INDEX IF NOT EXISTS idx_transactions_date
    ON transactions(date);

CREATE INDEX IF NOT EXISTS idx_transactions_type
    ON transactions(type);
