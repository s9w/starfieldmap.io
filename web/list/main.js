function notify(event) {
    let search_term = event.target.value.toLowerCase();
    let els = document.getElementById("bodies").children;
    let result_count = 0;
    for (let i = 0; i < els.length; i++) {
        let condition = els[i].dataset.lowercase_name.includes(search_term);
        if(condition)
        {
            els[i].classList.remove("hidden");
            result_count++;
        }
        else
            els[i].classList.add("hidden");
    }
    document.getElementById("search_result").textContent = `${result_count} results`;
}

function main()
{
    document.querySelector("#search").addEventListener("input", notify, false);
}

window.addEventListener("load", (event) => {
    main();
  });