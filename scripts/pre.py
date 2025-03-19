Import("env")
import os
import json

root = os.getcwd()

with open(root + '/package.json', 'r') as data:
    pkg = json.load(data)

with open(root + '/configure.json', 'r') as data:
    conf = json.load(data)

defs = [
    ("VERSION_WEBUI", pkg['version']),
    ("VERSION_FIRMWARE", conf['version'])
]

for (k, v) in conf['common'].items():
    defs.append((k.upper(), v))

for (k, v) in conf[env.get('PIOENV')].items():
    defs.append((k.upper(), v))

env.Append(CPPDEFINES=defs)

print("\n•----------------------------•")
print("   FIRMWARE VERSION:", conf['version'])
print("   WEB UI VERSION:  ", pkg['version'])
print("•----------------------------•\n")