# GPIO++
[![pipeline status](https://gitlab.com/ReimuNotMoe/GPIOPlusPlus/badges/master/pipeline.svg)](https://gitlab.com/ReimuNotMoe/GPIOPlusPlus/-/commits/master)

Easy-to-use C++ library for the new Linux GPIO API.

## Features
-  OOP design
-  Elegant event handling
-  Finds your desired pin by name
-  Not limited to a specific device
-  Doesn't look like Arduino APIs :P

## Requirements
-  Linux kernel 4.8+ with new GPIO API & epoll
-  pthread

And reasonably new versions of:
-  C++17 compatible compiler
-  CMake

## Install
Use of the [CPM](https://github.com/TheLartians/CPM.cmake) package manager is recommended.

```cmake
include(cmake/CPM.cmake)

CPMAddPackage(
        NAME GPIOPlusPlus
        GITHUB_REPOSITORY YukiWorkshop/GPIOPlusPlus
        VERSION 0.0.1
)

target_include_directories(your_project PUBLIC ${GPIOPlusPlus_SOURCE_DIR})
target_link_libraries(your_project GPIOPlusPlus pthread)
```

## Usage
```cpp
#include <GPIO++.hpp>

using namespace YukiWorkshop;
```

Get all available devices:

```cpp
std::vector<GPIO::Device> devs = GPIO::all_devices();
```

All operations should be surrounded by `try` and `catch`.
The exception type is `std::system_error` unless otherwise specified.

Open a device by number:
```cpp
try {
    auto d = GPIO::Device(0);
} catch (std::system_error& e) {
    // Error handling here
    std::cerr << "Oops! " << e.what() << "\n";
}
```

...or by path:
```cpp
auto d = GPIO::Device("/dev/gpiochip0");
```

...or by label:
```cpp
try {
    auto d = GPIO::find_device_by_label("pinctrl-bcm2835");
} catch (std::logic_error &e) {
    // No device with this label
}
```

Basic line operations:
```cpp
auto line0 = d.line(0, GPIO::LineMode::Input);
printf("Line 0: %s\n", line0.read() ? "HIGH" : "LOW");

auto line1 = d.line(1, GPIO::LineMode::Output);
line1.write(1);
```

Get a line by its name (won't work if it doesn't have one in device tree):
```cpp
auto line0 = d.line(d.lines_by_name["SDA1"], GPIO::LineMode::Input);
```

Events handling:
```cpp
d.on_event(2, GPIO::LineMode::Input, GPIO::EventMode::RisingEdge,
       [](GPIO::EventType evtype, uint64_t evtime){
           std::cout << "Hey man, your pin is HIGH at " << evtime << "\n";
       }
);

std::thread t([&](){
    d.run_eventlistener();
});
```

And stop them:
```cpp
d.stop_eventlistener();
t.join();
```

No more `digitalWrite`s!! Hurray!!!

## FAQ
#### No pull-up or pull-down?
Currently the standard kernel interface doesn't support them.

See [this discussion](https://patchwork.ozlabs.org/patch/1165565/) for more details.

## Documentation
TBD

## License
LGPLv3