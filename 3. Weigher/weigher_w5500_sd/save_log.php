<?php
// Terima data dari Arduino
$rawData = file_get_contents("php://input");

// Konfigurasi database
$host = "localhost";
$username = "admin";
$password = "itbekasioke";
$database = "weigher";

// Koneksi ke database
$conn = new mysqli($host, $username, $password, $database);

// Cek koneksi
if ($conn->connect_error) {
    die("Koneksi gagal: " . $conn->connect_error);
}

// Siapkan query untuk tabel utama
$stmt = $conn->prepare(
    "INSERT INTO data_weigher (device_id, device_name, product, weight, date, ip_address, wifi) VALUES (?, ?, ?, ?, ?, ?, ?)"
);

// Siapkan query untuk tabel log_fail
$stmtFail = $conn->prepare(
    "INSERT INTO log_fail (data_log, date) VALUES (?, ?)"
);

// Variabel untuk menghitung data berhasil dan gagal
$successCount = 0;
$failCount = 0;

// Pisahkan baris-baris data
$lines = explode("\n", $rawData);

// Mulai transaksi
$conn->begin_transaction();

try {
    foreach ($lines as $line) {
        // Trim whitespace
        $line = trim($line);

        // Lewati baris kosong
        if (empty($line)) {
            continue;
        }

        // Pisahkan data berdasarkan koma
        $data = explode(",", $line);

        // Pastikan jumlah kolom sesuai
        if (count($data) === 6) {
            $currentDate = date("Y-m-d H:i:s");

            // Bind parameter ke tabel utama
            $stmt->bind_param(
                "sssdsss",
                $data[0], // device_id
                $data[1], // device_name
                $data[2], // product
                $data[3], // weight
                $currentDate, // date
                $data[4], // ip_address
                $data[5] // wifi
            );

            // Eksekusi dan periksa keberhasilan
            if ($stmt->execute()) {
                $successCount++;
            } else {
                // Jika gagal simpan, catat baris yang gagal ke tabel log_fail
                $stmtFail->bind_param("ss", $line, $currentDate);
                $stmtFail->execute();
                $failCount++;
            }
        } else {
            // Jika format salah, catat langsung ke tabel log_fail
            $currentDate = date("Y-m-d H:i:s");
            $stmtFail->bind_param("ss", $line, $currentDate);
            $stmtFail->execute();
            $failCount++;
        }
    }

    // Commit transaksi
    $conn->commit();

    // Tutup statement
    $stmt->close();
    $stmtFail->close();

    // Kirim respon ke Arduino
    echo "oke";
} catch (Exception $e) {
    // Rollback transaksi jika terjadi kesalahan
    $conn->rollback();
    echo "error: " . $e->getMessage();
}

// Tutup koneksi
$conn->close();
?>
