/* FlowFund — Frontend (Server-backed with JWT)
   - Uses C++ backend on Render
   - Register/Login via prompts
   - Add/Edit/Delete transactions via API
   - Each user sees only their own data (private)
*/

const API_BASE = "https://budgettracker-2-qmpz.onrender.com"; // ✅ change if needed
const TOKEN_KEY = "flowfund.jwt.v1";

const $ = (id) => document.getElementById(id);

const els = {
  incomeAmount: $("incomeAmount"),
  expenseAmount: $("expenseAmount"),
  balanceAmount: $("balanceAmount"),
  txList: $("transactionList"),
  emptyState: $("emptyState"),
  addBtn: $("addBtn"),
  resetBtn: $("resetBtn"),

  modalBackdrop: $("modalBackdrop"),
  closeModalBtn: $("closeModalBtn"),
  cancelBtn: $("cancelBtn"),
  modalTitle: $("modalTitle"),

  form: $("txForm"),
  txType: $("txType"),
  txAmount: $("txAmount"),
  txDate: $("txDate"),
  txCategory: $("txCategory"),
  txTitle: $("txTitle"),
  txNote: $("txNote"),
  editId: $("editId"),

  typeFilter: $("typeFilter"),
  searchInput: $("searchInput"),
};

function todayISO() {
  const d = new Date();
  const yyyy = d.getFullYear();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  return `${yyyy}-${mm}-${dd}`;
}

function formatMoney(n) {
  const v = Number(n || 0);
  return `$${v.toFixed(2)}`;
}

