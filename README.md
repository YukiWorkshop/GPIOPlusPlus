# GPIO++
Easy-to-use C++ library for the new Linux GPIO API.

## Requirements
- Linux kernel 4.8+

And reasonably new versions of:
-  C++17 compatible compiler
-  CMake

## Install
Use of Git submodule and CMake subdirectory is recommended.

```shell script
mkdir cpp_modules && cd cpp_modules
git submodule add https://github.com/YukiWorkshop/GPIOPlusPlus
```

```cmake
add_subdirectory(cpp_modules/GPIOPlusPlus)
include_directories(cpp_modules/GPIOPlusPlus)
target_link_libraries(your_project GPIOPlusPlus)
```

## Usage
```cpp
#include <GPIO++.hpp>

using namespace YukiWorkshop;
```

Open a device by number:
```cpp
try {
    auto d = GPIO::Device(0);
} catch (std::system_error& e) {
    // Error handling here
}
```

...or by path:
```cpp
try {
    auto d = GPIO::Device("/dev/gpiochip0");
} catch (std::system_error& e) {
    // Error handling here
}
```

...or by label:
```cpp
try {
    auto d = GPIO::find_device_by_label("pinctrl-bcm2835");
} catch (...) {
    // No device with this name
}
```

Get all available devices:

```cpp
std::vector<GPIO::Device> devs = GPIO::all_devices();
```

Basic line operations:
```cpp
auto line0 = d.line(0, GPIO::LineMode::Input);
printf("Line 0: %s\n", line0.read() ? "HIGH" : "LOW");

auto line1 = d.line(1, GPIO::LineMode::Output);
line1.write(1);
```

TBD

No more `digitalWrite`s!!

## License
LGPLv3