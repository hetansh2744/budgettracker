import express from "express";
import cors from "cors";
import jwt from "jsonwebtoken";

const app = express();
const PORT = process.env.PORT || 3000;
const JWT_SECRET = process.env.JWT_SECRET || "flowfund_secret";

/* =====================================================
   🔥 CORS – THIS IS THE MOST IMPORTANT PART
   ===================================================== */

// ✅ Allow GitHub Pages + allow preflight
app.use((req, res, next) => {
  res.header("Access-Control-Allow-Origin", "https://hetansh2744.github.io");
  res.header("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
  res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");

  // ✅ Handle preflight BEFORE anything else
  if (req.method === "OPTIONS") {
    return res.sendStatus(204);
  }

  next();
});

// JSON parser AFTER CORS
app.use(express.json());

/* =====================================================
   In-memory demo storage
   ===================================================== */

const users = [];
const expenses = [];

/* =====================================================
   Helpers
   ===================================================== */

function signToken(user) {
  return jwt.sign(
    { id: user.id, email: user.email },
    JWT_SECRET,
    { expiresIn: "7d" }
  );
}

function auth(req, res, next) {
  const header = req.headers.authorization || "";
  if (!header.startsWith("Bearer ")) {
    return res.status(401).json({ message: "Missing token" });
  }

  try {
    const token = header.split(" ")[1];
    req.user = jwt.verify(token, JWT_SECRET);
    next();
  } catch {
    return res.status(401).json({ message: "Invalid token" });
  }
}

/* =====================================================
   Routes
   ===================================================== */

// ✅ Health check (already works)
app.get("/health", (req, res) => {
  res.json({ ok: true });
});

// ✅ Register (THIS WAS FAILING BEFORE)
app.post("/api/auth/register", (req, res) => {
  const { name, email, password } = req.body || {};

  if (!name || !email || !password) {
    return res.status(400).json({ message: "All fields required" });
  }

  if (users.find(u => u.email === email)) {
    return res.status(400).json({ message: "Email already registered" });
  }

  const user = {
    id: Date.now().toString(),
    name,
    email,
    password // demo only
  };

  users.push(user);

  const token = signToken(user);

  res.status(201).json({
    token,
    user: { id: user.id, name: user.name, email: user.email }
  });
});

// ✅ Login
app.post("/api/auth/login", (req, res) => {
  const { email, password } = req.body || {};

  const user = users.find(
    u => u.email === email && u.password === password
  );

  if (!user) {
    return res.status(401).json({ message: "Invalid credentials" });
  }

  const token = signToken(user);

  res.json({
    token,
    user: { id: user.id, name: user.name, email: user.email }
  });
});

// ✅ Get expenses
app.get("/api/expenses", auth, (req, res) => {
  res.json(expenses.filter(e => e.userId === req.user.id));
});

// ✅ Add expense
app.post("/api/expenses", auth, (req, res) => {
  const { title, amount, category, date } = req.body || {};

  if (!title || typeof amount !== "number") {
    return res.status(400).json({ message: "Invalid expense" });
  }

  const expense = {
    id: Date.now().toString(),
    userId: req.user.id,
    title,
    amount,
    category: category || "Other",
    date: date || new Date().toISOString().slice(0, 10)
  };

  expenses.push(expense);
  res.status(201).json(expense);
});

/* =====================================================
   Start server
   ===================================================== */

app.listen(PORT, () => {
  console.log(`🔥 FlowFund API running on port ${PORT}`);
});
