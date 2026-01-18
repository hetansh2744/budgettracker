/* ===========================
   FlowFund GitHub Pages UI
   =========================== */

/**
 * IMPORTANT:
 * Set this to your Render base URL.
 * Do NOT add a trailing slash.
 */
const API_BASE = "https://budgettracker-2-qmpz.onrender.com";

/**
 * Update these paths if your backend uses different endpoints.
 * If your backend uses /api/auth/login etc., this already matches.
 */
const ENDPOINTS = {
  health: "/health",
  register: "/api/auth/register",
  login: "/api/auth/login",
  expenses: "/api/expenses"
};

const state = {
  token: localStorage.getItem("flowfund_token") || "",
  user: JSON.parse(localStorage.getItem("flowfund_user") || "null")
};

function $(id) {
  return document.getElementById(id);
}

function log(msg, type = "info") {
  const el = $("log");
  const time = new Date().toLocaleTimeString();
  const prefix = type === "error" ? "❌" : type === "ok" ? "✅" : "ℹ️";
  el.innerHTML += `${prefix} [${time}] ${escapeHtml(msg)}<br/>`;
  el.scrollTop = el.scrollHeight;
}

function escapeHtml(str) {
  return String(str)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}

function setApiStatus(text, ok = false) {
  const pill = $("apiStatus");
  pill.textContent = `API: ${text}`;
  pill.style.borderColor = ok ? "rgba(44,255,136,0.45)" : "rgba(255,77,109,0.35)";
  pill.style.color = ok ? "rgba(232,255,241,0.85)" : "rgba(255,200,210,0.9)";
}

function setSessionUI() {
  const info = $("sessionInfo");
  const btnLogout = $("btnLogout");
  const btnLoad = $("btnLoadExpenses");

  if (state.token) {
    const email = state.user?.email || "(unknown)";
    info.textContent = `Logged in as ${email}`;
    btnLogout.disabled = false;
    btnLoad.disabled = false;
    $("expensesList").classList.remove("muted");
    $("expensesList").textContent = "Click Load to fetch expenses.";
  } else {
    info.textContent = "Not logged in";
    btnLogout.disabled = true;
    btnLoad.disabled = true;
    $("expensesList").classList.add("muted");
    $("expensesList").textContent = "Login to load expenses.";
  }
}

async function apiFetch(path, options = {}) {
  const url = API_BASE + path;

  const headers = {
    "Content-Type": "application/json",
    ...(options.headers || {})
  };

  if (state.token) {
    headers.Authorization = `Bearer ${state.token}`;
  }

  const res = await fetch(url, {
    ...options,
    headers
  });

  let data = null;
  const text = await res.text();
  try { data = text ? JSON.parse(text) : null; } catch { data = text; }

  if (!res.ok) {
    const msg = typeof data === "string" ? data : (data?.message || data?.error || `HTTP ${res.status}`);
    throw new Error(msg);
  }

  return data;
}

async function checkHealth() {
  try {
    const data = await apiFetch(ENDPOINTS.health, { method: "GET", headers: {} });
    setApiStatus("online", true);
    log(`Health OK: ${JSON.stringify(data)}`, "ok");
  } catch (e) {
    setApiStatus("offline", false);
    log(`Health failed: ${e.message}`, "error");
    log(`If /health is not your route, update ENDPOINTS.health in app.js`, "error");
  }
}

function saveSession(token, user) {
  state.token = token || "";
  state.user = user || null;
  localStorage.setItem("flowfund_token", state.token);
  localStorage.setItem("flowfund_user", JSON.stringify(state.user));
  setSessionUI();
}

function clearSession() {
  saveSession("", null);
  log("Logged out.", "ok");
}

async function handleRegister(e) {
  e.preventDefault();
  const name = $("regName").value.trim();
  const email = $("regEmail").value.trim();
  const password = $("regPassword").value;

  if (!name || !email || !password) return;

  try {
    log("Registering…");
    const data = await apiFetch(ENDPOINTS.register, {
      method: "POST",
      body: JSON.stringify({ name, email, password })
    });

    log("Registered successfully.", "ok");

    // Some backends return token on register, some don't.
    // We'll try common shapes.
    const token = data?.token || data?.accessToken || data?.jwt || "";
    const user = data?.user || { email, name };

    if (token) {
      saveSession(token, user);
      log("Auto-logged in after register.", "ok");
    } else {
      log("Now login using the Login form.", "info");
    }
    e.target.reset();
  } catch (err) {
    log(`Register failed: ${err.message}`, "error");
  }
}

