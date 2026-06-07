import os
import sys
import requests
import soundfile as sf
import numpy as np

# Configuration
SERVER_URL = "http://localhost:18080/process"
PROJECT_DIR = "/home/filipe/projects/retro-vst/web-server-vst"
INPUT_WAV = os.path.join(PROJECT_DIR, "resources/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav")
OUTPUT_DIR = os.path.join(PROJECT_DIR, "tmp")

def analyze_audio(input_path, output_path):
    print(f"\nAnalyzing output audio at {output_path} against input {input_path}...")
    
    # Check if file exists and has size > 0
    if not os.path.exists(output_path):
        print("❌ Error: Output file does not exist.")
        return False
        
    size = os.path.getsize(output_path)
    if size == 0:
        print("❌ Error: Output file is 0 bytes.")
        return False
        
    try:
        data_in, sr_in = sf.read(input_path)
        data_out, sr_out = sf.read(output_path)
    except Exception as e:
        print(f"❌ Error reading audio file: {e}")
        return False
        
    # Check shapes
    print(f"  Input:  {data_in.shape} samples, SR={sr_in}")
    print(f"  Output: {data_out.shape} samples, SR={sr_out}")
    
    if sr_in != sr_out:
        print(f"⚠️ Warning: Sample rate mismatch (in: {sr_in}, out: {sr_out})")
        
    if len(data_out) == 0:
        print("❌ Error: Output audio data is empty.")
        return False
        
    # Calculate stats
    max_val = np.max(np.abs(data_out))
    rms_val = np.sqrt(np.mean(data_out**2))
    
    print(f"  Output Max Amplitude: {max_val:.6f}")
    print(f"  Output RMS Amplitude: {rms_val:.6f}")
    
    # 1. Silence check
    if max_val < 1e-5:
        print("❌ Error: Output audio is silent (all values near 0).")
        return False
        
    # 2. Clipping check
    clipping_samples = np.sum(np.abs(data_out) >= 1.0)
    if clipping_samples > 0:
        clipping_pct = (clipping_samples / data_out.size) * 100
        print(f"⚠️ Warning: Output is clipping! {clipping_samples} samples ({clipping_pct:.4f}%) reached peak 1.0.")
    else:
        print("  Clipping check passed (no peak >= 1.0).")
        
    # 3. Processing verification (compare input vs output)
    min_len = min(len(data_in), len(data_out))
    compare_in = data_in[:min_len]
    compare_out = data_out[:min_len]
    
    if compare_in.ndim != compare_out.ndim:
        print("  Channel layout changed by plugin.")
        difference_rms = 1.0
    else:
        difference_rms = np.sqrt(np.mean((compare_in - compare_out)**2))
        
    print(f"  RMS of Difference (Input vs Output): {difference_rms:.6f}")
    
    # Check if the output actually changed
    if difference_rms < 1e-5:
        print("❌ Error: Output audio is identical to input! The VST plugin did not alter the audio.")
        return False
    else:
        print("  Processing verification passed (signal changed).")
        
    print("✅ Audio verification SUCCESSFUL! The processed audio is valid, audible, and modified by the VST.")
    return True

def run_test(plugin_name, params_dict):
    print(f"\n==================================================")
    print(f"Testing VST Plugin: {plugin_name}")
    print(f"==================================================")
    
    output_filename = f"processed_{plugin_name}.wav"
    output_path = os.path.join(OUTPUT_DIR, output_filename)
    
    # Build query params
    url_params = {
        "plugin": plugin_name,
        "preview": "true",
        "previewStartTime": "0"
    }
    # Add parameter values sparsely
    for param_idx, val in params_dict.items():
        url_params[f"p{param_idx}"] = str(val)
        
    print(f"Sending POST request to {SERVER_URL} with sparse params: {url_params}")
    
    try:
        with open(INPUT_WAV, "rb") as f:
            files = {"audio_file": (os.path.basename(INPUT_WAV), f, "audio/wav")}
            response = requests.post(SERVER_URL, params=url_params, files=files, timeout=30)
            
        if response.status_code != 200:
            print(f"❌ Server returned error code {response.status_code}: {response.text}")
            return False
            
        # Write output file
        with open(output_path, "wb") as f_out:
            f_out.write(response.content)
        print(f"Output saved to {output_path}")
        
        # Analyze
        return analyze_audio(INPUT_WAV, output_path)
        
    except Exception as e:
        print(f"❌ Request failed: {e}")
        return False

if __name__ == "__main__":
    # Ensure OUTPUT_DIR exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # 1. filter-stereo
    # We only set p0=0.0 (bypass off). Input and output gain remain at default 0dB!
    filter_params = {0: 0.0}
    success_filter = run_test("filter-stereo", filter_params)
    
    # 2. compressor-stereo
    # We only set p0=0.0 (bypass off). Input and output gain remain at default 0dB!
    comp_params = {0: 0.0}
    success_comp = run_test("compressor-stereo", comp_params)

    # 3. para-equalizer-x8-stereo
    # We set bypass=off (p0=0.0), filter type of band 0 = Peak/Bell (p19=0.5), and gain of band 0 = boost (p26=0.8)
    # We do NOT set intermediate parameters like g_in (p1) and g_out (p2) so they remain at default 0dB!
    eq_params = {
        0: 0.0,    # bypass off
        19: 0.5,   # ft_0 = Bell/Peak
        26: 0.8    # g_0 = boost
    }
    success_eq = run_test("para-equalizer-x8-stereo", eq_params)

    # 4. PitchedDelay
    # We enable Tap 1 (p15=1.0) and turn up volume (p13=0.8) and feedback (p7=0.5) and delay (p6=0.5)
    delay_params = {
        6: 0.5,    # delay
        7: 0.5,    # feedback
        13: 0.8,   # volume
        15: 1.0    # enabled
    }
    success_delay = run_test("PitchedDelay", delay_params)

    print("\n\n==================================================")
    print("SUMMARY OF TESTS:")
    print(f"  filter-stereo:            {'PASS' if success_filter else 'FAIL'}")
    print(f"  compressor-stereo:        {'PASS' if success_comp else 'FAIL'}")
    print(f"  para-equalizer-x8-stereo: {'PASS' if success_eq else 'FAIL'}")
    print(f"  PitchedDelay:             {'PASS' if success_delay else 'FAIL'}")
    print("==================================================")
    
    if not (success_filter and success_comp and success_eq and success_delay):
        sys.exit(1)
    else:
        sys.exit(0)
