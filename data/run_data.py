import json, zstd

with open('system_positions.json', 'r') as f:
    system_positions = json.load(f)
with open('systems.json', 'r') as f:
    systems = json.load(f)

for key in systems.keys():
    systems[key]["position"] = system_positions[systems[key]["position_index"]]
    del systems[key]["position_index"]

print(systems)

uncompressed_str = json.dumps(systems, separators=(',', ':'))
compressed_str = zstd.ZSTD_compress(bytes(uncompressed_str, 'utf-8'))
with open("../web/data", mode="wb") as file:
    file.write(compressed_str)
