<?php
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

// Periksa apakah file diunggah
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['file'])) {
    $fileTmpPath = $_FILES['file']['tmp_name'];
    $originalFileName = basename($_FILES['file']['name']);

    // Periksa apakah file bisa dibuka
    if (is_uploaded_file($fileTmpPath)) {
        // Baca seluruh konten file asli
        $originalFileContent = file_get_contents($fileTmpPath);
        $fileContent = file($fileTmpPath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

        // Siapkan query untuk tabel utama
        $stmt = $conn->prepare("INSERT INTO data_weigher (device_id, device_name, product, weight, date, ip_address, wifi) VALUES (?, ?, ?, ?, ?, ?, ?)");

        // Siapkan query untuk tabel log_fail
        $stmtFail = $conn->prepare("INSERT INTO log_fail (data_log, date) VALUES (?, ?)");

        // Variabel untuk menghitung data berhasil dan gagal
        $successCount = 0;
        $failCount = 0;

        // Iterasi setiap baris file
        foreach ($fileContent as $line) {
            // Pisahkan data berdasarkan koma
            $data = explode(",", $line);

            // Pastikan jumlah kolom sesuai
            if (count($data) === 6) {
                $currentDate = date("Y-m-d H:i:s"); // Format waktu saat ini

                // Bind parameter ke tabel utama
                $stmt->bind_param("sssdsss", $data[0], $data[1], $data[2], $data[3], $currentDate, $data[4], $data[5]);

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

        // Tutup statement
        $stmt->close();
        $stmtFail->close();

        // Simpan log ke file
        $dateNow = date("Y-m-d");
        $logDir = "log_txt/" . $dateNow;
        if (!is_dir($logDir)) {
            mkdir($logDir, 0777, true); // Buat folder jika belum ada
        }

        $logFileName = $logDir . "/" . basename($_FILES['file']['name'], ".txt") . "_" . date("Y-m-d_H-i-s") . ".txt";

        // Gabungkan informasi proses dengan konten file asli
        $logFileContent = "Proses selesai.\nBerhasil: $successCount baris\nGagal: $failCount baris\n\n--- Konten File Asli ---\n" . $originalFileContent;

        file_put_contents($logFileName, $logFileContent))

        echo
    } else {
        echo "Gagal membaca file.";
    }
} else {
    echo "Tidak ada file yang diunggah.";
}

// Tutup koneksi
$conn->close();
?>
