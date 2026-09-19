import subprocess
import os
import sys

# Try importing SCons env if available
try:
    Import("env")
    project_dir = env.get("PROJECT_DIR")
except Exception:
    project_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

if not project_dir:
    project_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

print("Executing pre-build hooks: Generating xgimi_keymap.h & web_assets.h in " + project_dir)

# 1. Run generate_keymap.py
kl_path = os.path.join(project_dir, "Vendor_000d_Product_3838.kl")
header_path = os.path.join(project_dir, "include", "xgimi_keymap.h")
gen_keymap_script = os.path.join(project_dir, "scripts", "generate_keymap.py")
subprocess.run([sys.executable, gen_keymap_script, kl_path, header_path], check=True)

# 2. Run bundle_web.py
bundle_script = os.path.join(project_dir, "scripts", "bundle_web.py")
subprocess.run([sys.executable, bundle_script], check=True)
