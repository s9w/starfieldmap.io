import json, zstd
from pathlib import Path

with open('star_positions.json', 'r') as f:
    star_positions = json.load(f)
with open('stars.json', 'r') as f:
    stars = json.load(f)

# Track positions without information
unused_star_positions = set()
for i in range(len(star_positions)):
    unused_star_positions.add(i)

for key in stars.keys():
    stars[key]["position"] = star_positions[stars[key]["position_index"]]
    unused_star_positions.remove(stars[key]["position_index"])
    del stars[key]["position_index"]

# Add stars without information
for position_index in unused_star_positions:
    stars[f"unknown_{position_index}"] = {"position": star_positions[position_index]}

# Now the systems
systems = {}
postfix = "_positions.json"
pathlist = Path("planet_and_moon_data").glob(f'*{postfix}')
for path in pathlist:
    filename = path.name
    system_name = filename[:-len(postfix)]
    with open(f'planet_and_moon_data/{system_name}_positions.json', 'r') as f:
        planet_positions = json.load(f)
    with open(f'planet_and_moon_data/{system_name}.json', 'r') as f:
        planet_data = json.load(f)
    system_data = {}
    for planet_name, planet_data in planet_data.items():
        system_data[planet_name] = planet_data
        system_data[planet_name]["position"] = planet_positions[system_data[planet_name]["position_index"]]
        del system_data[planet_name]["position_index"]
    systems[system_name] = system_data


data = {"stars": stars, "systems": systems}
print(data)

uncompressed_str = json.dumps(data, separators=(',', ':'))
compressed_str = zstd.ZSTD_compress(bytes(uncompressed_str, 'utf-8'))
with open("../web/data", mode="wb") as file:
    file.write(compressed_str)
