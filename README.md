# Mannequin

**WD Studios Visual, Automated Graphics Testing Framework for DX12, Vulkan, DX11, and PS5 (NDA).**

Mannequin captures GPU-rendered frames, compares them against reference images using perceptual metrics (NVIDIA FLIP), and reports visual regressions across all supported graphics APIs.

## Architecture

```
Mannequin/
├── Code/
│   ├── Engine/
│   │   ├── Core/                    # Core utilities and platform abstraction
│   │   └── RendererCore/            # Graphics Abstraction Layer (GAL)
│   │       ├── Device/              # nsGALDevice — abstract GPU device interface
│   │       ├── nsGALDeclarations.h  # Enums: formats, blend, topology, etc.
│   │       ├── nsGALDescriptions.h  # Resource creation descriptions
│   │       ├── nsGALHandles.h       # Typed opaque GPU resource handles
│   │       └── Utils/               # Visual testing pipeline
│   │           ├── nsImageCapture    # GPU → CPU image readback
│   │           ├── nsImageComparator # FLIP-based perceptual comparison
│   │           └── nsVisualTestRunner# Orchestrates test execution & reporting
│   ├── EnginePlugins/
│   │   ├── RendererDX12/            # Direct3D 12 backend (implemented)
│   │   ├── RendererVulkan/          # Vulkan backend (stub)
│   │   └── RendererDX11/            # Direct3D 11 backend (stub)
│   ├── Arbitor/                     # WPF GUI — test runner & result viewer
│   │   ├── T3Bootstrap/             # Main application window
│   │   └── T3Foundation/            # Framework: logging, services, models
│   ├── Tools/                       # ShaderCompiler, TexConv, Inspector, etc.
│   ├── UnitTests/                   # CoreTest, RendererTest, TestFramework
│   ├── Samples/                     # Example visual tests
│   ├── BuildSystem/                 # CMake + Azure Pipelines CI/CD
│   └── ThirdParty/                  # FLIP, DirectXCompiler, stb, etc.
├── Data/
│   ├── Base/Shaders/                # Default and utility shaders
│   └── UnitTests/                   # Test data and reference images
└── README.md
```

## Key Components

### Graphics Abstraction Layer (`nsGALDevice`)
API-agnostic interface that all renderer backends implement. Supports:
- Device lifecycle, resource creation (textures, buffers, swap chains)
- Command recording (render passes, draw calls, compute dispatch)
- **Readback** — critical for capturing frames back to CPU for comparison

### Visual Test Pipeline
1. **`nsImageCapture`** — Reads render target contents from GPU to CPU memory
2. **`nsImageComparator`** — Compares captured images against references using FLIP metrics
3. **`nsVisualTestRunner`** — Orchestrates test execution, generates JSON/JUnit reports

### Arbitor GUI
WPF application with:
- Test list with pass/fail indicators
- Side-by-side image comparison (test vs. reference vs. diff heatmap)
- Detailed metrics panel (mean error, max error, percentile, pixel failure rate)
- Log output and test run summary
- JSON result import from C++ test runner

## Building

### Prerequisites
- CMake 3.20+
- Visual Studio 2022 (for DX12 backend)
- .NET 8.0 SDK (for Arbitor GUI)
- Vulkan SDK (optional, for Vulkan backend)

### Build Steps
```bash
# Initialize submodules (FLIP)
git submodule update --init --recursive

# Configure with CMake
cmake -B build -S Code

# Build
cmake --build build --config Release
```

## Running Tests

### Command Line
```bash
./RendererTest --api DX12 --json-output --json-path results.json
```

### Arbitor GUI
Launch the Arbitor application, select your target API, and click "Run All Tests".
Results appear in the test list with comparison images.

## Supported Platforms
- **Windows** — DX12 (primary), DX11, Vulkan
- **Linux** — Vulkan
- **macOS** — Vulkan (MoltenVK)
- **Android** — Vulkan
- **PS5** — GNM (NDA protected)
- **Xbox (GDKX)** — DX12

## License
See [LICENSE](LICENSE) for details.
