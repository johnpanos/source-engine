import json
import os

with open('architecture/tool_migrations.json', 'r') as f:
    data = json.load(f)

for cohort in data.get('cohorts', []):
    cohort['wrapperSites'] = []

with open('architecture/tool_migrations.json', 'w') as f:
    json.dump(data, f, indent=2)
