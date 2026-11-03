import csv
import io
import os
from datetime import datetime
from decimal import Decimal, InvalidOperation

import pymysql
from flask import Flask, jsonify, request

app = Flask(__name__)
app.config["MAX_CONTENT_LENGTH"] = 10 * 1024 * 1024

DB_CONFIG = {
    "host": os.getenv("DB_HOST", "127.0.0.1"),
    "port": int(os.getenv("DB_PORT", "3306")),
    "user": os.getenv("DB_USER", "weigher"),
    "password": os.getenv("DB_PASSWORD", ""),
    "database": os.getenv("DB_NAME", "iot"),
    "charset": "utf8mb4",
    "autocommit": False,
}

SQL_REALTIME = """
INSERT IGNORE INTO weigher_realtime
(id, device_id, device_name, product, weight, ip_address, wifi, device_time)
VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
"""

SQL_LOG = """
INSERT IGNORE INTO weigher_log
(id, device_id, device_name, product, weight, ip_address, wifi, device_time)
VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
"""


def get_db():
    return pymysql.connect(**DB_CONFIG)


def parse_row(item_id, device_id, device_name, product, weight, device_time, ip_address, wifi):
    item_id = item_id.strip()
    device_id = device_id.strip()
    product = product.strip()
    if not item_id or not device_id or not product:
        raise ValueError("field kosong")
    if len(item_id) > 48:
        raise ValueError("id terlalu panjang")
    try:
        weight_value = Decimal(weight.strip())
    except InvalidOperation:
        raise ValueError("berat tidak valid")
    if weight_value <= 0:
        raise ValueError("berat tidak valid")
    parsed_time = datetime.strptime(device_time.strip(), "%Y-%m-%d %H:%M:%S")
    return (
        item_id,
        device_id,
        device_name.strip(),
        product,
        weight_value,
        ip_address.strip(),
        wifi.strip(),
        parsed_time,
    )


@app.get("/health")
def health():
    return jsonify(status="ok")


@app.post("/weigher/save_weigher")
def save_weigher():
    form = request.form
    required = ["id", "device_id", "device_name", "product", "weight", "ip_address", "wifi", "timestamp"]
    missing = [k for k in required if k not in form]
    if missing:
        return jsonify(status="error", message="field hilang: " + ", ".join(missing)), 400

    try:
        row = parse_row(
            form["id"],
            form["device_id"],
            form["device_name"],
            form["product"],
            form["weight"],
            form["timestamp"],
            form["ip_address"],
            form["wifi"],
        )
    except ValueError as e:
        return jsonify(status="error", message=str(e)), 400

    conn = get_db()
    try:
        with conn.cursor() as cur:
            cur.execute(SQL_REALTIME, row)
        conn.commit()
    except Exception as e:
        conn.rollback()
        return jsonify(status="error", message=str(e)), 500
    finally:
        conn.close()

    return jsonify(status="success")


@app.post("/weigher/upload_log_weigher")
def upload_log_weigher():
    file = request.files.get("file")
    if file is None:
        return jsonify(status="error", message="file tidak ada"), 400

    text = file.read().decode("utf-8", errors="replace")
    rows = []
    for number, record in enumerate(csv.reader(io.StringIO(text)), start=1):
        if not record or all(not c.strip() for c in record):
            continue
        if len(record) != 8:
            return jsonify(status="error", message="baris %d kolom tidak sesuai" % number), 400
        try:
            rows.append(parse_row(*record))
        except ValueError as e:
            return jsonify(status="error", message="baris %d: %s" % (number, e)), 400

    if not rows:
        return jsonify(status="error", message="file kosong"), 400

    conn = get_db()
    try:
        with conn.cursor() as cur:
            cur.executemany(SQL_LOG, rows)
        conn.commit()
    except Exception as e:
        conn.rollback()
        return jsonify(status="error", message=str(e)), 500
    finally:
        conn.close()

    return jsonify(status="success", jumlah_data=len(rows))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001)
