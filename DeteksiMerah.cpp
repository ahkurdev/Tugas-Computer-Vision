// =====================================================================
//  UTS COMPUTER VISION - DETEKSI OBJEK BERWARNA MERAH
//  Bahasa      : C++ (OpenCV 4.x) - Microsoft Visual Studio
//  Teknik      : Segmentasi warna pada ruang warna HSV
//
//  Tampilan:
//    Semua hasil (Original, Mask, Segmentasi, Deteksi) disatukan ke
//    dalam 1 WINDOW UTAMA dengan sekat kolom rapi dan proporsional.
//
//  Cara pakai:
//    1) Tanpa argumen  -> memakai webcam :  DeteksiMerah.exe
//    2) Dengan gambar  -> memakai file   :  DeteksiMerah.exe apel.jpg
//
//  Tombol keyboard:
//    ESC / q : keluar
//    s       : simpan gambar hasil (individual & gabungan multi-view)
//    p       : cetak parameter HSV yang sedang dipakai ke konsol
//    t / TAB : ganti mode layout (4 Kolom Berdampingan <-> Grid 2x2)
//  Mouse:
//    Klik kiri pada kolom/panel "1. Original" -> menampilkan nilai H,S,V
//    piksel yang diklik untuk kalibrasi rentang warna merah.
// =====================================================================

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace cv;
using namespace std;

// ---------------------------------------------------------------------
// LANGKAH 1: Parameter rentang warna MERAH pada ruang warna HSV.
// Di OpenCV: H = 0..179, S = 0..255, V = 0..255.
// Warna merah berada di KEDUA UJUNG skala Hue (mendekati 0 dan 180),
// sehingga dibutuhkan DUA rentang:
//   rentang 1 : H  0 - 10
//   rentang 2 : H 170 - 180
// ---------------------------------------------------------------------
int h1_min = 0,   h1_max = 10;    // rentang merah bagian bawah
int h2_min = 170, h2_max = 180;   // rentang merah bagian atas
int s_min  = 120;                 // saturasi minimum (membuang warna pucat)
int v_min  = 70;                  // kecerahan minimum (membuang area gelap)
int min_area = 500;               // luas minimum kontur (membuang noise kecil)

// Nama jendela
const string WIN_MAIN = "Deteksi Objek Merah - Multi View";
const string WIN_CTRL = "Pengaturan HSV";

// Citra HSV disimpan global agar bisa dibaca oleh callback mouse
Mat g_hsv;

// Menyimpan posisi panel 1 (Original) pada canvas multi-view untuk callback mouse
struct PanelOriginalInfo {
    Rect rect;          // Posisi & ukuran panel Original di dalam window utama
    Size origSize;      // Ukuran asli citra
} g_panelOriginal;

// ---------------------------------------------------------------------
// LANGKAH 2 (teknik mencari warna): callback mouse.
// Saat pengguna klik pada panel Original, program mencetak nilai H, S, V
// piksel tersebut.
// ---------------------------------------------------------------------
static void onMouse(int event, int x, int y, int, void*)
{
    if (event != EVENT_LBUTTONDOWN || g_hsv.empty()) return;

    // Periksa apakah klik berada di dalam batas panel "Original"
    if (g_panelOriginal.rect.contains(Point(x, y)))
    {
        int local_x = x - g_panelOriginal.rect.x;
        int local_y = y - g_panelOriginal.rect.y;

        int orig_x = (int)((double)local_x / g_panelOriginal.rect.width * g_panelOriginal.origSize.width);
        int orig_y = (int)((double)local_y / g_panelOriginal.rect.height * g_panelOriginal.origSize.height);

        orig_x = max(0, min(orig_x, g_hsv.cols - 1));
        orig_y = max(0, min(orig_y, g_hsv.rows - 1));

        Vec3b p = g_hsv.at<Vec3b>(orig_y, orig_x);   // Vec3b = (H, S, V)
        cout << "[KLIK ORIGINAL] Koordinat (" << orig_x << ", " << orig_y << ")  ->  "
             << "H = " << (int)p[0]
             << "  S = " << (int)p[1]
             << "  V = " << (int)p[2] << endl;
    }
}

