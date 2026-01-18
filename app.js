/* ===========================
   FlowFund GitHub Pages UI
   =========================== */

/**
 * IMPORTANT:
 * Set this to your Render base URL (no trailing slash)
 */
const API_BASE = "https://budgettracker-2-qmpz.onrender.com";

/**
 * Update these if your backend routes differ.
 */
const ENDPOINTS = {
  health: "/health",
  register: "/api/auth/register",
  login: "/api/auth/login",
  expenses: "/api/expenses"
};

const STORAGE = {
  token: "flowfund_token",
  user: "flowfund_user"
};

const state = {
  token: localStorage.getItem(STORAGE.token) || "",
  user: safeJsonParse(localStorage.getItem(STORAGE.user)) || null
};

function $(id) {
  return document.getElementById(id);
}

function safeJsonParse(v) {
  try {
    return v ? JSON.parse(v) : null;
  } catch {
    return null;
  }
}

function escapeHtml(str) {
  return String(str)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}

function log(msg, type = "info") {
  const el = $("log");
  if (!el) return;

  const time = new Date().toLocaleTimeString();
  const prefix = type === "error" ? "❌" : type === "ok" ? "✅" : "ℹ️";
  el.innerHTML += `${prefix} [${time}] ${escapeHtml(msg)}<br/>`;
  el.scrollTop = el.scrollHeight;
}

function must(id) {
  const el = $(id);
  if (!el) {
    // This is the #1 reason buttons "do nothing"
    console.error(`Missing element with id="${id}"`);
    log(`Missing element with id="${id}" (check index.html)`, "error");
  }
  return el;
}

function setApiStatus(text, ok = false) {
  const pill = $("apiStatus");
  if (!pill) return;
  pill.textContent = `API: ${text}`;
  pill.style.borderColor = ok ? "rgba(44,255,136,0.45)" : "rgba(255,77,109,0.35)";
  pill.style.color = ok ? "rgba(232,255,241,0.85)" : "rgba(255,200,210,0.9)";
}

function saveSession(token, user) {
  state.token = token || "";
  state.user = user || null;

  localStorage.setItem(STORAGE.token, state.token);
  localStorage.setItem(STORAGE.user, JSON.stringify(state.user));

  setSessionUI();
}

function clearSession() {
  saveSession("", null);
  log("Logged out.", "ok");
}

function setSessionUI() {
  const info = $("sessionInfo");
  const btnLogout = $("btnLogout");
  const btnLoad = $("btnLoadExpenses");
  const list = $("expensesList");

  if (state.token) {
    const email = state.user?.email || "(unknown)";
    if (info) info.textContent = `Logged in as ${email}`;
    if (btnLogout) btnLogout.disabled = false;
    if (btnLoad) btnLoad.disabled = false;

    if (list) {
      list.classList.remove("muted");
      list.textContent = "Click Load to fetch expenses.";
    }
  } else {
    if (info) info.textContent = "Not logged in";
    if (btnLogout) btnLogout.disabled = true;
    if (btnLoad) btnLoad.disabled = true;

    if (list) {
      list.classList.add("muted");
      list.textContent = "Login to load expenses.";
    }
  }
}

async function apiFetch(path, options = {}) {
  const url = API_BASE + path;

  const headers = {
    ...(options.headers || {})
  };

  // Only set JSON content-type when we have a body
  if (options.body && !headers["Content-Type"]) {
    headers["Content-Type"] = "application/json";
  }

  if (state.token) {
    headers.Authorization = `Bearer ${state.token}`;
  }

  let res;
  try {
    res = await fetch(url, { ...options, headers });
  } catch (e) {
    // Network / DNS / blocked
    throw new Error(`Network error calling API (${url}). ${e.message}`);
  }

  const raw = await res.text();
  let data = raw;
  try {
    data = raw ? JSON.parse(raw) : null;
  } catch {
    // keep as text
  }

  if (!res.ok) {
    const msg =
      typeof data === "string"
        ? data
        : (data?.message || data?.error || `HTTP ${res.status}`);
    throw new Error(msg);
  }

  return data;
}

async function checkHealth() {
  try {
    const data = await apiFetch(ENDPOINTS.health, { method: "GET" });
    setApiStatus("online", true);
    log(`Health OK: ${JSON.stringify(data)}`, "ok");
  } catch (e) {
    setApiStatus("offline", false);
    log(`Health failed: ${e.message}`, "error");
    log(`If your health route differs, change ENDPOINTS.health`, "error");
  }
}

