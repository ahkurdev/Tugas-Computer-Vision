Bahasa Indonesia version: [readme.id.md](readme.id.md)

# Red Object Detection (Computer Vision)

Image segmentation and red-object detection program using the HSV colour space with C++ and OpenCV.

## Key Features

- **HSV Colour Segmentation**: Combines two hue ranges (0-10 and 170-180) to cover the full red spectrum.
- **Morphological Operations**: Uses *Morphological Opening* and *Closing* to remove noise and fill holes in the binary mask.
- **Object Detection & Bounding Box**: Computes contours, finds the centroid (centre point) with image moments, calculates the area, and draws a green bounding box with a label on red objects.
- **Multi-View Display (1 Window)**:
  - Combines all 4 processing stages into a single window with neat dividers:
    1. *Original*
    2. *Red Mask (Binary)*
    3. *Segmentation Result*
    4. *Detection Result*
  - **Two Layout Modes**: Supports **4 Side-by-Side Columns (1x4)** for portrait/vertical images and **Grid (2x2)** for landscape images/webcam.
- **Interactive HSV Value Inspection**: Left-click on the *Original* panel to see the pixel's `H`, `S`, `V` values directly in the console.
- **Real-time Control Trackbars**: Sliders for dynamic calibration of Hue, Saturation, Value, and minimum area.

---

## Keyboard & Mouse Controls

| Key / Action | Function |
| :--- | :--- |
| **ESC** or **q** | Exit the program |
| **t** or **TAB** | Switch layout mode (4 Columns <-> 2x2 Grid) |
| **s** | Save result images (`hasil_1` to `hasil_4` and `hasil_multi_view.png`) |
| **p** | Print the active HSV parameters to the console |
| **Left Mouse Click** | Click on the "1. Original" panel to print the pixel's coordinates and `H, S, V` values |

---

## Prerequisites & Dependencies

- **C++ Compiler**: Visual Studio (MSVC) C++17 or compatible
- **OpenCV**: OpenCV 4.x (C++ build x64)

---

## How to Compile (MSVC / Developer Command Prompt)

Run the following command in the **x64 Native Tools Command Prompt for VS** or Developer PowerShell:

```cmd
cl.exe /EHsc /std:c++17 /O2 /I"<path_to_opencv>\build\include" DeteksiMerah.cpp /link /LIBPATH:"<path_to_opencv>\build\x64\vc16\lib" opencv_world4110.lib /OUT:DeteksiMerah.exe
```

> **Note**: Make sure the `opencv_world4110.dll` file is in the same folder as `DeteksiMerah.exe` so it can run.

---

## How to Run

1. **Using an Image File**:
   ```powershell
   .\DeteksiMerah.exe Jambu.jpg
   ```

   or

   ```powershell
   .\DeteksiMerah.exe Strawberry.jpg
   ```

2. **Using a Webcam (Live Stream)**:
   ```powershell
   .\DeteksiMerah.exe
   ```
