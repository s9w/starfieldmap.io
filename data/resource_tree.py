import json

def get_between(line, left, right):
    left_pos = line.find(left)
    right_pos = line.rfind(right)
    return line[left_pos+1:right_pos]

# building a shallow tree
shallow_tree = {}
next_id = ""
next_rarities = []
with open("xdump_resources.txt", "r") as f:
    for line in f:
        line = line.rstrip('\n')
        line = line.lstrip(" ")
        if line.startswith("FormID: IRES - Resource"):
            if next_id != "":
                shallow_tree[next_id] = next_rarities
                next_id = ""
                next_rarities = []
            next_id = get_between(line, "\"", "\"")
        if line.startswith("CNAM - Next Rarity"):
            next_rarities.append(get_between(line, "\"", "\""))



# building a proper tree
tree = {}

def get_descendents(res):
    descendents = {}
    if res in shallow_tree:
        for child in shallow_tree[res]:
            descendents[child] = get_descendents(child)
    return descendents

for res, next_rarities in shallow_tree.items():
    tree[res] = {}
    for next_rarity in next_rarities:
        tree[res][next_rarity] = get_descendents(next_rarity)


# removing single-node trees
for key in list(tree.keys()):
    if len(tree[key]) == 0:
        del tree[key]


# printing
def indent_print(content, level):
    s = ' ' * (2*level)
    s += content
    print(s)

def process_tree(key, value, level=0):
    indent_print(key, level)
    for child_key, child_value in value.items():
        process_tree(child_key, child_value, level+1)

for key, value in tree.items():
    process_tree(key, value)