"""Teste do caminho MP3 do /process (conversão ffmpeg + leitura sequencial).

Casos:
  1. MP3 curto, preview=true  (conversão + trecho de 10s)
  2. MP3 curto, preview=false (conversão + processamento completo)
  3. MP3 longo (~3 min), preview=false — caso que saturava CPU antes do fix
"""
import os
import time
import requests
import soundfile as sf
import numpy as np

SERVER_URL = "http://localhost:18080/process"
OUT_DIR = "/tmp"

# filter-stereo somente com bypass off (p0=0), demais params no default —
# mesmo setup do test_api.py, que produz saída audível.
PARAMS = {
    "plugin": "filter-stereo",
    "p0": "0.0",
}


def run_case(name, mp3_path, preview, expected_dur, timeout=300):
    print(f"\n=== {name} ===")
    url_params = dict(PARAMS)
    url_params["preview"] = "true" if preview else "false"
    url_params["previewStartTime"] = "0"

    t0 = time.time()
    with open(mp3_path, "rb") as f:
        files = {"audio_file": (os.path.basename(mp3_path), f, "audio/mpeg")}
        resp = requests.post(SERVER_URL, params=url_params, files=files, timeout=timeout)
    elapsed = time.time() - t0

    print(f"  HTTP {resp.status_code} em {elapsed:.2f}s, "
          f"Content-Type={resp.headers.get('Content-Type')}, {len(resp.content)} bytes")
    if resp.status_code != 200:
        print(f"  ❌ FALHOU: {resp.text[:200]}")
        return False

    out_path = os.path.join(OUT_DIR, f"out_{name}.wav")
    with open(out_path, "wb") as f:
        f.write(resp.content)

    data, sr = sf.read(out_path)
    dur = len(data) / sr
    peak = float(np.max(np.abs(data)))
    rms = float(np.sqrt(np.mean(np.square(data))))
    n_nan = int(np.sum(~np.isfinite(data)))
    print(f"  saída: {data.shape} @ {sr} Hz -> {dur:.2f}s | peak={peak:.4f} rms={rms:.4f} nan/inf={n_nan}")

    ok = True
    if n_nan:
        print("  ❌ contém NaN/Inf")
        ok = False
    if peak < 1e-5:
        print("  ❌ saída silenciosa")
        ok = False
    if abs(dur - expected_dur) > 1.0:
        print(f"  ❌ duração esperada ~{expected_dur:.1f}s, veio {dur:.2f}s")
        ok = False
    # blocos zerados no meio (sintoma de leitura quebrada)?
    nblk = len(data) // 4096
    blocks = data[: nblk * 4096].reshape(nblk, -1)
    silent_blocks = int(np.sum(np.max(np.abs(blocks), axis=1) < 1e-6))
    if silent_blocks > nblk * 0.05:
        print(f"  ❌ {silent_blocks}/{nblk} blocos silenciosos no meio do áudio")
        ok = False
    print("  ✅ OK" if ok else "  ❌ PROBLEMA")
    return ok


if __name__ == "__main__":
    short_dur = sf.info("/tmp/test_short_ref.wav").duration if os.path.exists("/tmp/test_short_ref.wav") else 3.25
    results = {
        "mp3_curto_preview": run_case("mp3_curto_preview", "/tmp/test_short.mp3", True, 3.25),
        "mp3_curto_full": run_case("mp3_curto_full", "/tmp/test_short.mp3", False, 3.25),
        "mp3_longo_3min_full": run_case("mp3_longo_3min_full", "/tmp/test_long.mp3", False, 180.0),
    }
    print("\n========== RESUMO MP3 ==========")
    for k, v in results.items():
        print(f"  {k}: {'PASS' if v else 'FAIL'}")
    exit(0 if all(results.values()) else 1)
