# Deteksi Objek Berwarna Merah (Computer Vision)

Program segmentasi citra dan deteksi objek berwarna merah menggunakan ruang warna HSV dengan C++ dan OpenCV.

## Fitur Utama

- **Segmentasi Warna HSV**: Menggabungkan dua rentang nilai Hue (0-10 dan 170-180) untuk mencakup spektrum warna merah secara menyeluruh.
- **Operasi Morfologi**: Menggunakan *Morphological Opening* dan *Closing* untuk menghilangkan noise dan menutup lubang pada mask biner.
- **Deteksi Objek & Bounding Box**: Menghitung kontur, mencari centroid (titik pusat) dengan image moments, menghitung luas area, dan menampilkan bounding box hijau serta label pada objek merah.
- **Tampilan Multi-View (1 Window)**:
  - Menggabungkan ke-4 tahapan proses ke dalam satu jendela dengan sekat rapi:
    1. *Original*
    2. *Mask Merah (Biner)*
    3. *Hasil Segmentasi*
    4. *Hasil Deteksi*
  - **Dua Mode Tata Letak**: Mendukung mode **4 Kolom Berdampingan (1x4)** untuk gambar potret/vertikal dan **Grid (2x2)** untuk gambar lanskap/webcam.
- **Inspeksi Nilai HSV Interaktif**: Klik kiri pada panel *Original* untuk melihat nilai `H`, `S`, `V` piksel langsung di konsol.
- **Trackbar Kontrol Real-time**: Slider untuk kalibrasi dinamis nilai Hue, Saturasi, Value, dan Luas area minimum.

---

## Kontrol Keyboard & Mouse

| Tombol / Aksi | Fungsi |
| :--- | :--- |
| **`ESC`** atau **`q`** | Keluar dari program |
| **`t`** atau **`TAB`** | Ganti mode tata letak (4 Kolom <-> Grid 2x2) |
| **`s`** | Simpan gambar hasil (`hasil_1` s/d `hasil_4` dan `hasil_multi_view.png`) |
| **`p`** | Cetak parameter HSV aktif ke konsol |
| **Klik Kiri Mouse** | Klik pada panel "1. Original" untuk mencetak koordinat dan nilai `H, S, V` piksel |

---

## Prasyarat & Dependensi

- **Compiler C++**: Visual Studio (MSVC) C++17 atau yang kompatibel
- **OpenCV**: OpenCV 4.x (C++ build x64)

---

## Cara Kompilasi (MSVC / Developer Command Prompt)

Jalankan perintah berikut di **x64 Native Tools Command Prompt for VS** atau Developer PowerShell:

```cmd
cl.exe /EHsc /std:c++17 /O2 /I"<path_ke_opencv>\build\include" DeteksiMerah.cpp /link /LIBPATH:"<path_ke_opencv>\build\x64\vc16\lib" opencv_world4110.lib /OUT:DeteksiMerah.exe
```

> **Catatan**: Pastikan file `opencv_world4110.dll` berada di folder yang sama dengan file `DeteksiMerah.exe` agar dapat dieksekusi.

---

## Cara Menjalankan

1. **Menggunakan Gambar File**:
   ```powershell
   .\DeteksiMerah.exe Jambu.jpg
   ```
   atau
   ```powershell
   .\DeteksiMerah.exe Strawberry.jpg
   ```

2. **Menggunakan Webcam (Live Stream)**:
   ```powershell
   .\DeteksiMerah.exe
   ```
