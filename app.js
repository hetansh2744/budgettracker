/* FlowFund — Frontend (GitHub Pages) + Backend (Render API)
   - Register / Login (JWT)
   - Create + List transactions from API
   - Summary from API
*/

const API_BASE = "https://budgettracker-2-qmpz.onrender.com"; // Render API
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

function getToken() {
  return localStorage.getItem(TOKEN_KEY) || "";
}
function setToken(t) {
  if (!t) localStorage.removeItem(TOKEN_KEY);
  else localStorage.setItem(TOKEN_KEY, t);
}

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

async function api(path, { method = "GET", body } = {}) {
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
  try { data = text ? JSON.parse(text) : null; } catch { data = text; }

  if (!res.ok) {
    const msg =
      data?.error?.message ||
      (typeof data === "string" ? data : "Request failed");
    throw new Error(msg);
  }
  return data;
}

/* ---------- Auth UI (prompt-based, minimal) ---------- */

async function ensureLoggedIn() {
  if (getToken()) return true;

  const choice = prompt(
    "You are not logged in.\nType 1 to Login, 2 to Register, or Cancel to stop:"
  );
  if (choice !== "1" && choice !== "2") return false;

  if (choice === "2") {
    const name = prompt("Name:");
    const email = prompt("Email:");
    const password = prompt("Password (min 6 chars):");
    if (!name || !email || !password) return false;

    const out = await api("/auth/register", {
      method: "POST",
      body: { name, email, password },
    });
    setToken(out.token);
    alert("Registered & logged in!");
    return true;
  }

  // login
  const email = prompt("Email:");
  const password = prompt("Password:");
  if (!email || !password) return false;

  const out = await api("/auth/login", {
    method: "POST",
    body: { email, password },
  });
  setToken(out.token);
  alert("Logged in!");
  return true;
}

function logout() {
  setToken("");
  alert("Logged out.");
  // Clear UI
  transactions = [];
  renderAll();
}

/* ---------- Data / Render ---------- */

let transactions = [];

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

function renderCards(summary) {
  els.incomeAmount.textContent = formatMoney(summary.income);
  els.expenseAmount.textContent = formatMoney(summary.expense);
  els.balanceAmount.textContent = formatMoney(summary.balance);
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
        <div class="tx-meta">${escapeHtml(t.date)} • ${escapeHtml(
      t.category
    )}</div>
      </div>

      <div class="tx-right">
        <div class="tx-amount ${amountClass}">${sign}${formatMoney(
      t.amount
    ).slice(1)}</div>
        <div class="tx-actions">
          <button class="icon-btn" data-action="delete" data-id="${t.id}">Delete</button>
        </div>
      </div>
    `;

    els.txList.appendChild(li);
  }
}

async function refreshFromServer() {
  const ok = await ensureLoggedIn();
  if (!ok) return;

  const [txOut, summary] = await Promise.all([
    api("/transactions"),
    api("/summary"),
  ]);

  transactions = txOut.items || [];
  renderCards(summary);
  renderList();
}

function openModal(mode) {
  els.modalBackdrop.classList.remove("hidden");
  els.modalBackdrop.setAttribute("aria-hidden", "false");

  els.editId.value = "";
  els.modalTitle.textContent = mode === "edit" ? "Edit Transaction" : "Add Transaction";

  els.txType.value = "INCOME";
  els.txAmount.value = "";
  els.txDate.value = todayISO();
  els.txCategory.value = "";
  els.txTitle.value = "";
  els.txNote.value = "";

  setTimeout(() => els.txAmount.focus(), 0);
}

function closeModal() {
  els.modalBackdrop.classList.add("hidden");
  els.modalBackdrop.setAttribute("aria-hidden", "true");
  els.form.reset();
  els.editId.value = "";
}

async function createTransaction(tx) {
  await api("/transactions", { method: "POST", body: tx });
  await refreshFromServer();
}

/* ---------- Events ---------- */

els.addBtn.addEventListener("click", async () => {
  const ok = await ensureLoggedIn();
  if (!ok) return;
  openModal("add");
});

// "Reset Demo" no longer makes sense once you use the server.
// We'll repurpose it as Logout for a clean demo.
els.resetBtn.textContent = "Logout";
els.resetBtn.title = "Logs out from this browser";
els.resetBtn.addEventListener("click", logout);

els.closeModalBtn.addEventListener("click", closeModal);
els.cancelBtn.addEventListener("click", closeModal);

els.modalBackdrop.addEventListener("click", (e) => {
  if (e.target === els.modalBackdrop) closeModal();
});

document.addEventListener("keydown", (e) => {
  if (e.key === "Escape" && !els.modalBackdrop.classList.contains("hidden"))
    closeModal();
});

els.form.addEventListener("submit", async (e) => {
  e.preventDefault();

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

  try {
    await createTransaction({ type, amount, date, category, title, note });
    closeModal();
  } catch (err) {
    alert(err.message || "Failed to save transaction");
  }
});

els.txList.addEventListener("click", async (e) => {
  const btn = e.target.closest("button");
  if (!btn) return;

  const action = btn.dataset.action;
  const id = btn.dataset.id;

  if (action === "delete") {
    alert(
      "Delete is not implemented on the backend yet.\n\nIf you want, I can add:\nDELETE /transactions/:id"
    );
  }
});

els.typeFilter.addEventListener("change", renderList);
els.searchInput.addEventListener("input", renderList);

/* Init */
refreshFromServer().catch(() => {
  // If not logged in, show zero summary UI
  els.incomeAmount.textContent = "$0.00";
  els.expenseAmount.textContent = "$0.00";
  els.balanceAmount.textContent = "$0.00";
  transactions = [];
  renderList();
});