async function handleRegister(e) {
  e.preventDefault();

  const name = must("regName")?.value?.trim() || "";
  const email = must("regEmail")?.value?.trim() || "";
  const password = must("regPassword")?.value || "";

  if (!name || !email || !password) {
    log("Register: please fill all fields.", "error");
    return;
  }

  try {
    log("Registering…");

    const data = await apiFetch(ENDPOINTS.register, {
      method: "POST",
      body: JSON.stringify({ name, email, password })
    });

    log(`Registered. Response: ${JSON.stringify(data)}`, "ok");

    // Try common token shapes
    const token = data?.token || data?.accessToken || data?.jwt || "";
    const user = data?.user || { email, name };

    if (token) {
      saveSession(token, user);
      log("Auto-logged in after register.", "ok");
    } else {
      log("No token returned on register. Use Login form now.", "info");
    }

    e.target.reset();
  } catch (err) {
    log(`Register failed: ${err.message}`, "error");
    log("Most common causes: wrong endpoint OR CORS blocked OR backend error.", "info");
  }
}

async function handleLogin(e) {
  e.preventDefault();

  const email = must("loginEmail")?.value?.trim() || "";
  const password = must("loginPassword")?.value || "";

  if (!email || !password) {
    log("Login: please fill email and password.", "error");
    return;
  }

  try {
    log("Logging in…");

    const data = await apiFetch(ENDPOINTS.login, {
      method: "POST",
      body: JSON.stringify({ email, password })
    });

    log(`Login response: ${JSON.stringify(data)}`, "info");

    const token = data?.token || data?.accessToken || data?.jwt;
    const user = data?.user || { email };

    if (!token) {
      throw new Error("Login response did not include a token (check backend response shape).");
    }

    saveSession(token, user);
    log("Login success.", "ok");
    e.target.reset();
  } catch (err) {
    log(`Login failed: ${err.message}`, "error");
    log("If you see CORS in browser console, backend must allow GitHub Pages origin.", "info");
  }
}

function renderExpenses(expenses) {
  const list = $("expensesList");
  if (!list) return;

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
    const expenses = Array.isArray(data) ? data : (data?.expenses || data?.data || []);

    renderExpenses(expenses);
    log(`Loaded ${expenses.length} expense(s).`, "ok");
  } catch (err) {
    log(`Load expenses failed: ${err.message}`, "error");
    log("Likely: wrong endpoint OR token rejected OR CORS.", "info");
  }
}

async function addExpense(e) {
  e.preventDefault();

  if (!state.token) {
    log("Login required to add expense.", "error");
    return;
  }

  const title = must("expTitle")?.value?.trim() || "";
  const amountRaw = must("expAmount")?.value || "";
  const category = must("expCategory")?.value || "Other";
  const date = must("expDate")?.value || "";

  const amount = parseFloat(amountRaw);

  if (!title || Number.isNaN(amount)) {
    log("Please enter a valid title and amount.", "error");
    return;
  }

  try {
    log("Adding expense…");

    const payload = { title, amount, category };
    if (date) payload.date = date;

    const data = await apiFetch(ENDPOINTS.expenses, {
      method: "POST",
      body: JSON.stringify(payload)
    });

    log(`Expense added. Response: ${JSON.stringify(data)}`, "ok");
    e.target.reset();

    // Refresh list
    await loadExpenses();
  } catch (err) {
    log(`Add expense failed: ${err.message}`, "error");
  }
}

function wireEvents() {
  const formRegister = must("formRegister");
  const formLogin = must("formLogin");
  const formExpense = must("formExpense");

  const btnLoad = must("btnLoadExpenses");
  const btnLogout = must("btnLogout");

  if (formRegister) formRegister.addEventListener("submit", handleRegister);
  if (formLogin) formLogin.addEventListener("submit", handleLogin);
  if (formExpense) formExpense.addEventListener("submit", addExpense);

  if (btnLoad) btnLoad.addEventListener("click", loadExpenses);
  if (btnLogout) btnLogout.addEventListener("click", () => {
    clearSession();
    renderExpenses([]);
  });

  // Extra: log any clicks (helps confirm JS is running)
  document.addEventListener("click", (e) => {
    const t = e.target;
    if (t && t.tagName === "BUTTON") {
      log(`Button clicked: "${t.textContent.trim()}"`, "info");
    }
  });

  log("Events wired.", "ok");
}

document.addEventListener("DOMContentLoaded", async () => {
  log("UI loaded.", "ok");

  wireEvents();
  setSessionUI();

  // Set date input to today (local)
  const d = new Date();
  const yyyy = d.getFullYear();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  const iso = `${yyyy}-${mm}-${dd}`;
  const expDate = $("expDate");
  if (expDate) expDate.value = iso;

  await checkHealth();
});
