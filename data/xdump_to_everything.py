import json, paramiko, zstd

def get_level(line):
    level = 0
    while line[0] == " ":
        level += 1
        line = line[1:]
    return level // 2


def extract(line, prefix, name, target, t):
    if line.startswith(prefix):
        if line == prefix:
            result = None
        else:
            result = t(line.replace(prefix+": ", ""))
        target[name] = result

def get_between(line, left, right):
    left_pos = line.find(left)
    right_pos = line.rfind(right)
    return line[left_pos+1:right_pos]

def parse_resources():
    resources = {}
    next_resource = {}
    next_id = ""
    for line in open("xdump_resources.txt", "r"):
        # for line in f.readlines()[1:4]:

            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("FormID: IRES - Resource"):
                if len(next_resource) > 0:
                    resources[next_id] = next_resource
                    next_id = ""
                    next_resource = {}
                next_id = get_between(line, "[", "]")
            extract(line, "FULL - Name", "name", next_resource, str)
            if "ResourceTypeCrafting" in line:
                next_resource["crafting_text"] = get_between(line, "\"", "\"")
    return resources


def parse_resource_generators():
    resource_generators = {}
    next_id = None
    next_rgen = {"res_formids": []}
    def finisher(line):
        nonlocal next_id
        nonlocal next_rgen
        if next_id:
            resource_generators[next_id] = next_rgen
            next_id = None
            next_rgen = {"res_formids": []}
        
    with open("xdump_resource_generation.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("FormID: RSGD - Resource Generation Data"):
                finisher(line)
                next_id = get_between(line, "[", "]")
            if "RNAM - Resource: IRES - Resource" in line:
                next_rgen["res_formids"].append(get_between(line, "[", "]"))
        finisher(line)
            
    return resource_generators



def parse_biomes():
    biomes = {}
    next_id = ""
    next_biome = {}
    def finisher(line):
        nonlocal next_id
        nonlocal next_biome
        if len(next_biome) > 0:
            biomes[next_id] = next_biome
            next_id = ""
            next_biome = {}
    with open("xdump_biomes.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("FormID: BIOM - Biome"):
                finisher(line)
                next_id = get_between(line, "[", "]")
            extract(line, "FULL - Name", "name", next_biome, str)
            if "RSGD - Resource Generation Data" in line:
                next_biome["resgen_formid"] = get_between(line, "[", "]")
        finisher(line)
    return biomes





def run():
    resources_by_id = parse_resources()
    print("resources_by_id", list(resources_by_id.items())[0])

    res_generators_by_id = parse_resource_generators()
    print("res_generators_by_id", list(res_generators_by_id.items())[0])

    biomes_by_id = parse_biomes()
    biome_names_by_id = {key:value["name"] for (key, value) in biomes_by_id.items()}
    print("biomes_by_id", list(biomes_by_id.items())[0])

    stars_for_galaxy = {}
    

    # print(json.dumps(systems, indent=2))
    next_star = {"planets": {}}
    with open("xdump_stars.txt", "r") as f:
        for line in f.readlines():
            level = get_level(line)
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("EDID"):
                if "star_id" in next_star:
                    next_key = next_star["star_id"]
                    next_star["position"] = [-next_star["x"], -next_star["y"], next_star["z"]]
                    next_star["level"] = None
                    next_star["traits"] = None
                    for key in ["x", "y", "z", "star_id"]:
                        del next_star[key]
                    stars_for_galaxy[next_key] = next_star
                next_star = {"planets": {}}
            extract(line, "FULL - Name", "name", next_star, str)
            extract(line, "DNAM - Star ID", "star_id", next_star, int)
            extract(line, "Spectral class", "spectral_class", next_star, str)
            extract(line, "Catalogue ID", "catalogue_id", next_star, str)
            extract(line, "Radius (in km)", "radius", next_star, float)
            extract(line, "Magnitude", "magnitude", next_star, float)
            extract(line, "Temperature in K", "temperature", next_star, int)
            extract(line, "Mass (in SM)", "mass", next_star, float)
            extract(line, "x", "x", next_star, float)
            extract(line, "y", "y", next_star, float)
            extract(line, "z", "z", next_star, float)

    new_planet = {"moons": [], "biomes": []}
    next_biome_id = None
    next_biome_percentage = None
    with open("xdump_planets.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            level = get_level(line)
            if line.startswith("EDID"):
                if "body_type" in new_planet and new_planet["body_type"] == "Planet":
                    stars_for_galaxy[new_planet["star_id"]]["planets"][new_planet["planet_id"]] = new_planet
                new_planet = {"moons": [], "biomes": []}
            
            extract(line, "FULL - Name", "name", new_planet, str)
            extract(line, "CNAM - Body type", "body_type", new_planet, str)
            extract(line, "GNAM - Visual size", "visual_size", new_planet, float)
            extract(line, "Radius in km", "radius", new_planet, float)
            extract(line, "Mass (in Earth Masses)", "mass", new_planet, float)
            extract(line, "DENS - Density", "density", new_planet, float)
            extract(line, "Gravity", "gravity", new_planet, float)
            extract(line, "Star ID", "star_id", new_planet, int)
            extract(line, "Planet ID", "planet_id", new_planet, int)
            extract(line, "xxx", "xxx", new_planet, float)
            extract(line, "GNAM - mystery", "gnam_mystery", new_planet, float)
            extract(line, "x1", "hnam_float", new_planet, float)
            if line.startswith("Unknown: BIOM - Biome"):
                next_biome_id = get_between(line, "[", "]")
            if line.startswith("Percentage: "):
                next_biome_percentage = float(line.replace("Percentage: ", ""))
            if level != 2 and next_biome_id != None and next_biome_percentage != None:
                new_planet["biomes"].append({"percentage": next_biome_percentage, "biome_id": next_biome_id})
                next_biome_id = None
                next_biome_percentage = None

    # print(json.dumps(systems, indent=2))

    new_moon = {"biomes": []}
    with open("xdump_planets.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("EDID"):
                if "body_type" in new_moon and new_moon["body_type"] == "Moon":
                    stars_for_galaxy[new_moon["star_id"]]["planets"][new_moon["primary_planet_id"]]["moons"].append(new_moon)
                new_moon = {"biomes": []}
            
            extract(line, "FULL - Name", "name", new_moon, str)
            extract(line, "CNAM - Body type", "body_type", new_moon, str)
            extract(line, "GNAM - Visual size", "visual_size", new_moon, float)
            extract(line, "Radius in km", "radius", new_moon, float)
            extract(line, "Mass (in Earth Masses)", "mass", new_moon, float)
            extract(line, "DENS - Density", "density", new_moon, float)
            extract(line, "Gravity", "gravity", new_moon, float)
            extract(line, "Star ID", "star_id", new_moon, int)
            extract(line, "Primary planet ID", "primary_planet_id", new_moon, int)
            extract(line, "xxx", "xxx", new_moon, float)
            extract(line, "GNAM - mystery", "gnam_mystery", new_moon, float)
            extract(line, "x1", "hnam_float", new_moon, float)
            if line.startswith("Unknown: BIOM - Biome"):
                next_biome_id = get_between(line, "[", "]")
            if line.startswith("Percentage: "):
                next_biome_percentage = float(line.replace("Percentage: ", ""))
            if level != 2 and next_biome_id != None and next_biome_percentage != None:
                new_moon["biomes"].append({"percentage": next_biome_percentage, "biome_id": next_biome_id})
                next_biome_id = None
                next_biome_percentage = None

    # Corrections after I don't need that silly layout anymore
    for key in list(stars_for_galaxy.keys()):
        value = stars_for_galaxy[key]
        new_key = value["name"]
        del value["name"]
        stars_for_galaxy[new_key] = value
        del stars_for_galaxy[key]

    # moon and planet count
    for star in stars_for_galaxy.values():
        moon_count = 0
        planet_count = 0
        for planet in star["planets"].values():
            planet_count += 1
            for moon in planet["moons"]:
                moon_count += 1
        star["planet_count"] = planet_count
        star["moon_count"] = moon_count

    # res_by_biome_name
    res_by_biome_name = {}
    for biome in biomes_by_id.values():
        if "resgen_formid" in biome:
            resgen_formid = biome["resgen_formid"]
            resgen = res_generators_by_id[resgen_formid]
            res_by_biome_name[biome["name"]] = [resources_by_id[x]["name"] for x in resgen["res_formids"]]
    
    single_biome_bodies = []
    for star_name, star in stars_for_galaxy.items():
        for planet_name, planet in star["planets"].items():
            if len(planet["biomes"]) == 1:
                single_biome_bodies.append(planet)
            for moon in planet["moons"]:
                if len(moon["biomes"]) == 1:
                    single_biome_bodies.append(moon)
    single_biome_bodies = sorted(single_biome_bodies, key=lambda x: x['biomes'][0]["biome_id"])

    everything = {
        "stars": stars_for_galaxy,
        "biome_names_by_id": biome_names_by_id,
        "res_by_biome_name": res_by_biome_name,
        "res_generators_by_id": res_generators_by_id,
        "resources_by_id": resources_by_id,
        "biomes_by_id": biomes_by_id,
        "stars_for_galaxy": stars_for_galaxy
    }
    with open("everything.json", "w") as out_f:
        json.dump(everything, out_f, indent=2)

run()