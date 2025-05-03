
---

# VeloBar

**VeloBar** is an open-source, lightweight, and customizable **topbar** component for Windows.  
It uses **Qt QML** to create a modern, native UI with hardware acceleration and blur effects, making it perfect for creating sleek topbars for desktop applications.

---

## Features

- 🖥️ **Native Topbar Component**: Create a persistent bar across the top of your screen.
- 🎨 **Modern UI**: Built with Qt QML for smooth, hardware-accelerated graphics.
- 🌟 **Acrylic Effects**: Windows 10/11 blur effects support.
- 📱 **System Integration**: 
  - Native menu extraction from active windows
  - Network status monitoring
  - Battery status
  - System time and date
- ⚡ **Lightweight and Fast**: Minimal system resource usage.
- 🛠️ **Open Source**: MIT licensed.

---

## Requirements

- **Windows 10 or later**
- **Qt 6.5.0 or later**
- **C++ Development Environment**
  - Visual Studio 2019/2022 with MSVC compiler

---

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/rinechxn/velobar.git
   cd velobar
   ```

2. Build using Qt Creator:
   - Open `Bar.pro` in Qt Creator
   - Configure the project with your Qt kit
   - Build and run!

Alternative: Command line build:
```bash
qmake Bar.pro
make (or nmake with MSVC)
```

---

## Technical Features

- **Native Menu Integration**
  - Automatically captures and displays menus from active windows
  - Maintains native functionality while providing modern styling

- **System Integration**
  - Real-time network status monitoring (Ethernet/WiFi)
  - Battery status and charging indication
  - System clock and date display

- **Modern UI Effects**
  - Acrylic blur (Windows 10/11)
  - Dynamic transparency
  - Smooth animations
  - Custom shader effects

- **High DPI Support**
  - Automatic scaling for high DPI displays
  - Crisp rendering on all screen resolutions

---

## Customization

The UI is built with QML, making it easy to customize:

- **Styling**: Modify the QML files to change appearances
- **Layouts**: Adjust component positioning and arrangement
- **Effects**: Customize blur, transparency, and animations
- **Functionality**: Add new features through C++ and QML

---

## Contributing

Pull requests are welcome! Areas for contribution:

- UI/UX improvements
- Performance optimizations
- New features
- Bug fixes
- Documentation

Please follow the project's coding style and include appropriate tests.

---

## Related Projects

- [Qt](https://www.qt.io/)
- [Qt QML](https://doc.qt.io/qt-6/qmlapplications.html)
- [Windows UI Library](https://learn.microsoft.com/en-us/windows/apps/winui/)