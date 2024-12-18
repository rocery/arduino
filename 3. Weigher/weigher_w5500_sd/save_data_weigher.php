<?php
// Konfigurasi database
$host = "localhost";
$username = "root";
$password = "";
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

    // Periksa apakah file bisa dibuka
    if (is_uploaded_file($fileTmpPath)) {
        $fileContent = file($fileTmpPath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

        // Query untuk menyimpan ke tabel utama
        $stmt = $conn->prepare("INSERT INTO data_log (col1, col2, col3, col4, col5) VALUES (?, ?, ?, ?, ?)");
        // Query untuk menyimpan ke tabel log_fail
        $stmtFail = $conn->prepare("INSERT INTO log_fail (failed_data) VALUES (?)");

        foreach ($fileContent as $line) {
            // Pisahkan data berdasarkan koma
            $data = explode(",", $line);

            // Pastikan jumlah kolom sesuai
            if (count($data) === 5) {
                $stmt->bind_param("ssdss", $data[0], $data[1], $data[2], $data[3], $data[4]);

                if (!$stmt->execute()) {
                    // Jika gagal simpan, catat baris yang gagal ke tabel log_fail
                    $stmtFail->bind_param("s", $line);
                    $stmtFail->execute();
                }
            } else {
                // Jika format salah, catat langsung ke tabel log_fail
                $stmtFail->bind_param("s", $line);
                $stmtFail->execute();
            }
        }

        $stmt->close();
        $stmtFail->close();

        echo "Proses selesai. Data berhasil dan gagal diproses telah dicatat.";
    } else {
        echo "Gagal membaca file.";
    }
} else {
    echo "Tidak ada file yang diunggah.";
}

$conn->close();
?>
