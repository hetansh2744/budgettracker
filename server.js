import express from "express";
import cors from "cors";
import jwt from "jsonwebtoken";

const app = express();
const PORT = process.env.PORT || 3000;
const JWT_SECRET = process.env.JWT_SECRET || "flowfund_secret_key_demo";

/* ===========================
   Middleware
   =========================== */

app.use(express.json());

// ✅ CORS for GitHub Pages + local dev
app.use(
  cors({
    origin: [
      "https://hetansh2744.github.io",
      "http://localhost:5500",
      "http://127.0.0.1:5500"
    ],
    methods: ["GET", "POST", "PUT", "DELETE", "OPTIONS"],
    allowedHeaders: ["Content-Type", "Authorization"]
  })
);

// ✅ Preflight support (THIS fixes "Failed to fetch")
app.options("*", cors());

/* ===========================
   Demo storage (in-memory)
   Replace with DB later
   =========================== */

const users = [];     // { id, name, email, password }
const expenses = [];  // { id, userId, title, amount, category, date }

/* ===========================
   Helpers
   =========================== */

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
    return res.status(401).json({ message: "Missing Bearer token" });
  }

  try {
    const token = header.split(" ")[1];
    const decoded = jwt.verify(token, JWT_SECRET);
    req.user = decoded;
    next();
  } catch (e) {
    return res.status(401).json({ message: "Invalid or expired token" });
  }
}

/* ===========================
   Routes
   =========================== */

app.get("/health", (req, res) => {
  res.json({ ok: true });
});

/* ----- AUTH ----- */

app.post("/api/auth/register", (req, res) => {
  const { name, email, password } = req.body || {};

  if (!name || !email || !password) {
    return res.status(400).json({ message: "name, email, password required" });
  }

  const exists = users.find((u) => u.email.toLowerCase() === email.toLowerCase());
  if (exists) {
    return res.status(400).json({ message: "Email already registered" });
  }

  const user = {
    id: Date.now().toString(),
    name,
    email,
    password // ⚠️ demo only — use bcrypt in real app
  };
  users.push(user);

  const token = signToken(user);

  return res.status(201).json({
    token,
    user: { id: user.id, name: user.name, email: user.email }
  });
});

app.post("/api/auth/login", (req, res) => {
  const { email, password } = req.body || {};

  if (!email || !password) {
    return res.status(400).json({ message: "email and password required" });
  }

  const user = users.find(
    (u) =>
      u.email.toLowerCase() === email.toLowerCase() &&
      u.password === password
  );

  if (!user) {
    return res.status(401).json({ message: "Invalid email or password" });
  }

  const token = signToken(user);

  return res.json({
    token,
    user: { id: user.id, name: user.name, email: user.email }
  });
});

/* ----- EXPENSES (protected) ----- */

app.get("/api/expenses", auth, (req, res) => {
  const list = expenses.filter((e) => e.userId === req.user.id);
  res.json(list);
});

app.post("/api/expenses", auth, (req, res) => {
  const { title, amount, category, date } = req.body || {};

  if (!title || typeof amount !== "number") {
    return res.status(400).json({ message: "title and numeric amount required" });
  }

  const exp = {
    id: Date.now().toString(),
    userId: req.user.id,
    title,
    amount,
    category: category || "Other",
    date: date || new Date().toISOString().slice(0, 10)
  };

  expenses.push(exp);
  res.status(201).json(exp);
});

/* ===========================
   Start
   =========================== */

app.listen(PORT, () => {
  console.log(`🔥 FlowFund API running on port ${PORT}`);
});
