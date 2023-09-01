import numpy as np
import json
from pydoc import locate

space_extraction_path = "S:\\bae_Extracts\\misc\\space\\"

def get_column_index(name, row, type="str"):
    t = locate(type)
    index = np.where(columns == name)[0][0]
    return t(row[index])

csv = np.loadtxt(space_extraction_path+"stars.csv", dtype=str, delimiter=",")
columns = csv[0]
data = csv[1:]

positions = []
star_data = {}
for row in data:
    name = get_column_index("proper", row)
    # if name == "Narion":
    #     print(get_column_index("gl", row))
    if name != "":
        pos_index = len(positions)
        positions.append([-get_column_index("x", row, "float"), get_column_index("y", row, "float"), -get_column_index("z", row, "float")])
        star_data[name] = {
            "position_index": pos_index,

            "level": "TODO",
            "spectral_class": get_column_index("spect", row),
            "catalogue_id": get_column_index("gl", row),
            "temperature": get_column_index("Temp", row),
            "mass": get_column_index("mass", row),
            "radius": "TODO",
            "magnitude": get_column_index("absmag", row),
            "planets": "TODO",
            "moons": "TODO"
            }

with open("star_positions.json", "w") as out_f:
    json.dump(positions, out_f, indent=2)
with open("stars.json", "w") as out_f:
    json.dump(star_data, out_f, indent=2)