function escapeHtml(s) {
  return String(s ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

function getToken() {
  return localStorage.getItem(TOKEN_KEY) || "";
}

function setToken(token) {
  if (!token) localStorage.removeItem(TOKEN_KEY);
  else localStorage.setItem(TOKEN_KEY, token);
}

async function apiFetch(path, { method = "GET", body } = {}) {
  const headers = { "Content-Type": "application/json" };
  const token = getToken();
  if (token) headers.Authorization = `Bearer ${token}`;

  const res = await fetch(`${API_BASE}${path}`, {
    method,
    headers,
    body: body ? JSON.stringify(body) : undefined,
  });

  const text = await res.text();
  let data = null;
  try {
    data = text ? JSON.parse(text) : null;
  } catch {
    data = { raw: text };
  }

  if (!res.ok) {
    const msg =
      data?.error?.message ||
      data?.message ||
      `Request failed: ${res.status} ${res.statusText}`;
    const code = data?.error?.code || "ERROR";
    const err = new Error(msg);
    err.code = code;
    err.status = res.status;
    throw err;
  }

  return data;
}

/* ---------------------------
   Auth UI (no HTML changes)
----------------------------*/

function ensureAuthButtons() {
  // inject small auth buttons into header-actions
  const headerActions = document.querySelector(".header-actions");
  if (!headerActions) return;

  if (document.getElementById("loginBtn")) return;

  const loginBtn = document.createElement("button");
  loginBtn.id = "loginBtn";
  loginBtn.className = "btn ghost";
  loginBtn.textContent = "Login";

  const registerBtn = document.createElement("button");
  registerBtn.id = "registerBtn";
  registerBtn.className = "btn ghost";
  registerBtn.textContent = "Register";

  const logoutBtn = document.createElement("button");
  logoutBtn.id = "logoutBtn";
  logoutBtn.className = "btn ghost";
  logoutBtn.textContent = "Logout";
  logoutBtn.style.display = "none";

  headerActions.prepend(logoutBtn);
  headerActions.prepend(registerBtn);
  headerActions.prepend(loginBtn);

  loginBtn.addEventListener("click", async () => {
    await loginFlow();
  });

  registerBtn.addEventListener("click", async () => {
    await registerFlow();
  });

  logoutBtn.addEventListener("click", () => {
    setToken("");
    transactions = [];
    renderAll();
    syncAuthButtons();
    alert("Logged out.");
  });

  syncAuthButtons();
}

function syncAuthButtons() {
  const token = getToken();
  const loginBtn = document.getElementById("loginBtn");
  const registerBtn = document.getElementById("registerBtn");
  const logoutBtn = document.getElementById("logoutBtn");
  if (!loginBtn || !registerBtn || !logoutBtn) return;

  const loggedIn = !!token;
  loginBtn.style.display = loggedIn ? "none" : "";
  registerBtn.style.display = loggedIn ? "none" : "";
  logoutBtn.style.display = loggedIn ? "" : "none";
}

async function loginFlow() {
  const email = prompt("Email:");
  if (!email) return;

  const password = prompt("Password:");
  if (!password) return;

  try {
    const data = await apiFetch("/auth/login", {
      method: "POST",
      body: { email, password },
    });
    setToken(data.token || "");
    syncAuthButtons();
    await loadFromServer();
    alert("Login successful ✅");
  } catch (e) {
    alert(`Login failed: ${e.message}`);
  }
}

async function registerFlow() {
  const name = prompt("Name:");
  if (!name) return;

  const email = prompt("Email:");
  if (!email) return;

  const password = prompt("Password (min 6 chars):");
  if (!password) return;

  try {
    const data = await apiFetch("/auth/register", {
      method: "POST",
      body: { name, email, password },
    });
    setToken(data.token || "");
    syncAuthButtons();
    await loadFromServer();
    alert("Registered & logged in ✅");
  } catch (e) {
    alert(`Register failed: ${e.message}`);
  }
}

/* ---------------------------
   UI Modal
----------------------------*/

function openModal(mode, tx) {
  els.modalBackdrop.classList.remove("hidden");
  els.modalBackdrop.setAttribute("aria-hidden", "false");

  els.editId.value = tx?.id ?? "";
  els.modalTitle.textContent = mode === "edit" ? "Edit Transaction" : "Add Transaction";

  els.txType.value = tx?.type || "INCOME";
  els.txAmount.value = tx?.amount ?? "";
  els.txDate.value = tx?.date || todayISO();
  els.txCategory.value = tx?.category || "";
  els.txTitle.value = tx?.title || "";
  els.txNote.value = tx?.note || "";

  setTimeout(() => els.txAmount.focus(), 0);
}

function closeModal() {
  els.modalBackdrop.classList.add("hidden");
  els.modalBackdrop.setAttribute("aria-hidden", "true");
  els.form.reset();
  els.editId.value = "";
}

/* ---------------------------
   State + Rendering
----------------------------*/

let transactions = []; // server-backed list

function computeSummary(list) {
  let income = 0;
  let expense = 0;

  for (const t of list) {
    const amt = Number(t.amount || 0);
    if (t.type === "INCOME") income += amt;
    else expense += amt;
  }

  return { income, expense, balance: income - expense };
}

function applyFilters(list) {
  const type = els.typeFilter.value;
  const q = els.searchInput.value.trim().toLowerCase();

  return list.filter((t) => {
    const matchesType = type === "ALL" || t.type === type;
    const hay = `${t.title} ${t.category} ${t.note} ${t.date}`.toLowerCase();
    const matchesQ = !q || hay.includes(q);
    return matchesType && matchesQ;
  });
}

function renderCards() {
  const { income, expense, balance } = computeSummary(transactions);
  els.incomeAmount.textContent = formatMoney(income);
  els.expenseAmount.textContent = formatMoney(expense);
  els.balanceAmount.textContent = formatMoney(balance);
}

function renderList() {
  const filtered = applyFilters(
    [...transactions].sort((a, b) => (b.date || "").localeCompare(a.date || ""))
  );

  els.txList.innerHTML = "";

  if (filtered.length === 0) {
    els.emptyState.classList.remove("hidden");
    return;
  }
  els.emptyState.classList.add("hidden");

  for (const t of filtered) {
    const isIncome = t.type === "INCOME";
    const sign = isIncome ? "+" : "-";
    const amountClass = isIncome ? "amount-income" : "amount-expense";

    const li = document.createElement("li");
    li.className = "tx";

    li.innerHTML = `
      <div>
        <div class="tx-title">${escapeHtml(t.title)}</div>
        <div class="tx-meta">${escapeHtml(t.date)} • ${escapeHtml(t.category)}</div>
      </div>

      <div class="tx-right">
        <div class="tx-amount ${amountClass}">${sign}${formatMoney(t.amount).slice(1)}</div>
        <div class="tx-actions">
          <button class="icon-btn" data-action="edit" data-id="${t.id}">Edit</button>
          <button class="icon-btn" data-action="delete" data-id="${t.id}">Delete</button>
        </div>
      </div>
    `;

    els.txList.appendChild(li);
  }
}

function renderAll() {
  renderCards();
  renderList();
}

/* ---------------------------
   Server operations
----------------------------*/

async function loadFromServer() {
  if (!getToken()) {
    transactions = [];
    renderAll();
    return;
  }

  // Get list
  const txData = await apiFetch("/transactions");
  transactions = (txData.items || []).map((t) => ({
    id: String(t.id),
    type: t.type,
    amount: Number(t.amount),
    date: t.date,
    category: t.category,
    title: t.title,
    note: t.note || "",
    currency: t.currency || "CAD",
  }));

  // Optionally you can also call /summary, but UI already calculates from list
  renderAll();
}

async function createTransaction(tx) {
  const body = {
    type: tx.type,
    amount: Number(tx.amount),
    date: tx.date,
    category: tx.category,
    title: tx.title,
    note: tx.note || "",
    currency: tx.currency || "CAD",
  };

  const data = await apiFetch("/transactions", { method: "POST", body });
  return String(data.id);
}

async function updateTransaction(id, tx) {
  const body = {
    type: tx.type,
    amount: Number(tx.amount),
    date: tx.date,
    category: tx.category,
    title: tx.title,
    note: tx.note || "",
    currency: tx.currency || "CAD",
  };

  await apiFetch(`/transactions/${id}`, { method: "PUT", body });
}

async function deleteTransactionOnServer(id) {
  await apiFetch(`/transactions/${id}`, { method: "DELETE" });
}

/* ---------------------------
   Events
----------------------------*/

els.addBtn.addEventListener("click", async () => {
  if (!getToken()) {
    const go = confirm("You must login to save private transactions.\n\nPress OK to Login.");
    if (go) await loginFlow();
    if (!getToken()) return;
  }
  openModal("add");
});

els.resetBtn.addEventListener("click", async () => {
  // Since server data is per-user, "Reset Demo" doesn't really apply.
  // We'll interpret it as "logout + clear UI" (safe + simple).
  if (getToken()) {
    const ok = confirm("This will log you out (your server data stays safe in DB). Continue?");
    if (!ok) return;
    setToken("");
    transactions = [];
    syncAuthButtons();
    renderAll();
    return;
  }
  alert("Nothing to reset. Login to start using FlowFund.");
});

els.closeModalBtn.addEventListener("click", closeModal);
els.cancelBtn.addEventListener("click", closeModal);

els.modalBackdrop.addEventListener("click", (e) => {
  if (e.target === els.modalBackdrop) closeModal();
});

document.addEventListener("keydown", (e) => {
  if (e.key === "Escape" && !els.modalBackdrop.classList.contains("hidden")) closeModal();
});

els.form.addEventListener("submit", async (e) => {
  e.preventDefault();

  if (!getToken()) {
    alert("You are not logged in.");
    closeModal();
    return;
  }

  const id = els.editId.value || "";
  const type = els.txType.value;
  const amount = Number(els.txAmount.value);

  const date = els.txDate.value;
  const category = els.txCategory.value.trim();
  const title = els.txTitle.value.trim();
  const note = els.txNote.value.trim();

  if (!date || !category || !title || !(amount > 0)) {
    alert("Please fill in all required fields correctly.");
    return;
  }

  const tx = { id, type, amount, date, category, title, note, currency: "CAD" };

  try {
    if (id) {
      await updateTransaction(id, tx);
    } else {
      const newId = await createTransaction(tx);
      tx.id = newId;
    }

    await loadFromServer();
    closeModal();
  } catch (err) {
    alert(`Save failed: ${err.message}`);
  }
});

els.txList.addEventListener("click", async (e) => {
  const btn = e.target.closest("button");
  if (!btn) return;

  const action = btn.dataset.action;
  const id = btn.dataset.id;

  const tx = transactions.find((t) => t.id === id);

  if (action === "edit" && tx) {
    openModal("edit", tx);
    return;
  }

  if (action === "delete") {
    const ok = confirm("Delete this transaction?");
    if (!ok) return;

    try {
      await deleteTransactionOnServer(id);
      await loadFromServer();
    } catch (err) {
      alert(`Delete failed: ${err.message}`);
    }
  }
});

els.typeFilter.addEventListener("change", renderList);
els.searchInput.addEventListener("input", renderList);

/* ---------------------------
   Init
----------------------------*/

ensureAuthButtons();
loadFromServer().catch((e) => {
  // If backend is sleeping on free Render, first request may fail.
  console.error(e);
  renderAll();
});
