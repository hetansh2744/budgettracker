// ============================
// Finance Tracker (Interactive)
// - Works on GitHub Pages via localStorage
// - Add/Edit/Delete transactions
// - Live summaries
// ============================

const STORE_KEY = "finance_tracker_transactions_v1";

// ---- DOM ----
const elIncome = document.getElementById("incomeAmount");
const elExpense = document.getElementById("expenseAmount");
const elBalance = document.getElementById("balanceAmount");
const elList = document.getElementById("transactionList");
const elEmpty = document.getElementById("emptyState");

const btnAdd = document.getElementById("btnAdd");
const btnReset = document.getElementById("btnReset");

const filterType = document.getElementById("filterType");
const searchText = document.getElementById("searchText");
const sortBy = document.getElementById("sortBy");

// Modal/form
const overlay = document.getElementById("modalOverlay");
const btnClose = document.getElementById("btnClose");
const btnCancel = document.getElementById("btnCancel");
const form = document.getElementById("txForm");
const modalTitle = document.getElementById("modalTitle");

const txId = document.getElementById("txId");
const txType = document.getElementById("txType");
const txAmount = document.getElementById("txAmount");
const txDate = document.getElementById("txDate");
const txCurrency = document.getElementById("txCurrency");
const txDesc = document.getElementById("txDesc");
const txCategory = document.getElementById("txCategory");
const txNote = document.getElementById("txNote");

// ---- Helpers ----
function money(n) {
  const v = Number(n || 0);
  return `$${v.toFixed(2)}`;
}

function todayISO() {
  const d = new Date();
  const yyyy = d.getFullYear();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  return `${yyyy}-${mm}-${dd}`;
}

function uid() {
  return `${Date.now()}_${Math.random().toString(16).slice(2)}`;
}

function loadTransactions() {
  try {
    const raw = localStorage.getItem(STORE_KEY);
    if (!raw) return seedData();
    const parsed = JSON.parse(raw);
    if (!Array.isArray(parsed)) return seedData();
    return parsed;
  } catch {
    return seedData();
  }
}

function saveTransactions(items) {
  localStorage.setItem(STORE_KEY, JSON.stringify(items));
}

function seedData() {
  const demo = [
    { id: uid(), type: "INCOME", amount: 4000, currency: "CAD", date: todayISO(), description: "Salary", category: "Salary", note: "" },
    { id: uid(), type: "EXPENSE", amount: 1200, currency: "CAD", date: todayISO(), description: "Rent", category: "Bills", note: "" },
    { id: uid(), type: "EXPENSE", amount: 250, currency: "CAD", date: todayISO(), description: "Groceries", category: "Food", note: "" },
    { id: uid(), type: "INCOME", amount: 200, currency: "CAD", date: todayISO(), description: "Freelance", category: "Side Hustle", note: "" },
  ];
  saveTransactions(demo);
  return demo;
}

// ---- State ----
let transactions = loadTransactions();

// ---- Render ----
function computeSummary(items) {
  let income = 0, expense = 0;
  for (const t of items) {
    if (t.type === "INCOME") income += Number(t.amount || 0);
    else expense += Number(t.amount || 0);
  }
  return { income, expense, balance: income - expense };
}

function applyFilters(items) {
  const type = filterType.value;
  const q = (searchText.value || "").trim().toLowerCase();
  let out = [...items];

  if (type !== "ALL") out = out.filter(x => x.type === type);

  if (q) {
    out = out.filter(x => {
      const hay = `${x.description||""} ${x.category||""} ${x.note||""}`.toLowerCase();
      return hay.includes(q);
    });
  }

  return out;
}

function applySort(items) {
  const mode = sortBy.value;
  const out = [...items];

  if (mode === "DATE_DESC") out.sort((a,b) => (b.date||"").localeCompare(a.date||""));
  if (mode === "DATE_ASC") out.sort((a,b) => (a.date||"").localeCompare(b.date||""));
  if (mode === "AMOUNT_DESC") out.sort((a,b) => Number(b.amount||0) - Number(a.amount||0));
  if (mode === "AMOUNT_ASC") out.sort((a,b) => Number(a.amount||0) - Number(b.amount||0));

  return out;
}

