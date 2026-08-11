# 🛡️ SecureOS

A lightweight, hobbyist x86_32 operating system built from scratch. SecureOS focuses on learning low-level system architecture, kernel development, and bare-metal programming without relying on standard libraries.

---

## 🚀 Key Features

* **Multiboot Compliant:** Boots seamlessly via the GRUB bootloader on x86 architectures.
* **Custom Splash Screen:** Features direct bitmap graphics support (`logo.bmp`) initialized right at boot time.
* **Streamlined Build System:** A unified `Makefile` automates compilation, linking, and ISO generation.
* **Pure Low-Level:** Written strictly in **C**, **x86 Assembly** and Rust with zero external runtime dependencies.

## 🛠️ Tech Stack & Tools

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Kernel Logic** | C (GCC Cross-Compiler), Rust(Rustup Toolchain-Manager) | Core operating system logic, drivers, complex logic |
| **Bootstrapping** | x86 Assembly (NASM) | Entry point|
| **Bootloader** | GNU GRUB | Multiboot-compliant system loading |
| **Emulation** | QEMU | Fast hardware emulation for testing |

---

## 📁 Repository Structure

```text
SecureOS/
├── kernel/             # Core OS source code (C & Assembly)
|   └── include/        # Headers for self-made libraries
|       └── ...         # Headers
|   └── lib/            # Self-made libraries
|       └── ...         # Libraries
|   └── entry.asm       # GRUB entry file
|   └── kernel.c        # Main kernel file
|   └── linker          # Linkerfile
├── iso_root/           # Root directory for building the ISO image
│   └── boot/           # Boot directory for GRUB
│       └── grub/       # GRUB bootloader configuration files
|           └── grub.cfg# GRUB config file      
|── LICENSE             # MIT License
├── Makefile            # Automation script for building and cleaning
├── logo.bmp/logo.raw   # Boot screen logo image
└── README.md           # Project documentation
```

---

## 💻 Getting Started

### Prerequisites

To compile and run SecureOS, you need a Linux environment (or Windows Subsystem for Linux - WSL) with the following tools installed:

```bash
# Ubuntu / Debian setup
sudo apt update
sudo apt install build-essential nasm xorriso qemu-system-x86
```

### Building the Project

1. **Clone the repository:**
   ```bash
   git clone https://github.com
   cd SecureOS
   ```

2. **Compile the source and generate the bootable ISO:**
   ```bash
   make
   ```

### Running in Emulation

You can boot and test your freshly compiled `secureos.iso` directly using QEMU:

```bash
qemu-system-i386 -cdrom secureos.iso
```

---

## 🗺️ Development Roadmap

- [x] Set up GRUB and Multiboot headers
- [x] Implement automated Makefile build system
- [x] Basic Assembly bootloader entry point
- [x] Implement Global Descriptor Table (GDT)
- [x] Implement Interrupt Descriptor Table (IDT) & ISRs
- [x] Create a basic VGA text-mode driver for screen output
- [x] Basic Keyboard driver input handling
- [x] Basic Sound(beep) driver
- [x] PCI-Scanner
- [ ] Basic Sata
- [ ] Coming soon

---

## 🤝 Contributing

Contributions make the open-source community an amazing place to learn and create. Since SecureOS is in its early architectural stages, **any help is highly appreciated!**

1. **Fork** the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. **Commit** your Changes (`git commit -m 'Add some AmazingFeature'`)
4. **Push** to the Branch (`git push origin feature/AmazingFeature`)
5. Open a **Pull Request**

## 📄 License

Distributed under the **MIT License**. See the [`LICENSE`](LICENSE) file in the repository for more information.
