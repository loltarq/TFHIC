
#!/usr/bin/env python3
import json, argparse
parser = argparse.ArgumentParser(description="Make a thermal yields JSON template")
parser.add_argument("--bins", type=int, default=10)
parser.add_argument("--pdg", type=str, default="211,321,2212", help="comma-separated PDG list")
parser.add_argument("--out", type=str, default="thermal_yields/out/example_yields.json")
args = parser.parse_args()

pdgs=[int(x) for x in args.pdg.split(",")]

data={"meta":{"note":"Fill values with Thermal-FIST dN/dy; 'primary' definition assumed"},
      "bins":[]}
for i in range(args.bins):
    binobj={"bin_index":i, "yields_dNdy_primary":{str(p):0.0 for p in pdgs}}
    data["bins"].append(binobj)

with open(args.out,"w") as f:
    json.dump(data,f,indent=2)
print("Wrote", args.out)
