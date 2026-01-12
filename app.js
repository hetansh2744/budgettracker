/* FlowFund — Minimal Finance Dashboard (LocalStorage)
   - Add / Edit / Delete transactions
   - Filter + search
   - Auto compute income/expense/balance
   - Persists in browser (works on GitHub Pages)
*/

const STORAGE_KEY = "flowfund.transactions.v1";

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

function uid() {
  return Math.random().toString(16).slice(2) + Date.now().toString(16);
}

function todayISO() {
  const d = new Date();
  const yyyy = d.getFullYear();
  const mm = String(d.getMonth() + 1).padStart(2, "0");
  const dd = String(d.getDate()).padStart(2, "0");
  return `${yyyy}-${mm}-${dd}`;
}

function demoData() {
  return [
    { id: uid(), type: "INCOME", amount: 4000, date: "2026-01-10", category: "Salary", title: "Salary", note: "" },
    { id: uid(), type: "EXPENSE", amount: 1200, date: "2026-01-11", category: "Housing", title: "Rent", note: "" },
    { id: uid(), type: "EXPENSE", amount: 250, date: "2026-01-11", category: "Food", title: "Groceries", note: "" },
    { id: uid(), type: "INCOME", amount: 200, date: "2026-01-11", category: "Side Hustle", title: "Freelance", note: "" },
  ];
}

function loadTransactions() {
  const raw = localStorage.getItem(STORAGE_KEY);
  if (!raw) return demoData();
  try {
    const parsed = JSON.parse(raw);
    return Array.isArray(parsed) ? parsed : demoData();
  } catch {
    return demoData();
  }
}

function saveTransactions(list) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(list));
}

let transactions = loadTransactions();

function formatMoney(n) {
  const v = Number(n || 0);
  return `$${v.toFixed(2)}`;
}

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

  return list.filter(t => {
    const matchesType = (type === "ALL") || (t.type === type);
    const hay = `${t.title} ${t.category} ${t.note} ${t.date}`.toLowerCase();
    const matchesQ = !q || hay.includes(q);
    return matchesType && matchesQ;
  });
}

function escapeHtml(s) {
  return String(s ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

function renderCards() {
  const { income, expense, balance } = computeSummary(transactions);
  els.incomeAmount.textContent = formatMoney(income);
  els.expenseAmount.textContent = formatMoney(expense);
  els.balanceAmount.textContent = formatMoney(balance);
}

function renderList() {
  const filtered = applyFilters([...transactions].sort((a, b) => (b.date || "").localeCompare(a.date || "")));
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

function openModal(mode, tx) {
  els.modalBackdrop.classList.remove("hidden");
  els.modalBackdrop.setAttribute("aria-hidden", "false");

  els.editId.value = tx?.id || "";
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

function upsertTransaction(newTx) {
  const idx = transactions.findIndex(t => t.id === newTx.id);
  if (idx >= 0) transactions[idx] = newTx;
  else transactions.push(newTx);

  saveTransactions(transactions);
  renderAll();
}

function deleteTransaction(id) {
  transactions = transactions.filter(t => t.id !== id);
  saveTransactions(transactions);
  renderAll();
}

/* Events */
els.addBtn.addEventListener("click", () => openModal("add"));

els.resetBtn.addEventListener("click", () => {
  transactions = demoData();
  saveTransactions(transactions);
  renderAll();
});

els.closeModalBtn.addEventListener("click", closeModal);
els.cancelBtn.addEventListener("click", closeModal);

els.modalBackdrop.addEventListener("click", (e) => {
  if (e.target === els.modalBackdrop) closeModal();
});

document.addEventListener("keydown", (e) => {
  if (e.key === "Escape" && !els.modalBackdrop.classList.contains("hidden")) closeModal();
});

els.form.addEventListener("submit", (e) => {
  e.preventDefault();

  const id = els.editId.value || uid();
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

  upsertTransaction({ id, type, amount, date, category, title, note });
  closeModal();
});

els.txList.addEventListener("click", (e) => {
  const btn = e.target.closest("button");
  if (!btn) return;

  const action = btn.dataset.action;
  const id = btn.dataset.id;
  const tx = transactions.find(t => t.id === id);

  if (action === "edit" && tx) openModal("edit", tx);
  if (action === "delete") {
    if (confirm("Delete this transaction?")) deleteTransaction(id);
  }
});

els.typeFilter.addEventListener("change", renderList);
els.searchInput.addEventListener("input", renderList);

/* Init */
renderAll();
