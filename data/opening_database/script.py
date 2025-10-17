import requests
import re
import zipfile
import chess.pgn
import json
import random

EXTRACT_MOVES = 10
MAX_GAMES = 20000
OUTPUT_FILE = "openings.json"

HEADERS = {
    "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                  "(KHTML, like Gecko) Chrome/120.0 Safari/537.36"
}

# === STEP 1: Find latest TWIC zip ===
print("🌐 Fetching latest TWIC link...")
index_url = "https://theweekinchess.com/twic"
res = requests.get(index_url, headers=HEADERS)
res.raise_for_status()

match = re.search(r'https://theweekinchess\.com/zips/twic\d+g\.zip', res.text)
if not match:
    raise Exception("❌ Could not find TWIC zip link on page.")
zip_url = match.group(0)
print(f"✅ Found latest TWIC zip: {zip_url}")

# === STEP 2: Download ZIP ===
zip_path = "twic_latest.zip"
print("📦 Downloading TWIC zip...")
with requests.get(zip_url, stream=True, headers=HEADERS) as r:
    r.raise_for_status()
    with open(zip_path, 'wb') as f:
        for chunk in r.iter_content(chunk_size=8192):
            f.write(chunk)
print("✅ Downloaded:", zip_path)

# === STEP 3: Extract PGN ===
print("📂 Extracting PGN...")
pgn_path = None
with zipfile.ZipFile(zip_path, 'r') as zf:
    for name in zf.namelist():
        if name.lower().endswith(".pgn"):
            pgn_path = name
            zf.extract(name)
            break
if not pgn_path:
    raise Exception("❌ No PGN file found in TWIC zip.")
print("✅ Extracted:", pgn_path)

# === STEP 4: Parse games ===
openings = []
print("🧠 Parsing PGN games...")
with open(pgn_path, 'r', encoding='utf-8', errors='ignore') as pgn:
    for i in range(MAX_GAMES):
        game = chess.pgn.read_game(pgn)
        if not game:
            break
        board = game.board()
        moves = []
        for j, move in enumerate(game.mainline_moves()):
            if j >= EXTRACT_MOVES:
                break
            moves.append(move.uci())
            board.push(move)
        if len(moves) > 4:
            openings.append(moves)
print(f"✅ Collected {len(openings)} openings")

# === STEP 5: Save JSON ===
random.shuffle(openings)
with open(OUTPUT_FILE, "w") as f:
    json.dump(openings, f, indent=2)
print(f"💾 Saved {len(openings)} openings to {OUTPUT_FILE}")
