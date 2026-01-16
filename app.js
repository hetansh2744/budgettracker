// =============================
// FlowFund Frontend (GitHub Pages)
// =============================

// ✅ Your Render backend:
const API_BASE = "https://budgettracker-2-qmpz.onrender.com";

// ---------- Token helpers ----------
function getToken() {
  return localStorage.getItem("token") || "";
}

function setToken(token) {
  localStorage.setItem("token", token);
}

function clearToken() {
  localStorage.removeItem("token");
}

function isLoggedIn() {
  return !!getToken();
}

// ---------- Small UI helpers ----------
function $(id) {
  return document.getElementById(id);
}

function show(el) {
  if (el) el.style.display = "block";
}

function hide(el) {
  if (el) el.style.display = "none";
}

function setText(el, text) {
  if (el) el.textContent = text;
}

function showError(msg) {
  const el = $("error");
  if (el) {
    el.textContent = msg;
    el.style.display = "block";
  } else {
    alert(msg);
  }
}

function clearError() {
  const el = $("error");
  if (el) {
    el.textContent = "";
    el.style.display = "none";
  }
}

// ---------- API wrapper (adds Authorization automatically) ----------
async function api(path, { method = "GET", body = null, auth = false } = {}) {
  const headers = { "Content-Type": "application/json" };

  if (auth) {
    const token = getToken();
    if (!token) {
      // Don’t spam login loop — just show auth screen once.
      throw { status: 401, message: "Not logged in" };
    }
    headers["Authorization"] = `Bearer ${token}`;
  }

  const res = await fetch(`${API_BASE}${path}`, {
    method,
    headers,
    body: body ? JSON.stringify(body) : null
  });

  // If backend returns non-JSON sometimes, be safe:
  const text = await res.text();
  let data = null;
  try { data = text ? JSON.parse(text) : null; } catch { data = null; }

  if (!res.ok) {
    const msg =
      data?.error?.message ||
      data?.message ||
      `Request failed (${res.status})`;
    throw { status: res.status, message: msg, data };
  }

  return data;
}

// ---------- Auth UI control ----------
function renderAuthState() {
  const authBox = $("authBox");       // login/register container
  const appBox = $("appBox");         // main app container
  const logoutBtn = $("logoutBtn");   // optional button

  clearError();

  if (isLoggedIn()) {
    hide(authBox);
    show(appBox);
    show(logoutBtn);
  } else {
    show(authBox);
    hide(appBox);
    hide(logoutBtn);
  }
}

// ---------- Auth actions ----------
async function onRegister(e) {
  e.preventDefault();
  clearError();

  const name = $("regName")?.value?.trim() || "";
  const email = $("regEmail")?.value?.trim() || "";
  const password = $("regPassword")?.value || "";

  if (!name || !email || password.length < 6) {
    return showError("Register: name, email, password (>=6) required.");
  }

  try {
    const data = await api("/auth/register", {
      method: "POST",
      body: { name, email, password },
      auth: false
    });

    if (!data?.token) return showError("Register failed: No token received.");
    setToken(data.token);

    renderAuthState();
    await refreshAll();
  } catch (err) {
    showError(`Register failed: ${err.message || "Unknown error"}`);
  }
}

async function onLogin(e) {
  e.preventDefault();
  clearError();

  const email = $("loginEmail")?.value?.trim() || "";
  const password = $("loginPassword")?.value || "";

  if (!email || !password) {
    return showError("Login: email and password required.");
  }

  try {
    const data = await api("/auth/login", {
      method: "POST",
      body: { email, password },
      auth: false
    });

    if (!data?.token) return showError("Login failed: No token received.");
    setToken(data.token);

    renderAuthState();
    await refreshAll();
  } catch (err) {
    showError(`Login failed: ${err.message || "Unknown error"}`);
  }
}

function onLogout() {
  clearToken();
  renderAuthState();
  // Optional: clear UI
  setText($("summary"), "");
  const list = $("txList");
  if (list) list.innerHTML = "";
}

// ---------- Transactions ----------
async function onAddTransaction(e) {
  e.preventDefault();
  clearError();

  const type = $("txType")?.value || "EXPENSE";
  const amount = Number($("txAmount")?.value || 0);
  const date = $("txDate")?.value || "";
  const category = $("txCategory")?.value?.trim() || "";
  const title = $("txTitle")?.value?.trim() || "";
  const note = $("txNote")?.value?.trim() || "";
  const currency = $("txCurrency")?.value || "CAD";

  if (!["INCOME", "EXPENSE"].includes(type)) return showError("Type must be INCOME or EXPENSE.");
  if (!amount || amount <= 0) return showError("Amount must be > 0.");
  if (!date || !category || !title) return showError("date, category, title required.");

  try {
    await api("/transactions", {
      method: "POST",
      body: { type, amount, date, category, title, note, currency },
      auth: true
    });

    // reset some fields (optional)
    if ($("txAmount")) $("txAmount").value = "";
    if ($("txTitle")) $("txTitle").value = "";
    if ($("txNote")) $("txNote").value = "";

    await refreshAll();
  } catch (err) {
    if (err.status === 401) {
      // Token invalid/expired or missing — go to login once
      clearToken();
      renderAuthState();
      return showError("Session expired. Please login again.");
    }
    showError(`Add transaction failed: ${err.message || "Unknown error"}`);
  }
}

async function loadTransactions() {
  const list = $("txList");
  if (!list) return;

  list.innerHTML = "Loading...";

  try {
    const data = await api("/transactions", { auth: true });
    const items = data?.items || [];

    if (items.length === 0) {
      list.innerHTML = "<li>No transactions yet.</li>";
      return;
    }

    list.innerHTML = items.map(tx => {
      const sign = tx.type === "EXPENSE" ? "-" : "+";
      return `<li>
        <strong>${tx.title}</strong> (${tx.category}) — ${sign}${tx.amount} ${tx.currency}
        <br/>
        <small>${tx.date}${tx.note ? " • " + tx.note : ""}</small>
      </li>`;
    }).join("");
  } catch (err) {
    if (err.status === 401) {
      clearToken();
      renderAuthState();
      showError("Session expired. Please login again.");
      return;
    }
    list.innerHTML = "<li>Failed to load transactions.</li>";
    showError(err.message || "Failed to load transactions.");
  }
}

async function loadSummary() {
  const el = $("summary");
  if (!el) return;

  setText(el, "Loading...");

  try {
    const data = await api("/summary", { auth: true });
    const income = data?.income ?? 0;
    const expense = data?.expense ?? 0;
    const balance = data?.balance ?? 0;

    setText(el, `Income: ${income} | Expense: ${expense} | Balance: ${balance}`);
  } catch (err) {
    if (err.status === 401) {
      clearToken();
      renderAuthState();
      showError("Session expired. Please login again.");
      return;
    }
    setText(el, "");
    showError(err.message || "Failed to load summary.");
  }
}

async function refreshAll() {
  await Promise.all([loadSummary(), loadTransactions()]);
}

// ---------- Wire up events ----------
function init() {
  // forms/buttons (IDs expected in HTML)
  $("registerForm")?.addEventListener("submit", onRegister);
  $("loginForm")?.addEventListener("submit", onLogin);
  $("txForm")?.addEventListener("submit", onAddTransaction);
  $("logoutBtn")?.addEventListener("click", onLogout);

  renderAuthState();

  // If already logged in, load data
  if (isLoggedIn()) {
    refreshAll().catch(() => {});
  }
}

document.addEventListener("DOMContentLoaded", init);
