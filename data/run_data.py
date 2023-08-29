import json, zstd

with open('star_positions.json', 'r') as f:
    star_positions = json.load(f)
with open('stars.json', 'r') as f:
    stars = json.load(f)

for key in stars.keys():
    stars[key]["position"] = star_positions[stars[key]["position_index"]]
    del stars[key]["position_index"]

data = {"stars": stars}
print(data)

uncompressed_str = json.dumps(data, separators=(',', ':'))
compressed_str = zstd.ZSTD_compress(bytes(uncompressed_str, 'utf-8'))
with open("../web/data", mode="wb") as file:
    file.write(compressed_str)
