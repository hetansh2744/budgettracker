/* ===========================
   FlowFund GitHub Pages UI
   =========================== */

const API_BASE = "https://budgettracker-2-qmpz.onrender.com"; // Render API (no trailing slash)

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

function setApiStatus(text, ok = false) {
  const pill = $("apiStatus");
  if (!pill) return;
  pill.textContent = `API: ${text}`;
  pill.style.borderColor = ok ? "rgba(44,255,136,0.45)" : "rgba(255,77,109,0.35)";
  pill.style.color = ok ? "rgba(232,255,241,0.85)" : "rgba(255,200,210,0.9)";
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

async function apiFetch(path, options = {}) {
  const url = API_BASE + path;

  const headers = { ...(options.headers || {}) };

  // Set JSON header only if body exists
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
  }
}

async function handleRegister(e) {
  e.preventDefault();

  const name = $("regName")?.value?.trim() || "";
  const email = $("regEmail")?.value?.trim() || "";
  const password = $("regPassword")?.value || "";

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

    const token = data?.token || data?.accessToken || data?.jwt || "";
    const user = data?.user || { name, email };

    if (token) {
      saveSession(token, user);
      log("Auto-logged in after register.", "ok");
    } else {
      log("No token returned on register. Please login.", "info");
    }

    e.target.reset();
  } catch (err) {
    log(`Register failed: ${err.message}`, "error");
    log("If you see 'Failed to fetch', backend CORS/OPTIONS must be fixed.", "info");
  }
}

async function handleLogin(e) {
  e.preventDefault();

  const email = $("loginEmail")?.value?.trim() || "";
  const password = $("loginPassword")?.value || "";

  if (!email || !password) {
    log("Login: please fill all fields.", "error");
    return;
  }

  try {
    log("Logging in…");

    const data = await apiFetch(ENDPOINTS.login, {
      method: "POST",
      body: JSON.stringify({ email, password })
    });

    const token = data?.token || data?.accessToken || data?.jwt;
    const user = data?.user || { email };

    if (!token) {
      throw new Error("Login response did not include a token.");
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
  if (!list) return;

  if (!Array.isArray(expenses) || expenses.length === 0) {
    list.innerHTML = `<div class="muted">No expenses found.</div>`;
    return;
  }

  list.innerHTML = expenses
    .map((x) => {
      const title = escapeHtml(x.title ?? "Expense");
      const amount = Number(x.amount ?? 0).toFixed(2);
      const category = escapeHtml(x.category ?? "Other");
      const date = escapeHtml((x.date ?? "").toString().slice(0, 10));

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
  }
}

async function addExpense(e) {
  e.preventDefault();

  if (!state.token) {
    log("Login required to add expense.", "error");
    return;
  }

  const title = $("expTitle")?.value?.trim() || "";
  const amount = parseFloat($("expAmount")?.value || "");
  const category = $("expCategory")?.value || "Other";
  const date = $("expDate")?.value || "";

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

    log(`Expense added: ${JSON.stringify(data)}`, "ok");
    e.target.reset();
    await loadExpenses();
  } catch (err) {
    log(`Add expense failed: ${err.message}`, "error");
  }
}

function wireEvents() {
  $("formRegister")?.addEventListener("submit", handleRegister);
  $("formLogin")?.addEventListener("submit", handleLogin);
  $("formExpense")?.addEventListener("submit", addExpense);

  $("btnLoadExpenses")?.addEventListener("click", loadExpenses);
  $("btnLogout")?.addEventListener("click", () => {
    clearSession();
    renderExpenses([]);
  });

  document.addEventListener("click", (e) => {
    if (e.target?.tagName === "BUTTON") {
      log(`Button clicked: "${e.target.textContent.trim()}"`, "info");
    }
  });
}

document.addEventListener("DOMContentLoaded", async () => {
  log("UI loaded.", "ok");
  wireEvents();
  setSessionUI();

  // Set date to today
  const d = new Date();
  const iso = `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, "0")}-${String(d.getDate()).padStart(2, "0")}`;
  if ($("expDate")) $("expDate").value = iso;

  await checkHealth();
});
