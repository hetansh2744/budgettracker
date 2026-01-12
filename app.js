const listEl = document.getElementById("transactionList");
const incomeEl = document.getElementById("incomeAmount");
const expenseEl = document.getElementById("expenseAmount");
const balanceEl = document.getElementById("balanceAmount");
const emptyEl = document.getElementById("emptyState");

const modal = document.getElementById("modal");
const addBtn = document.getElementById("addBtn");
const cancelBtn = document.getElementById("cancelBtn");
const form = document.getElementById("txForm");

let transactions = JSON.parse(localStorage.getItem("flowfund-data")) || [
  { id: 1, type: "INCOME", title: "Salary", amount: 4000, date: "2026-01-01" },
  { id: 2, type: "EXPENSE", title: "Rent", amount: 1200, date: "2026-01-02" }
];

function save() {
  localStorage.setItem("flowfund-data", JSON.stringify(transactions));
}

function render() {
  listEl.innerHTML = "";
  let income = 0, expense = 0;

  if (transactions.length === 0) emptyEl.style.display = "block";
  else emptyEl.style.display = "none";

  transactions.forEach(tx => {
    if (tx.type === "INCOME") income += tx.amount;
    else expense += tx.amount;

    const li = document.createElement("li");
    li.innerHTML = `
      <span>${tx.title} • ${tx.date}</span>
      <span>${tx.type === "INCOME" ? "+" : "-"}$${tx.amount}</span>
      <div class="tx-actions">
        <button onclick="removeTx(${tx.id})">✕</button>
      </div>
    `;
    listEl.appendChild(li);
  });

  incomeEl.textContent = `$${income}`;
  expenseEl.textContent = `$${expense}`;
  balanceEl.textContent = `$${income - expense}`;
}

function removeTx(id) {
  transactions = transactions.filter(t => t.id !== id);
  save();
  render();
}

addBtn.onclick = () => modal.classList.remove("hidden");
cancelBtn.onclick = () => modal.classList.add("hidden");

form.onsubmit = e => {
  e.preventDefault();
  const tx = {
    id: Date.now(),
    type: type.value,
    title: title.value,
    amount: parseFloat(amount.value),
    date: date.value
  };
  transactions.push(tx);
  save();
  render();
  modal.classList.add("hidden");
  form.reset();
};

render();