function render() {
  // cards from ALL transactions (not just filtered list)
  const { income, expense, balance } = computeSummary(transactions);
  elIncome.textContent = money(income);
  elExpense.textContent = money(expense);
  elBalance.textContent = money(balance);

  // list view
  let view = applyFilters(transactions);
  view = applySort(view);

  elList.innerHTML = "";
  elEmpty.style.display = view.length ? "none" : "block";

  for (const t of view) {
    const li = document.createElement("li");
    li.className = "item";

    const amtClass = t.type === "INCOME" ? "income" : "expense";
    const sign = t.type === "INCOME" ? "+" : "-";

    const left = document.createElement("div");
    left.innerHTML = `
      <div class="item-title">${escapeHtml(t.description || "(no description)")}</div>
      <div class="item-meta">
        ${escapeHtml(t.date || "")}
        ${t.category ? ` • ${escapeHtml(t.category)}` : ""}
        ${t.note ? ` • ${escapeHtml(t.note)}` : ""}
      </div>
      <div class="row-actions">
        <button class="small-btn" data-action="edit" data-id="${t.id}">Edit</button>
        <button class="small-btn" data-action="delete" data-id="${t.id}">Delete</button>
      </div>
    `;

    const right = document.createElement("div");
    right.innerHTML = `<div class="amount ${amtClass}">${sign}${money(Number(t.amount||0)).replace("$","$")}</div>`;

    li.appendChild(left);
    li.appendChild(right);
    elList.appendChild(li);
  }
}

function escapeHtml(s) {
  return String(s).replace(/[&<>"']/g, (m) => ({
    "&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"
  }[m]));
}

// ---- Modal ----
function openModal(mode, tx = null) {
  overlay.classList.remove("hidden");
  overlay.setAttribute("aria-hidden", "false");

  if (mode === "add") {
    modalTitle.textContent = "Add transaction";
    txId.value = "";
    txType.value = "EXPENSE";
    txAmount.value = "";
    txDate.value = todayISO();
    txCurrency.value = "CAD";
    txDesc.value = "";
    txCategory.value = "";
    txNote.value = "";
  } else {
    modalTitle.textContent = "Edit transaction";
    txId.value = tx.id;
    txType.value = tx.type;
    txAmount.value = tx.amount;
    txDate.value = tx.date;
    txCurrency.value = tx.currency || "CAD";
    txDesc.value = tx.description || "";
    txCategory.value = tx.category || "";
    txNote.value = tx.note || "";
  }
}

function closeModal() {
  overlay.classList.add("hidden");
  overlay.setAttribute("aria-hidden", "true");
}

// ---- CRUD ----
function addTransaction(obj) {
  transactions.unshift(obj);
  saveTransactions(transactions);
  render();
}

function updateTransaction(id, patch) {
  const idx = transactions.findIndex(x => x.id === id);
  if (idx === -1) return;
  transactions[idx] = { ...transactions[idx], ...patch };
  saveTransactions(transactions);
  render();
}

function deleteTransaction(id) {
  transactions = transactions.filter(x => x.id !== id);
  saveTransactions(transactions);
  render();
}

// ---- Events ----
btnAdd.addEventListener("click", () => openModal("add"));

btnReset.addEventListener("click", () => {
  const ok = confirm("Reset all local transactions? (This clears saved browser data)");
  if (!ok) return;
  localStorage.removeItem(STORE_KEY);
  transactions = loadTransactions();
  render();
});

btnClose.addEventListener("click", closeModal);
btnCancel.addEventListener("click", closeModal);

overlay.addEventListener("click", (e) => {
  if (e.target === overlay) closeModal();
});

form.addEventListener("submit", (e) => {
  e.preventDefault();

  const amount = Number(txAmount.value);
  if (!Number.isFinite(amount) || amount <= 0) {
    alert("Amount must be greater than 0");
    return;
  }

  const payload = {
    type: txType.value,
    amount: Math.round(amount * 100) / 100,
    currency: (txCurrency.value || "CAD").toUpperCase(),
    date: txDate.value || todayISO(),
    description: (txDesc.value || "").trim(),
    category: (txCategory.value || "").trim(),
    note: (txNote.value || "").trim(),
  };

  if (!payload.description) {
    alert("Description is required");
    return;
  }

  const existingId = txId.value;

  if (!existingId) {
    addTransaction({ id: uid(), ...payload });
  } else {
    updateTransaction(existingId, payload);
  }

  closeModal();
});

elList.addEventListener("click", (e) => {
  const btn = e.target.closest("button");
  if (!btn) return;

  const action = btn.getAttribute("data-action");
  const id = btn.getAttribute("data-id");
  const tx = transactions.find(x => x.id === id);
  if (!tx) return;

  if (action === "edit") {
    openModal("edit", tx);
  } else if (action === "delete") {
    const ok = confirm("Delete this transaction?");
    if (ok) deleteTransaction(id);
  }
});

filterType.addEventListener("change", render);
searchText.addEventListener("input", render);
sortBy.addEventListener("change", render);

// ---- Init ----
render();
