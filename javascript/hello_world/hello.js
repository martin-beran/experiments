const button_alert = document.querySelector("#button_alert");
button_alert.addEventListener("click",() => {
    alert("Alert button pressed");
});

const button_var = document.querySelector("#button_var");
button_var.addEventListener("click", () => {
    const str = prompt('Button label');
    if (str != null && str !== "")
        button_var.textContent = str;
});

const button_enable = document.querySelector("#button_enable");
button_enable.checked = true;
button_enable.addEventListener("change", () => {
    button_alert.disabled = !button_enable.checked;
});

const menus = document.querySelectorAll("#menu_bar > div > div");
document.addEventListener("click", () => {
    menus.forEach((m)=> m.classList.remove("shown"));
});
menus.forEach((m) => {
    m.previousElementSibling.addEventListener("click", (e) => {
        menus.forEach((o) => {
            if (o !== m)
                o.classList.remove("shown");
        });
        m.classList.toggle("shown");
        e.stopPropagation();
    });
    for (let i of m.children)
        i.addEventListener("click", () => {
            alert(`Menu item ${i.textContent} selected`)
        });
});
