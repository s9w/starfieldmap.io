import numpy as np
import json, paramiko, zstd
from pydoc import locate

space_extraction_path = "S:\\bae_Extracts\\misc\\space\\"

def get_stars_data(name, row, type="str"):
    t = locate(type)
    index = np.where(stars_columns == name)[0][0]
    return t(row[index])
def get_galaxy_data(name, row, type="str"):
    t = locate(type)
    index = np.where(galaxy_columns == name)[0][0]
    return t(row[index])

stars_csv = np.loadtxt(space_extraction_path+"stars.csv", dtype=str, delimiter=",")
stars_columns = stars_csv[0]
stars_data = stars_csv[1:]

galaxy_csv = np.loadtxt(space_extraction_path+"galaxy.csv", dtype=str, delimiter=",")
galaxy_columns = galaxy_csv[0]
galaxy_data = galaxy_csv[1:]
# "Body Type" 5 is asteroid belt, 2 is planet, 3 is moon. These are the only ones
# it has a "Primary" if it's a moon, that's its planet_index
# planet_index starts at 1, goes through the galaxy.csv
# "Primary" is 0 exactly once for a moon, probably a bug. But in a moon that has no planet
# warning: "Settled" and "Star ID" aren't always set for moons, but either one always is
# TODO: I can'T see the asteroid belt in Alpha CEntauri for example


sun_radius = 696340 # TODO not exactly right, they seem to use a slightly different number

everything = {}
star_id_to_name = {}
for row in stars_data:
    name = get_stars_data("proper", row)
    if name != "":
        star_id_to_name[get_stars_data("id", row, "int")] = name
        everything[name] = {
            "position": [-get_stars_data("x", row, "float"), get_stars_data("y", row, "float"), -get_stars_data("z", row, "float")],

            "level": "TODO",
            "spectral_class": get_stars_data("spect", row),
            "catalogue_id": get_stars_data("gl", row),
            "temperature": get_stars_data("Temp", row),
            "mass": get_stars_data("mass", row),
            "radius": int(sun_radius * get_stars_data("radius", row, "float")),
            "magnitude": get_stars_data("absmag", row),
            "planets": {},
            "traits": "TODO",
            "resources": "TODO"
            }

def get_star_name_from_galaxy_row(row):
    if get_galaxy_data("Body Type", row) not in ["2", "3"]:
        print("only for planets and moons")
        exit(0)
    star_id = get_galaxy_data("Star ID", row, "int")
    if star_id == 0:
        return get_galaxy_data("Settled", row)
    else:
        return star_id_to_name[star_id]

magnetosphere_lookup = {
    "Very Strong" : "powerful",
    "Weak": "strong" # TODO: this seems wrong, bad lookup?
}

def get_planet_moon_data(row):
    return {
            "type": get_galaxy_data("Type", row),
            "gravity": get_galaxy_data("Gravity", row, "float"),
            "temperature": "TODO", # probably f("Heat")
            "atmosphere": "TODO", # probably f"Atmos. Handle"
            "magnetosphere": magnetosphere_lookup.get(get_galaxy_data("Mag. Field", row), "TODO"),
            "fauna": "TODO",
            "flora": "TODO",
            "water": "TODO", # probably f("Hydro %")

            "moons": {}
        }

# planets
planet_indices = {}
for row in galaxy_data:
    if get_galaxy_data("Body Type", row) == "2":
        star_name = get_star_name_from_galaxy_row(row)
        planet = get_planet_moon_data(row)
        planet["moons"] = {}
        planet_name = get_galaxy_data("Name", row)
        planet_index = len(everything[star_name]["planets"])
        planet_indices[planet_name] = 1 + planet_index
        everything[star_name]["planets"][planet_name] = planet

# moons
for row in galaxy_data:
    if get_galaxy_data("Body Type", row) == "3":
        star_name = get_star_name_from_galaxy_row(row)
        moon_name = get_galaxy_data("Name", row)
        primary = get_galaxy_data("Primary", row, "int")
        moon = get_planet_moon_data(row)
        for planet_name in everything[star_name]["planets"].keys():
            if planet_indices[planet_name] == primary:
                everything[star_name]["planets"][planet_name]["moons"][moon_name] = moon

# planet and moon count
for star_data in everything.values():
    star_data["planet_count"] = len(star_data["planets"])
    moon_count = 0
    for planet in star_data["planets"].values():
        moon_count += len(planet["moons"])
    star_data["moon_count"] = moon_count

# with open("everything.json", "w") as out_f:
#     json.dump(everything, out_f, indent=2)

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
