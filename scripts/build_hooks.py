import subprocess
import os

Import("env")

def pre_build_script(source, target, env):
    print("Executing pre-build hooks: Generating xgimi_keymap.h & web_assets.h...")
    project_dir = env.get("PROJECT_DIR")
    
    # 1. Run generate_keymap.py
    kl_path = os.path.join(project_dir, "Vendor_000d_Product_3838.kl")
    header_path = os.path.join(project_dir, "include", "xgimi_keymap.h")
    gen_keymap_script = os.path.join(project_dir, "scripts", "generate_keymap.py")
    subprocess.run(["py", gen_keymap_script, kl_path, header_path], check=True)

    # 2. Run bundle_web.py
    bundle_script = os.path.join(project_dir, "scripts", "bundle_web.py")
    subprocess.run(["py", bundle_script], check=True)

pre_build_script(None, None, env)
