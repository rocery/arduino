CREATE TABLE IF NOT EXISTS weigher_realtime (
  id VARCHAR(48) NOT NULL,
  device_id VARCHAR(32) NOT NULL,
  device_name VARCHAR(64) NOT NULL,
  product VARCHAR(64) NOT NULL,
  weight DECIMAL(10,4) NOT NULL,
  ip_address VARCHAR(45) NOT NULL,
  wifi VARCHAR(64) NOT NULL,
  device_time DATETIME NOT NULL,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_realtime_device_time (device_id, device_time),
  KEY idx_realtime_product (product)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS weigher_log (
  id VARCHAR(48) NOT NULL,
  device_id VARCHAR(32) NOT NULL,
  device_name VARCHAR(64) NOT NULL,
  product VARCHAR(64) NOT NULL,
  weight DECIMAL(10,4) NOT NULL,
  ip_address VARCHAR(45) NOT NULL,
  wifi VARCHAR(64) NOT NULL,
  device_time DATETIME NOT NULL,
  uploaded_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_log_device_time (device_id, device_time),
  KEY idx_log_product (product)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