// ---------------------------------------------------------------------
// Helper: Membangun satu citra komposit (Multi-View) dengan sekat & judul kolom
// ---------------------------------------------------------------------
static Mat createMultiView(const Mat& imgOri, const Mat& imgMask,
                           const Mat& imgSeg, const Mat& imgDet,
                           bool layoutGrid)
{
    // Pastikan mask biner diubah ke format 3-channel (BGR) agar kompatibel
    Mat maskBgr;
    if (imgMask.channels() == 1)
        cvtColor(imgMask, maskBgr, COLOR_GRAY2BGR);
    else
        maskBgr = imgMask;

    const Mat panels[4] = { imgOri, maskBgr, imgSeg, imgDet };
    const string titles[4] = {
        "1. Original (Klik utk cek HSV)",
        "2. Mask Merah (Biner)",
        "3. Hasil Segmentasi",
        "4. Hasil Deteksi"
    };
    const Scalar titleColors[4] = {
        Scalar(50, 230, 255),   // Cyan / Kuning cerah
        Scalar(220, 220, 220),  // Putih abu
        Scalar(80, 230, 80),    // Hijau cerah
        Scalar(80, 180, 255)    // Oranye / Kuning
    };

    const int bannerH = 26;     // Tinggi header banner tiap kolom
    const int sekatW  = 4;      // Ketebalan sekat pemisah
    const int footerH = 24;     // Tinggi footer petunjuk tombol

    int pw = 0, ph = 0;
    int canvasW = 0, canvasH = 0;

    if (!layoutGrid)
    {
        // ------------------ MODE 1: 4 Kolom Berdampingan (1 x 4) ------------------
        // Sangat cocok untuk gambar potret (tinggi) atau monitor widescreen
        int max_pw = 320;
        int max_ph = 480;
        double scale = min((double)max_pw / imgOri.cols, (double)max_ph / imgOri.rows);
        pw = max(160, (int)(imgOri.cols * scale));
        ph = max(160, (int)(imgOri.rows * scale));

        canvasW = (pw * 4) + (sekatW * 3);
        canvasH = bannerH + ph + footerH;
    }
    else
    {
        // ------------------ MODE 2: Grid 2 Baris x 2 Kolom (2 x 2) ------------------
        // Sangat cocok untuk gambar lanskap / webcam
        int max_pw = 460;
        int max_ph = 320;
        double scale = min((double)max_pw / imgOri.cols, (double)max_ph / imgOri.rows);
        pw = max(160, (int)(imgOri.cols * scale));
        ph = max(160, (int)(imgOri.rows * scale));

        canvasW = (pw * 2) + sekatW;
        canvasH = ((bannerH + ph) * 2) + sekatW + footerH;
    }

    // Buat canvas dengan warna background gelap (modern dark theme)
    Mat canvas(canvasH, canvasW, CV_8UC3, Scalar(28, 28, 28));

    for (int i = 0; i < 4; i++)
    {
        int col = 0, row = 0;
        int cellX = 0, cellY = 0;

        if (!layoutGrid)
        {
            col = i;
            row = 0;
            cellX = col * (pw + sekatW);
            cellY = 0;
        }
        else
        {
            col = i % 2;
            row = i / 2;
            cellX = col * (pw + sekatW);
            cellY = row * (bannerH + ph + sekatW);
        }

        // 1. Gambar header banner tiap kolom
        Rect bannerRect(cellX, cellY, pw, bannerH);
        rectangle(canvas, bannerRect, Scalar(42, 42, 42), FILLED);
        line(canvas, Point(cellX, cellY + bannerH - 1),
                     Point(cellX + pw, cellY + bannerH - 1),
                     Scalar(65, 65, 65), 1);

        putText(canvas, titles[i], Point(cellX + 8, cellY + 18),
                FONT_HERSHEY_SIMPLEX, 0.45, titleColors[i], 1, LINE_AA);

        // 2. Resize citra dan tempelkan ke panel
        Mat resizedPanel;
        resize(panels[i], resizedPanel, Size(pw, ph), 0, 0, INTER_AREA);
        Rect imgRect(cellX, cellY + bannerH, pw, ph);
        resizedPanel.copyTo(canvas(imgRect));

        // 3. Catat posisi panel 1 (Original) untuk mouse click callback
        if (i == 0)
        {
            g_panelOriginal.rect = imgRect;
            g_panelOriginal.origSize = imgOri.size();
        }

        // 4. Sekat pemisah (garis pembatas)
        rectangle(canvas, Rect(cellX, cellY, pw, bannerH + ph), Scalar(65, 65, 65), 1);
    }

    // Footer petunjuk shortcut di bagian bawah window
    int footerY = canvasH - footerH;
    rectangle(canvas, Rect(0, footerY, canvasW, footerH), Scalar(20, 20, 20), FILLED);
    line(canvas, Point(0, footerY), Point(canvasW, footerY), Scalar(65, 65, 65), 1);

    string modeStr = layoutGrid ? "Grid [2x2]" : "4 Kolom [1x4]";
    string helpText = "[ESC/q] Keluar  |  [s] Simpan  |  [p] Cetak HSV  |  [t/TAB] Ganti Layout: " + modeStr;
    putText(canvas, helpText, Point(10, footerY + 16),
            FONT_HERSHEY_SIMPLEX, 0.40, Scalar(190, 190, 190), 1, LINE_AA);

    return canvas;
}

