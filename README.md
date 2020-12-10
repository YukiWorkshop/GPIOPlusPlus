# GPIO++
[![pipeline status](https://gitlab.com/ReimuNotMoe/GPIOPlusPlus/badges/master/pipeline.svg)](https://gitlab.com/ReimuNotMoe/GPIOPlusPlus/-/commits/master)

Easy-to-use C++ library for the new Linux GPIO API.

## Features
-  OOP design
-  Elegant event handling: automatic or manual
-  Finds your desired pin by name (need DT/ACPI support)
-  Not limited to a specific hardware platform
-  Thread safe
-  **Doesn't look like Arduino APIs** :P

## Requirements
-  Linux kernel 4.8+ with new GPIO API (`/dev/gpiochipX`) & epoll
-  **Linux kernel 5.4+ to make pullup/pulldown actually working**

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
        VERSION 0.0.4
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
auto line0 = d.line(0, GPIO::LineMode::Input | GPIO::LineMode::PullUp);
printf("Line 0: %s\n", line0.read() ? "HIGH" : "LOW");

auto line1 = d.line(1, GPIO::LineMode::Output);
line1.write(1);
```

Get a line by its name (won't work if it doesn't have one in device tree):
```cpp
auto line0 = d.line(d.lines_by_name["SDA1"], GPIO::LineMode::Input);
```

Add events:
```cpp
int handle = d.add_event(2, GPIO::LineMode::Input, GPIO::EventMode::RisingEdge,
               [](GPIO::EventType evtype, uint64_t evtime){
                   std::cout << "Hey man, your pin is HIGH at " << evtime << "\n";
               }
);
```

And remove them:
```cpp
d.remove_event(handle);
```

Automatic events handling:
```cpp
std::thread t([&](){
    d.run_eventlistener();
});
```

And stop them:
```cpp
d.stop_eventlistener();
t.join();
```

Manual events handling:
```cpp
int epfd = epoll_create(42);

epoll_event ev;

for (auto &it : d.event_fds()) {
    ev.events = EPOLLIN;
    ev.data.fd = it;

    epoll_ctl(epfd, EPOLL_CTL_ADD, it, &ev);
}

int ep_rc;
epoll_event evs[16];

while ((ep_rc = epoll_wait(epfd, evs, 16, 1000)) != -1) {
    if (ep_rc > 0) {
        for (uint i=0; i<ep_rc; i++)
            d.process_event(evs[i].data.fd);
    }

    // ...
}
```

No more `digitalWrite`s!! Hurray!!!!!!

## License
LGPLv3