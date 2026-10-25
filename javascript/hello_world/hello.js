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

function TableData()
{
    this.data = [];
}
TableData.prototype.add = function(id, key, value) {
    this.data.push({id: id, key: key, value: value});
    console.log(`add [${id}, ${key}, ${value}]`);
}

function KvTableData()
{
    //TableData.call(this);
    //TableData.prototype.constructor.call(this);
    Object.getPrototypeOf(Object.getPrototypeOf(this)).constructor.call(this);
    console.log("KvTableData created");
}
Object.setPrototypeOf(KvTableData.prototype, TableData.prototype);

const kv_table_data = new KvTableData();
kv_table_data.add(1, "abc", 123),
kv_table_data.add(2, "bcd", 234),
kv_table_data.add(3, "cde", 345),
kv_table_data.add(4, "def", 456)
kv_table_data.add(5, "efg", 567)
const kv_table_data2 = new KvTableData();
kv_table_data2.add(0, "", 0);
const kv_table = document.querySelector("#kv_table tbody");
for (var dr of kv_table_data.data) {
    tr = document.createElement("tr");
    kv_table.appendChild(tr);
    for (var k of ["id", "key", "value"]) {
        td = document.createElement("td");
        tr.appendChild(td);
        td.appendChild(document.createTextNode(dr[k]));
    }
}