int main(int argc, char** argv)
{
    // -----------------------------------------------------------------
    // LANGKAH 3: Menentukan sumber citra (webcam atau file gambar)
    // -----------------------------------------------------------------
    VideoCapture cap;
    Mat staticImage;
    bool useCamera = (argc < 2);

    if (useCamera)
    {
        cap.open(0);
        if (!cap.isOpened())
        {
            cerr << "Webcam tidak bisa dibuka." << endl;
            return -1;
        }
    }
    else
    {
        staticImage = imread(argv[1], IMREAD_COLOR);
        if (staticImage.empty())
        {
            cerr << "Gambar tidak bisa dibaca: " << argv[1] << endl;
            return -1;
        }
    }

    // -----------------------------------------------------------------
    // LANGKAH 4: Membuat jendela utama (1 WINDOW), trackbar, dan mouse callback
    // -----------------------------------------------------------------
    namedWindow(WIN_MAIN, WINDOW_NORMAL); // Memungkinkan user resize/maximize jika diperlukan
    namedWindow(WIN_CTRL, WINDOW_NORMAL);
    resizeWindow(WIN_CTRL, 420, 320);

    setMouseCallback(WIN_MAIN, onMouse);

    createTrackbar("H1 min",   WIN_CTRL, &h1_min,   180);
    createTrackbar("H1 max",   WIN_CTRL, &h1_max,   180);
    createTrackbar("H2 min",   WIN_CTRL, &h2_min,   180);
    createTrackbar("H2 max",   WIN_CTRL, &h2_max,   180);
    createTrackbar("S min",    WIN_CTRL, &s_min,    255);
    createTrackbar("V min",    WIN_CTRL, &v_min,    255);
    createTrackbar("Luas min", WIN_CTRL, &min_area, 5000);

    // Tentukan mode awal secara pintar:
    // Jika citra lebih tinggi dari lebarnya (potret), gunakan mode 4 Kolom berdampingan.
    // Jika citra lanskap / kamera standar, gunakan mode Grid 2x2.
    bool layoutGrid = false;
    if (!useCamera && staticImage.cols >= staticImage.rows)
        layoutGrid = true;
    else if (useCamera)
        layoutGrid = true;

    cout << "==========================================================" << endl;
    cout << " PROGRAM DETEKSI OBJEK MERAH (MULTI-VIEW 1 WINDOW)" << endl;
    cout << " ESC/q   : Keluar" << endl;
    cout << " s       : Simpan hasil citra" << endl;
    cout << " p       : Cetak parameter HSV aktif" << endl;
    cout << " t / TAB : Ganti tata letak (4 Kolom <-> Grid 2x2)" << endl;
    cout << " Klik kiri pada panel '1. Original' untuk cek nilai H,S,V" << endl;
    cout << "==========================================================" << endl;

    // -----------------------------------------------------------------
    // LANGKAH 5: Loop utama pemrosesan citra
    // -----------------------------------------------------------------
    while (true)
    {
        // 5a. Ambil frame
        Mat frame;
        if (useCamera)
        {
            cap >> frame;
            if (frame.empty()) break;
            flip(frame, frame, 1);            // efek cermin agar natural
        }
        else
        {
            frame = staticImage.clone();
        }

        // 5b. Batasi ukuran frame jika terlalu besar agar pemrosesan cepat
        if (frame.cols > 1200)
        {
            double scale = 1200.0 / frame.cols;
            resize(frame, frame, Size(), scale, scale, INTER_AREA);
        }

        // 5c. Pra-pemrosesan: Gaussian blur untuk mengurangi noise
        Mat blurred, hsv;
        GaussianBlur(frame, blurred, Size(7, 7), 0);

        // 5d. Konversi BGR -> HSV.
        cvtColor(blurred, hsv, COLOR_BGR2HSV);
        g_hsv = hsv.clone();                  // untuk callback mouse

        // 5e. Thresholding warna: dua rentang merah lalu digabung (OR)
        Mat mask1, mask2, mask;
        inRange(hsv, Scalar(h1_min, s_min, v_min), Scalar(h1_max, 255, 255), mask1);
        inRange(hsv, Scalar(h2_min, s_min, v_min), Scalar(h2_max, 255, 255), mask2);
        bitwise_or(mask1, mask2, mask);

        // 5f. Operasi morfologi untuk membersihkan mask
        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
        morphologyEx(mask, mask, MORPH_OPEN,  kernel, Point(-1, -1), 2);
        morphologyEx(mask, mask, MORPH_CLOSE, kernel, Point(-1, -1), 2);

        // 5g. Segmentasi: tampilkan hanya piksel yang lolos mask
        Mat segmented;
        frame.copyTo(segmented, mask);

        // 5h. Cari kontur objek merah pada mask
        vector<vector<Point>> contours;
        Mat maskCopy = mask.clone();
        findContours(maskCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // 5i. Gambar kotak pembatas, titik pusat, dan label pada tiap objek
        Mat output = frame.clone();
        int jumlah = 0;

        for (size_t i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[i]);
            if (area < min_area || area <= 0) continue;

            jumlah++;
            Rect box = boundingRect(contours[i]);
            rectangle(output, box, Scalar(0, 255, 0), 2);

            // Titik pusat (centroid) dari image moments
            Moments m = moments(contours[i]);
            if (m.m00 > 0)
            {
                Point pusat((int)(m.m10 / m.m00), (int)(m.m01 / m.m00));
                circle(output, pusat, 5, Scalar(255, 0, 0), FILLED);
            }

            string label = "Merah #" + to_string(jumlah);
            putText(output, label, Point(box.x, max(box.y - 8, 15)),
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        }

        putText(output, "Objek merah: " + to_string(jumlah), Point(10, 25),
                FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);

        // 5j. Gabungkan ke dalam 1 window utama (Multi-View)
        Mat multiView = createMultiView(frame, mask, segmented, output, layoutGrid);
        imshow(WIN_MAIN, multiView);

        // 5k. Kontrol Keyboard
        int key = waitKey(30);
        if (key != -1)
        {
            int k = key & 0xFF;
            if (k == 27 || k == 'q' || k == 'Q') break; // ESC atau q

            // Toggle layout dengan 't' atau 'TAB' (k == 9)
            if (k == 't' || k == 'T' || k == 9)
            {
                layoutGrid = !layoutGrid;
                cout << "Layout diubah ke: " << (layoutGrid ? "Grid [2x2]" : "4 Kolom Berdampingan [1x4]") << endl;
            }

            if (k == 's' || k == 'S')
            {
                imwrite("hasil_1_original.png", frame);
                imwrite("hasil_2_mask.png", mask);
                imwrite("hasil_3_segmentasi.png", segmented);
                imwrite("hasil_4_deteksi.png", output);
                imwrite("hasil_multi_view.png", multiView);
                cout << ">> SEMUA GAMBAR HASIL BERHASIL DISIMPAN (hasil_1 s/d 4 dan hasil_multi_view.png) <<" << endl;
            }

            if (k == 'p' || k == 'P')
            {
                cout << "--- PARAMETER HSV SAAT INI ---" << endl;
                cout << "Rentang 1 : H " << h1_min << "-" << h1_max << endl;
                cout << "Rentang 2 : H " << h2_min << "-" << h2_max << endl;
                cout << "S min     = " << s_min << endl;
                cout << "V min     = " << v_min << endl;
                cout << "Luas min  = " << min_area << endl;
                cout << "------------------------------" << endl;
            }
        }
    }

    destroyAllWindows();
    return 0;
}
