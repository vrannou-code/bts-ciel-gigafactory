import requests
from flask import Flask, request, jsonify
from ldap3 import Server, Connection, ALL, SUBTREE

app = Flask(__name__)

LDAP_HOST = 'openldap'
LDAP_PORT = 389
LDAP_USER = 'cn=admin,dc=example,dc=org'
LDAP_PASS = 'admin'
BASE_DN   = 'ou=people,dc=example,dc=org'

# Telegraf Configuration
TELEGRAF_URL = "http://telegraf:8186/logs"

def send_to_telegraf(payload: dict):
    try:
        r = requests.post(TELEGRAF_URL, json=payload, timeout=2)
        print("→ Telegraf payload:", payload, flush=True)
        print("→ Telegraf POST status:", r.status_code, flush=True)
        r.raise_for_status()
    except requests.RequestException as err:
        print(f"[WARN] Telegraf logging failed: {err}", flush=True)


@app.route("/check_badge", methods=["POST"])
def check_badge():
    data = request.get_json()
    badge = data.get("badgeNumber")
    device_id = data.get("device_id")
    zone = data.get("zone")

    if not badge:
        return jsonify({"error": "badgeNumber missing"}), 400

    try:
        server = Server(LDAP_HOST, port=LDAP_PORT, get_info=ALL)
        conn = Connection(server, user=LDAP_USER, password=LDAP_PASS, auto_bind=True)

        search_filter = f"(employeeNumber={badge})"
        conn.search(search_base=BASE_DN, search_filter=search_filter, search_scope=SUBTREE)

        access_granted = len(conn.entries) > 0
        conn.unbind()
        
        status = "granted" if access_granted else "refused"

        # Construction et envoi
        payload = {
            "badge_id": badge,
            "zone": zone,
            "status": status,
            "device_id": device_id,
            "access": 1
        }
        
        # journalisation TIG
        send_to_telegraf(payload)
        
        return jsonify({"access": access_granted})

    except Exception as e:
        return jsonify({"error": str(e)}), 500
