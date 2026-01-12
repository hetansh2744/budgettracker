// Mock data for now (replace with API calls)
const data = {
  income: 4200,
  expenses: 2750,
  transactions: [
    { desc: "Salary", amount: 4000, type: "INCOME" },
    { desc: "Rent", amount: -1200, type: "EXPENSE" },
    { desc: "Groceries", amount: -250, type: "EXPENSE" },
    { desc: "Freelance", amount: 200, type: "INCOME" }
  ]
};

// Update cards
document.getElementById("incomeAmount").textContent = `$${data.income}`;
document.getElementById("expenseAmount").textContent = `$${data.expenses}`;
document.getElementById("balanceAmount").textContent =
  `$${data.income - data.expenses}`;

// Render transactions
const list = document.getElementById("transactionList");

data.transactions.forEach(t => {
  const li = document.createElement("li");
  li.innerHTML = `
    <span>${t.desc}</span>
    <span style="color:${t.amount >= 0 ? '#22c55e' : '#ef4444'}">
      ${t.amount >= 0 ? '+' : ''}$${Math.abs(t.amount)}
    </span>
  `;
  list.appendChild(li);
});
