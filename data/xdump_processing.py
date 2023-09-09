import json, paramiko, zstd

starfield_dir = "S:\SteamLibrary\steamapps\common\Starfield/"

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
    with open(starfield_dir+"out_resources.txt", "r") as f:
        for line in f.readlines():
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
resources = parse_resources()
print("resources", list(resources.items())[0])
# print(resources)

def parse_resource_generators():
    resource_generators = {}
    next_id = ""
    next_rgen = {}
    def finisher(line):
        nonlocal next_id
        nonlocal next_rgen
        if len(next_rgen) > 0:
            resource_generators[next_id] = next_rgen
            next_id = ""
            next_rgen = {}
        
    with open(starfield_dir+"out_resource_generation.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("FormID: RSGD - Resource Generation Data"):
                finisher(line)
                next_id = get_between(line, "[", "]")
            if "RNAM - Resource: IRES - Resource" in line:
                next_rgen["res_formid"] = get_between(line, "[", "]")
        finisher(line)
            
    return resource_generators
resource_generators = parse_resource_generators()
print("res generators", list(resource_generators.items())[0])

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
    with open(starfield_dir+"out_biomes.txt", "r") as f:
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
biomes = parse_biomes()
biome_names = {key:value["name"] for (key, value) in biomes.items()}
print("biomes", list(biomes.items())[0])

stars = {}
next_star = {"planets": {}}

with open(starfield_dir+"out_stars.txt", "r") as f:
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
                stars[next_key] = next_star
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
        

# print(json.dumps(systems, indent=2))

new_planet = {"moons": [], "biomes": []}
next_biome_id = None
next_biome_percentage = None
with open(starfield_dir+"out_planets.txt", "r") as f:
    for line in f.readlines():
        line = line.rstrip('\n')
        line = line.lstrip(" ")
        level = get_level(line)
        if line.startswith("EDID"):
            if "body_type" in new_planet and new_planet["body_type"] == "Planet":
                stars[new_planet["star_id"]]["planets"][new_planet["planet_id"]] = new_planet
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
with open(starfield_dir+"out_planets.txt", "r") as f:
    for line in f.readlines():
        line = line.rstrip('\n')
        line = line.lstrip(" ")
        if line.startswith("EDID"):
            if "body_type" in new_moon and new_moon["body_type"] == "Moon":
                stars[new_moon["star_id"]]["planets"][new_moon["primary_planet_id"]]["moons"].append(new_moon)
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
for key in list(stars.keys()):
    value = stars[key]
    new_key = value["name"]
    del value["name"]
    stars[new_key] = value
    del stars[key]
for star in stars.values():
    moon_count = 0
    planet_count = 0
    for planet in star["planets"].values():
        planet_count += 1
        for moon in planet["moons"]:
            moon_count += 1
    star["planet_count"] = planet_count
    star["moon_count"] = moon_count


def add_to_star_resources(planet_or_star, star):
    planet_or_star["resources"] = []
    for planet_biome in planet_or_star["biomes"]:
        biome = biomes[planet_biome["biome_id"]]
        if "resgen_formid" in biome:
            resgen = resource_generators[biome["resgen_formid"]]
            res = resources[resgen["res_formid"]]
            if res["name"] not in star["resources"]:
                star["resources"].append(res["name"])
            if res["name"] not in planet_or_star["resources"]:
                planet_or_star["resources"].append(res["name"])

# adding resources
for star in stars.values():
    star["resources"] = []
    for planet in star["planets"].values():
        add_to_star_resources(planet, star)
        for moon in planet["moons"]:
            add_to_star_resources(planet, star)

# print(json.dumps(systems, indent=2))

everything = {"stars": stars, "biome_names": biome_names}
with open("everything.json", "w") as out_f:
    json.dump(everything, out_f, indent=2)

# compression
uncompressed_str = json.dumps(everything, separators=(',', ':'))
compressed_str = zstd.ZSTD_compress(bytes(uncompressed_str, 'utf-8'))
with open("data", mode="wb") as file:
    file.write(compressed_str)

# Credential load
with open('../local_stuff/secrets.json', mode='r', encoding='UTF-8') as file:
    secrets = json.load(file)

# Upload compressed json payload to ftp
print("uploading data...", end="", flush=True)
with paramiko.SSHClient() as ssh_client:
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh_client.connect(hostname=secrets["ftp_hostname"], username=secrets["ftp_username"], password=secrets["ftp_password"])
    with ssh_client.open_sftp() as ftp:
        files = ftp.put("data", "data")
print("done")