async function handleLogin(e) {
  e.preventDefault();
  const email = $("loginEmail").value.trim();
  const password = $("loginPassword").value;

  try {
    log("Logging in…");
    const data = await apiFetch(ENDPOINTS.login, {
      method: "POST",
      body: JSON.stringify({ email, password })
    });

    // Common response shapes:
    const token = data?.token || data?.accessToken || data?.jwt;
    const user = data?.user || { email };

    if (!token) {
      throw new Error("Login response did not include a token. Update app.js parsing to match your backend.");
    }

    saveSession(token, user);
    log("Login success.", "ok");
    e.target.reset();
  } catch (err) {
    log(`Login failed: ${err.message}`, "error");
  }
}

function renderExpenses(expenses) {
  const list = $("expensesList");
  if (!Array.isArray(expenses) || expenses.length === 0) {
    list.innerHTML = `<div class="muted">No expenses found.</div>`;
    return;
  }

  list.innerHTML = expenses
    .map((x) => {
      const title = escapeHtml(x.title ?? x.name ?? "Expense");
      const amount = Number(x.amount ?? x.value ?? 0).toFixed(2);
      const category = escapeHtml(x.category ?? "Other");
      const date = escapeHtml((x.date ?? x.createdAt ?? "").toString().slice(0, 10));

      return `
        <div class="item">
          <div class="item-top">
            <div class="item-title">${title}</div>
            <div class="mono">$${amount}</div>
          </div>
          <div class="item-meta">
            <span>📌 ${category}</span>
            ${date ? `<span>🗓️ ${date}</span>` : ""}
          </div>
        </div>
      `;
    })
    .join("");
}

async function loadExpenses() {
  if (!state.token) {
    log("Please login first.", "error");
    return;
  }

  try {
    log("Loading expenses…");
    const data = await apiFetch(ENDPOINTS.expenses, { method: "GET" });

    // Some APIs return { expenses: [...] }, some return [...]
    const expenses = Array.isArray(data) ? data : (data?.expenses || data?.data || []);
    renderExpenses(expenses);
    log(`Loaded ${expenses.length} expense(s).`, "ok");
  } catch (err) {
    log(`Load expenses failed: ${err.message}`, "error");
    log(`If your route is different, update ENDPOINTS.expenses in app.js`, "info");
  }
}

async function addExpense(e) {
  e.preventDefault();

  if (!state.token) {
    log("Login required to add expense.", "error");
    return;
  }

  const title = $("expTitle").value.trim();
  const amount = parseFloat($("expAmount").value);
  const category = $("expCategory").value;
  const date = $("expDate").value || undefined;

  try {
    log("Adding expense…");
    const payload = { title, amount, category };
    if (date) payload.date = date;

    const data = await apiFetch(ENDPOINTS.expenses, {
      method: "POST",
      body: JSON.stringify(payload)
    });

    log("Expense added.", "ok");
    e.target.reset();

    // Refresh list after add (optional)
    await loadExpenses();

    // Show what backend returned (helps debug)
    log(`Add response: ${JSON.stringify(data)}`, "info");
  } catch (err) {
    log(`Add expense failed: ${err.message}`, "error");
  }
}

function wireEvents() {
  $("formRegister").addEventListener("submit", handleRegister);
  $("formLogin").addEventListener("submit", handleLogin);
  $("formExpense").addEventListener("submit", addExpense);

  $("btnLoadExpenses").addEventListener("click", loadExpenses);
  $("btnLogout").addEventListener("click", () => {
    clearSession();
    renderExpenses([]);
  });
}

document.addEventListener("DOMContentLoaded", async () => {
  log("UI loaded. Wiring events…");
  wireEvents();
  setSessionUI();

  // Auto set date to today
  const today = new Date();
  const yyyy = today.getFullYear();
  const mm = String(today.getMonth() + 1).padStart(2, "0");
  const dd = String(today.getDate()).padStart(2, "0");
  const iso = `${yyyy}-${mm}-${dd}`;
  $("expDate").value = iso;

  await checkHealth();
});
