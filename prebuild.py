Import("env")
import json

with open('./web/package.json', 'r') as pkg:
    data = json.load(pkg)

env.Append(CPPDEFINES=[
    ("VERSION_WEBUI", data['version'])
])

print("UI VERSION: ", data['version'])
