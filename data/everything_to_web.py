import json, paramiko, zstd    

everything = json.load(open("everything.json"))
res_generators_by_id = everything["res_generators_by_id"]
resources_by_id = everything["resources_by_id"]
biomes_by_id = everything["biomes_by_id"]
stars_for_galaxy = everything["stars_for_galaxy"]

def add_to_star_resources(planet_or_star, star):
    planet_or_star["resources"] = []
    for planet_biome in planet_or_star["biomes"]:
        biome = biomes_by_id[planet_biome["biome_id"]]
        if "resgen_formid" in biome:
            resgen = res_generators_by_id[biome["resgen_formid"]]
            for res in [resources_by_id[formid] for formid in resgen["res_formids"]]:
                if res["name"] not in star["resources"]:
                    star["resources"].append(res["name"])
                if res["name"] not in planet_or_star["resources"]:
                    planet_or_star["resources"].append(res["name"])

# adding resources
for star in stars_for_galaxy.values():
    star["resources"] = []
    for planet in star["planets"].values():
        add_to_star_resources(planet, star)
        for moon in planet["moons"]:
            add_to_star_resources(planet, star)

# add label shifts
with open("label_shifts.json", "r") as f:
    label_shifts = json.load(f)
for star_name, star in stars_for_galaxy.items():
    star["extra_classes"] = []
    if star_name in label_shifts:
        star["extra_classes"].append(label_shifts[star_name])

# compression
uncompressed_str = json.dumps(stars_for_galaxy, separators=(',', ':'))
compressed_str = zstd.ZSTD_compress(bytes(uncompressed_str, 'utf-8'))
with open("data", mode="wb") as file:
    file.write(compressed_str)

# # Credential load
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