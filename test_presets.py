import os
import sys
import json
import time
import requests
import subprocess
import soundfile as sf
import numpy as np

# Configuration
SERVER_URL = "http://localhost:18080/process"
PROJECT_DIR = "/home/filipe/projects/retro-vst/web-server-vst"
FRONTEND_DIR = "/home/filipe/projects/retro-vst/retro-vst"
INPUT_WAV = os.path.join(PROJECT_DIR, "resources/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav")
OUTPUT_DIR = os.path.join(PROJECT_DIR, "tmp")
PLUGIN_JSON_PATH = os.path.join(FRONTEND_DIR, "src/Plugin.json")

def start_server():
    # Check if server is already running
    try:
        r = requests.get("http://localhost:18080/", timeout=1)
        if r.status_code == 200:
            print("🚀 VST Server is already running.")
            return None
    except requests.exceptions.RequestException:
        pass

    print("🚀 Starting VST Server locally...")
    # Ensure Xvfb is running on :99
    try:
        subprocess.run("pgrep Xvfb || Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render &", shell=True)
        time.sleep(1)
    except Exception as e:
        print(f"Warning: Failed to ensure Xvfb: {e}")

    env = os.environ.copy()
    env["DISPLAY"] = ":99"
    server_bin = os.path.join(PROJECT_DIR, "build/bin/AudioProcessingProject")
    
    process = subprocess.Popen(
        [server_bin, PROJECT_DIR],
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # Wait for the server to be ready
    for _ in range(10):
        try:
            r = requests.get("http://localhost:18080/", timeout=1)
            if r.status_code == 200:
                print("✅ VST Server started and ready!")
                return process
        except requests.exceptions.RequestException:
            time.sleep(1)
            
    print("❌ Failed to start VST Server.")
    process.terminate()
    sys.exit(1)

def run_preset_test(plugin, process_server=None):
    plugin_name = plugin["name"]
    print(f"\n--- Testing default preset for: {plugin['label']} ({plugin_name}) ---")
    
    # Construct query parameters using default values from Plugin.json
    url_params = {
        "plugin": plugin_name,
        "preview": "true",
        "previewStartTime": "0"
    }
    
    for i, param in enumerate(plugin["parameters"]):
        val = param["defaultValue"]
        if param["type"] == "slider":
            # Normalize slider value
            min_val = param["min"]
            max_val = param["max"]
            normalized = (val - min_val) / (max_val - min_val)
            val_str = f"{normalized:.6f}"
        else:
            val_str = str(val)
        url_params[f"p{i}"] = val_str
        
    output_filename = f"preset_test_{plugin_name}.wav"
    output_path = os.path.join(OUTPUT_DIR, output_filename)
    
    try:
        with open(INPUT_WAV, "rb") as f:
            files = {"audio_file": (os.path.basename(INPUT_WAV), f, "audio/wav")}
            response = requests.post(SERVER_URL, params=url_params, files=files, timeout=15)
            
        if response.status_code != 200:
            print(f"❌ Server error {response.status_code}: {response.text}")
            return False
            
        with open(output_path, "wb") as f_out:
            f_out.write(response.content)
            
        # Analyze processed audio
        data_in, sr_in = sf.read(INPUT_WAV)
        data_out, sr_out = sf.read(output_path)
        
        # Verify stereo channels
        if data_out.ndim != 2 or data_out.shape[1] != 2:
            print(f"❌ Error: Output audio is not stereo. Shape: {data_out.shape}")
            return False
            
        # Compute RMS for Left and Right channels
        left_channel = data_out[:, 0]
        right_channel = data_out[:, 1]
        
        rms_left = np.sqrt(np.mean(left_channel**2))
        rms_right = np.sqrt(np.mean(right_channel**2))
        
        print(f"  Left Channel RMS:  {rms_left:.6f}")
        print(f"  Right Channel RMS: {rms_right:.6f}")
        
        # Check if either channel is silent
        if rms_left < 0.001:
            print("❌ Error: Left channel is silent!")
            return False
        if rms_right < 0.001:
            print("❌ Error: Right channel is silent!")
            return False
            
        # Verify that output is actually modified by the plugin
        min_len = min(len(data_in), len(data_out))
        diff_rms = np.sqrt(np.mean((data_in[:min_len] - data_out[:min_len])**2))
        print(f"  RMS Difference (Input vs Output): {diff_rms:.6f}")
        
        if diff_rms < 1e-5:
            print("❌ Error: Output is identical to input (plugin did not process).")
            return False
            
        print("✅ Preset validation SUCCESSFUL!")
        return True
        
    except Exception as e:
        print(f"❌ Test failed with exception: {e}")
        return False

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # Load Plugin.json
    try:
        with open(PLUGIN_JSON_PATH, "r") as f:
            plugins = json.load(f)
    except Exception as e:
        print(f"Error reading {PLUGIN_JSON_PATH}: {e}")
        sys.exit(1)
        
    # Start VST server
    server_process = start_server()
    
    all_success = True
    try:
        for plugin in plugins:
            plugin_so = os.path.join(PROJECT_DIR, "vst", f"{plugin['name']}.so")
            if not os.path.exists(plugin_so):
                print(f"\n⚠️ Skipping {plugin['label']} ({plugin['name']}) because {plugin_so} does not exist.")
                continue
            success = run_preset_test(plugin)
            if not success:
                all_success = False
    finally:
        if server_process:
            print("\n🧹 Stopping VST Server...")
            server_process.terminate()
            server_process.wait()
            
    if all_success:
        print("\n🎉 ALL PLUGINS DEFAULT PRESETS VALIDATED SUCCESSFULLY! No mono-channel/silence bugs found.")
        sys.exit(0)
    else:
        print("\n❌ SOME PLUGIN PRESET TESTS FAILED.")
        sys.exit(1)

if __name__ == "__main__":
    main()
