import json

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

def parse_resources():
    resources = {}
    next_resource = {}
    next_id = ""
    with open(starfield_dir+"out_resources.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("FormID: IRES - Resource"):
                if True:
                    resources[next_id] = next_resource
                    next_id = ""
                    next_resource = {}
                left_pos = line.find("[")
                right_pos = line.find("]")
                next_id = line[left_pos+1:right_pos]
            extract(line, "FULL - Name", "name", next_resource, str)
            if "ResourceTypeCrafting" in line:
                left_pos = line.find("\"")
                right_pos = line.rfind("\"")
                next_resource["crafting_text"] = line[left_pos+1:right_pos]
    return resources
resources = parse_resources()
# print(resources)

def parse_resource_generators():
    resource_generators = {}
    next_id = ""
    next_rgen = {}
    with open(starfield_dir+"out_resource_generation.txt", "r") as f:
        for line in f.readlines():
            line = line.rstrip('\n')
            line = line.lstrip(" ")
            if line.startswith("FormID: RSGD - Resource Generation Data"):
                if True:
                    resource_generators[next_id] = next_rgen
                    next_id = ""
                    next_rgen = {}
                left_pos = line.find("[")
                right_pos = line.find("]")
                next_id = line[left_pos+1:right_pos]
            if "RNAM - Resource: IRES - Resource" in line:
                left_pos = line.find("[")
                right_pos = line.find("]")
                next_rgen["res_formid"] = line[left_pos+1:right_pos]
                # next_rgen["embed"] = resources[line[left_pos+1:right_pos]]
    return resource_generators
resource_generators = parse_resource_generators()
print(resource_generators)
exit()

systems = {}
next_star = {"planets": {}}
with open(starfield_dir+"out_stars.txt", "r") as f:
    for line in f.readlines():
        level = get_level(line)
        line = line.rstrip('\n')
        line = line.lstrip(" ")
        if line.startswith("EDID"):
            if "star_id" in next_star:
                next_key = next_star["star_id"]
                next_star["position"] = [next_star["x"], next_star["y"], next_star["z"]]
                next_star["level"] = None
                next_star["traits"] = None
                for key in ["x", "y", "z", "star_id"]:
                    del next_star[key]
                systems[next_key] = next_star
            next_star = {"planets": {}}
        extract("FULL - Name", "name", next_star, str)
        extract("DNAM - Star ID", "star_id", next_star, int)
        extract("Spectral class", "spectral_class", next_star, str)
        extract("Catalogue ID", "catalogue_id", next_star, str)
        extract("Radius (in km)", "radius", next_star, float)
        extract("Magnitude", "magnitude", next_star, float)
        extract("Temperature in K", "temperature", next_star, int)
        extract("Mass (in SM)", "mass", next_star, float)
        extract("x", "x", next_star, float)
        extract("y", "y", next_star, float)
        extract("z", "z", next_star, float)

# print(json.dumps(systems, indent=2))

new_planet = {}
with open(starfield_dir+"out_planets.txt", "r") as f:
    for line in f.readlines():
        line = line.rstrip('\n')
        line = line.lstrip(" ")
        if line.startswith("EDID"):
            if len(new_planet) > 0 and new_planet["body_type"] == "Planet":
                systems[new_planet["star_id"]]["planets"][new_planet["planet_id"]] = new_planet
            new_planet = {"moons": []}
        
        extract("FULL - Name", "name", new_planet, str)
        extract("CNAM - Body type", "body_type", new_planet, str)
        extract("GNAM - Visual size", "visual_size", new_planet, float)
        extract("Radius in km", "radius", new_planet, float)
        extract("Mass (in Earth Masses)", "mass", new_planet, float)
        extract("DENS - Density", "density", new_planet, float)
        extract("Gravity", "gravity", new_planet, float)
        extract("Star ID", "star_id", new_planet, int)
        extract("Planet ID", "planet_id", new_planet, int)
        extract("xxx", "xxx", new_planet, float)
        extract("GNAM - mystery", "gnam_mystery", new_planet, float)
        extract("x1", "hnam_float", new_planet, float)

# print(json.dumps(systems, indent=2))

new_moon = {}
with open(starfield_dir+"out_planets.txt", "r") as f:
    for line in f.readlines():
        line = line.rstrip('\n')
        line = line.lstrip(" ")
        if line.startswith("EDID"):
            if len(new_moon) > 0 and new_moon["body_type"] == "Moon":
                systems[new_moon["star_id"]]["planets"][new_moon["primary_planet_id"]]["moons"].append(new_moon)
            new_moon = {}
        
        extract("FULL - Name", "name", new_moon, str)
        extract("CNAM - Body type", "body_type", new_moon, str)
        extract("GNAM - Visual size", "visual_size", new_moon, float)
        extract("Radius in km", "radius", new_moon, float)
        extract("Mass (in Earth Masses)", "mass", new_moon, float)
        extract("DENS - Density", "density", new_moon, float)
        extract("Gravity", "gravity", new_moon, float)
        extract("Star ID", "star_id", new_moon, int)
        extract("Primary planet ID", "primary_planet_id", new_moon, int)
        extract("xxx", "xxx", new_moon, float)
        extract("GNAM - mystery", "gnam_mystery", new_moon, float)
        extract("x1", "hnam_float", new_moon, float)


# for star_id, star in systems.items():
#     if star["name"] == "Sol":
#         # print(star)
#         for planet in star["planets"].values():
#             print(planet["name"], len(planet["moons"]))
# exit()

# list = sorted(stars.values(), key=lambda v: v.get("xxx", 0.0), reverse=True)
# print(list[:5])

# for star_id, system in systems.items():
#     print("system", system["name"])
#     for planet in system["planets"].values():
#         print(f"index: {planet['planet_id']}: {planet['xxx']}")

radius = []
mass = []
y = []
hnam_floats = []
planet_ids = []
colors = []
for star_id, system in systems.items():
    for planet in system["planets"].values():
        radius.append(planet["radius"])
        mass.append(planet["mass"])
        y.append(planet["gnam_mystery"])
        hnam_floats.append(planet["hnam_float"])
        planet_ids.append(planet["planet_id"])
        colors.append("blue")

        for moon in planet["moons"]:
            radius.append(moon["radius"])
            mass.append(moon["mass"])
            y.append(moon["gnam_mystery"])
            planet_ids.append(moon["primary_planet_id"])
            hnam_floats.append(moon["hnam_float"])
            colors.append("gray")

        # if value["body_type"] == "Planet":
        #     colors.append("blue")
        # else:
        #     colors.append("gray")
