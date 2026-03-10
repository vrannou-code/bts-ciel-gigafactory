#!/usr/bin/env python3

import requests
import random
import time
import datetime

TELEGRAF_URL = "http://localhost:8186/logs"

zones = ["zone-technique", "zone-administration"]
devices = {
    "zone-technique": "disco-l475-iot01A-zt",
    "zone-administration": "disco-l475-iot01A-za"
}
badges = [
    {"id": "01:02:03:04", "status": "granted"},
    {"id": "01:02:03:05", "status": "refused"},
    {"id": "01:02:03:06", "status": "refused"},
    {"id": "01:02:03:07", "status": "refused"}
]

def generate_event():
    zone = random.choice(zones)
    badge = random.choice(badges)
    payload = {
        "badge_id": badge["id"],
        "zone": zone,
        "status": badge["status"],
        "device_id": devices[zone],
        "access": 1
    }
    return payload

def main():
    while True:
        data = generate_event()
        try:
            response = requests.post(TELEGRAF_URL, json=data)
            print(f"[{datetime.datetime.now()}] POST {data} -> {response.status_code}")
        except Exception as e:
            print(f"[!] Erreur: {e}")
        time.sleep(30)

if __name__ == "__main__":
    main()
