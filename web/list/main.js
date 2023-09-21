function expand_all()
{
    let els = document.getElementById("bodies").children;
    for (let i = 0; i < els.length; i++)
        els[i].open = true;
}


function on_search_term_change()
{
    let search_term = document.querySelector("#search").value;
    let els = document.getElementById("bodies").children;
    let result_count = 0;
    for (let i = 0; i < els.length; i++) {
        els[i].open = false;
        let condition = els[i].dataset.lowercase_name.includes(search_term);
        if(condition)
        {
            els[i].classList.remove("hidden");
            result_count++;
        }
        else
            els[i].classList.add("hidden");
    }

    const expand_threshold = 30;
    if(result_count < expand_threshold)
    {
        for (let i = 0; i < els.length; i++) {
            if(els[i].classList.contains("hidden") == false)
            {
                els[i].open = true;
            }
        }
    }

    document.getElementById("search_result").textContent = `${result_count} results`;
}


function get_new_elem(type, content=null, classlist=null)
{
    let result = document.createElement(type);
    if(content != null)
        result.textContent = content;
    if(classlist != null)
        result.classList = classlist;
    return result;
}


function build_site(bodies)
{
    bodies.forEach(body => {
        let details_elem = get_new_elem("details");
        details_elem.id = body.name;
        details_elem.dataset.lowercase_name = body.name.toLowerCase();
        details_elem.dataset.type = body.class;
        details_elem.classList.add(body.class);

        let summary_elem = get_new_elem("summary");
        body.nav.forEach((nav_item, i, array) => {
            if(i == array.length-1)
                summary_elem.appendChild(get_new_elem("div", nav_item, ["last"]));
            else
                summary_elem.appendChild(get_new_elem("div", nav_item));
        });
        summary_elem.appendChild(get_new_elem("div", body.formid, ["debug"]));
        details_elem.appendChild(summary_elem);

        let quickfacts_el = get_new_elem("div", "", ["quickfacts"]);
        let temp_el = get_new_elem("span", `${body.temperature}°C`, ["temperature"])
        temp_el.dataset.temp_lvl = body.temp_level
        quickfacts_el.appendChild(temp_el);
        quickfacts_el.appendChild(get_new_elem("span", `; ${body.gravity}g; ${body.oxygen_amount}% O₂`));
        details_elem.appendChild(quickfacts_el);

        details_elem.appendChild(get_new_elem("div", `${body.fauna_count} Fauna`, ["fauna_count"]));

        let floras_elem = get_new_elem("div", null, ["floras"]);
        if(body.flora.length == 0)
            floras_elem.appendChild(get_new_elem("div", "No Floras"));
        else
        {
            floras_elem.appendChild(get_new_elem("div", "Floras:"));
            body.flora.forEach((flora, i_flora, array) => {
                let flora_elem = get_new_elem("div", null, ["flora"]);
                flora_elem.appendChild(get_new_elem("div", `${i_flora+1}. ${flora.name} (${flora.formid})`, ["flora_name"]));
                floras_elem.appendChild(flora_elem);
            });
        }
        details_elem.appendChild(floras_elem);


        let biomes_elem = get_new_elem("div", null, ["biomes"]);
        if(body.biomes.length == 0)
            biomes_elem.appendChild(get_new_elem("div", "No Biomes"));
        else
        {
            biomes_elem.appendChild(get_new_elem("div", "Biomes:"));
            body.biomes.forEach(biome => {
                let biome_elem = get_new_elem("div", null, ["biome"]);
                biome_elem.appendChild(get_new_elem("div", `${biome.percentage}% ${biome.name} (${biome.formid})`, ["biome_name"]));
                biomes_elem.appendChild(biome_elem);
            });
        }
        details_elem.appendChild(biomes_elem);


        let traits_elem = get_new_elem("div", null, ["traits"]);
        if(body.traits.length == 0)
            traits_elem.appendChild(get_new_elem("div", "No Traits"));
        else
        {
            traits_elem.appendChild(get_new_elem("div", "Traits:"));
            body.traits.forEach(trait => {
                let trait_elem = get_new_elem("div", null, ["trait"]);
                let img_elem = get_new_elem("img", null, ["trait_img"]);
                img_elem.src = `traits/${trait}.png`;
                trait_elem.appendChild(img_elem);
                trait_elem.appendChild(get_new_elem("div", `${trait}`));
                traits_elem.appendChild(trait_elem);
            });
        }
        details_elem.appendChild(traits_elem);


        document.getElementById("bodies").appendChild(details_elem);
    });
}

function get_loc()
{
    let loc = decodeURI(new URL(document.URL).hash);
    if(loc.startsWith("#"))
        loc = loc.slice(1);
    return loc;
}


async function main()
{
    const compressedBuf = await fetch('binary_payload', {cache: "no-store"}).then(
        res => res.arrayBuffer()
    );
    const compressed = new Uint8Array(compressedBuf);
    var string = new TextDecoder().decode(fzstd.decompress(compressed));
    build_site(JSON.parse(string)["bodies"]);

    document.querySelector("#search").addEventListener("input", on_search_term_change, false);
    let loc = get_loc();
    if(loc)
    {
        document.querySelector("#search").value = loc;
        on_search_term_change();
    }
}


window.addEventListener("load", () => {
    main();
  });
