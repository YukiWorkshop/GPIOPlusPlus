/*
    This file is part of GPIO++.
    Copyright (C) 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>
#include <unordered_map>
#include <map>
#include <functional>
#include <shared_mutex>
#include <stdexcept>
#include <system_error>

#include <cstring>
#include <cinttypes>

#include <fcntl.h>
#include <unistd.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/version.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>

#include "Utils.hpp"

#ifndef GPIOHANDLE_REQUEST_BIAS_DISABLE
#define GPIOHANDLE_REQUEST_BIAS_DISABLE 0
#endif

#ifndef GPIOHANDLE_REQUEST_BIAS_PULL_UP
#define GPIOHANDLE_REQUEST_BIAS_PULL_UP 0
#endif

#ifndef GPIOHANDLE_REQUEST_BIAS_PULL_DOWN
#define GPIOHANDLE_REQUEST_BIAS_PULL_DOWN 0
#endif

namespace YukiWorkshop::GPIO {
	class Device;
	class Line;

	enum class LineMode : int {
		Input = GPIOHANDLE_REQUEST_INPUT,
		Output = GPIOHANDLE_REQUEST_OUTPUT,
		ActiveLow = GPIOHANDLE_REQUEST_ACTIVE_LOW,
		OpenDrain = GPIOHANDLE_REQUEST_OPEN_DRAIN,
		OpenSource = GPIOHANDLE_REQUEST_OPEN_SOURCE,
		NoPull = GPIOHANDLE_REQUEST_BIAS_DISABLE,
		PullUp = GPIOHANDLE_REQUEST_BIAS_PULL_UP,
		PullDown = GPIOHANDLE_REQUEST_BIAS_PULL_DOWN,
	};

	enum class EventMode : int {
		RisingEdge = GPIOEVENT_REQUEST_RISING_EDGE,
		FallingEdge = GPIOEVENT_REQUEST_FALLING_EDGE,
		Both = RisingEdge | FallingEdge
	};

	enum class EventType : int {
		RisingEdge = GPIOEVENT_EVENT_RISING_EDGE,
		FallingEdge = GPIOEVENT_EVENT_FALLING_EDGE,
		Both = RisingEdge | FallingEdge
	};

	inline constexpr LineMode operator&(LineMode x, LineMode y) {
		return static_cast<LineMode>(static_cast<int>(x) & static_cast<int>(y));
	}

	inline constexpr LineMode operator|(LineMode x, LineMode y) {
		return static_cast<LineMode>(static_cast<int>(x) | static_cast<int>(y));
	}

	inline constexpr LineMode operator^(LineMode x, LineMode y) {
		return static_cast<LineMode>(static_cast<int>(x) ^ static_cast<int>(y));
	}

	inline constexpr LineMode operator~(LineMode x) {
		return static_cast<LineMode>(~static_cast<int>(x));
	}

	inline LineMode & operator&=(LineMode & x, LineMode y) {
		x = x & y;
		return x;
	}

	inline LineMode & operator|=(LineMode & x, LineMode y) {
		x = x | y;
		return x;
	}

	inline LineMode & operator^=(LineMode & x, LineMode y) {
		x = x ^ y;
		return x;
	}

	inline constexpr EventMode operator&(EventMode x, EventMode y) {
		return static_cast<EventMode>(static_cast<int>(x) & static_cast<int>(y));
	}

	inline constexpr EventMode operator|(EventMode x, EventMode y) {
		return static_cast<EventMode>(static_cast<int>(x) | static_cast<int>(y));
	}

	inline constexpr EventMode operator^(EventMode x, EventMode y) {
		return static_cast<EventMode>(static_cast<int>(x) ^ static_cast<int>(y));
	}

	inline constexpr EventMode operator~(EventMode x) {
		return static_cast<EventMode>(~static_cast<int>(x));
	}

	inline EventMode & operator&=(EventMode & x, EventMode y) {
		x = x & y;
		return x;
	}

	inline EventMode & operator|=(EventMode & x, EventMode y) {
		x = x | y;
		return x;
	}

	inline EventMode & operator^=(EventMode & x, EventMode y) {
		x = x ^ y;
		return x;
	}

	inline constexpr EventType operator&(EventType x, EventType y) {
		return static_cast<EventType>(static_cast<int>(x) & static_cast<int>(y));
	}

	inline constexpr EventType operator|(EventType x, EventType y) {
		return static_cast<EventType>(static_cast<int>(x) | static_cast<int>(y));
	}

	inline constexpr EventType operator^(EventType x, EventType y) {
		return static_cast<EventType>(static_cast<int>(x) ^ static_cast<int>(y));
	}

	inline constexpr EventType operator~(EventType x) {
		return static_cast<EventType>(~static_cast<int>(x));
	}

	inline EventType & operator&=(EventType & x, EventType y) {
		x = x & y;
		return x;
	}

	inline EventType & operator|=(EventType & x, EventType y) {
		x = x | y;
		return x;
	}

	inline EventType & operator^=(EventType & x, EventType y) {
		x = x ^ y;
		return x;
	}

	struct LineSpec {
		uint32_t line_number;
		uint8_t default_value;
	};

	class Line {
	protected:
		int fd = -1;
		uint8_t size = 0;
	public:
		Line() = default;

		Line(int __fd, size_t __size) : fd(__fd), size(__size) {}

		virtual ~Line() {
			close(fd);
		}
	};

	class LineSingle : public Line {
	private:
		int pfd = -1;
		uint32_t offset_ = 0;
		std::string name_, label_;
	public:
		LineSingle() = default;

		LineSingle(int __fd, int __pfd, size_t __size, const gpioline_info& __info) : Line(__fd, __size) {
			pfd = __pfd;
			offset_ = __info.line_offset;
			name_ = __info.name;
			label_ = __info.consumer;
		}

		LineSingle(const LineSingle& other) {
			fd = dup(other.fd);
			size = other.size;
			pfd = other.pfd;
			name_ = other.name_;
			label_ = other.label_;
		}

		LineSingle& operator=(const LineSingle& other) {
			fd = dup(other.fd);
			size = other.size;
			pfd = other.pfd;
			name_ = other.name_;
			label_ = other.label_;

			return *this;
		}

		bool debug = false;

		const std::string& name() const noexcept {
			return name_;
		}

		const std::string& label() const noexcept {
			return label_;
		}

		uint32_t number() const noexcept {
			return offset_;
		}

		LineMode mode() const;

		void set_mode(LineMode __mode, uint8_t __default_value = 0, const std::string& __label = "");

		uint8_t read();
		void write(uint8_t __value);
	};

	class LineMultiple : public Line {
	public:
		LineMultiple() = default;

		LineMultiple(int __fd, size_t __size) : Line(__fd, __size) {}

		LineMultiple(const LineMultiple& other) {
			fd = dup(other.fd);
			size = other.size;
		}

		LineMultiple& operator=(const LineMultiple& other) {
			fd = dup(other.fd);
			size = other.size;

			return *this;
		}

		std::vector<uint8_t> read();
		void write(const std::vector<uint8_t>& __values);
	};

	class Device {
	private:
		int fd = -1;
		int epfd = -1;
		bool eventlistener_run = false;

		std::string path_;
		std::string name_, label_;
		uint32_t num_lines_ = 0;
		std::shared_mutex event_lock;

		std::map<uint32_t, std::string> lines_by_num_;
		std::map<std::string, uint32_t> lines_by_name_;

		std::unordered_map<int, std::function<void(EventType, uint64_t)>> events_map;

		void get_device_info();

	public:
		Device() = default;

		bool debug = false;

		explicit Device(uint32_t __id) {
			open(__id);
		}

		explicit Device(const std::string& __path) {
			open(__path);
		}

		~Device() {
			if (fd > 0)
				close(fd);
			if (epfd > 0)
				close(epfd);
		}

		Device& operator=(const Device& other) {
			open(other.path_);
			return *this;
		}

		Device(const Device& other) {
			open(other.path_);
		}

		const std::string& name() const noexcept {
			return name_;
		}

		const std::string& label() const noexcept {
			return label_;
		}

		const std::string& path() const noexcept {
			return path_;
		}

		uint32_t num_lines() const noexcept {
			return num_lines_;
		}

		std::map<uint32_t, std::string>& lines_by_num();
		std::map<std::string, uint32_t>& lines_by_name();

		void open(const std::string& __path);
		void open(uint32_t __id);

		LineSingle line(uint32_t __line_number, LineMode __mode, uint8_t __default_value = 0, const std::string& __label = "");
		LineMultiple line(const std::initializer_list<LineSpec>& __lss, LineMode __mode, const std::string& __label = "");

		int add_event(uint32_t __line_number, LineMode __line_mode, EventMode __event_mode,
			      const std::function<void(EventType, uint64_t)>& __handler, const std::string& __label = "");

		void remove_event(int __event_handle);

		void process_event(int __event_handle);

		std::vector<int> event_fds();

		bool is_event_fd(int __fd);

		void run_eventlistener();
		void stop_eventlistener();
	};

	extern std::vector<Device> all_devices();
	extern GPIO::Device find_device_by_label(const std::string& __label);
	extern GPIO::Device find_device_by_name(const std::string& __name);
}