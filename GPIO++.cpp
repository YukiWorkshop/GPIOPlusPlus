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

#include "GPIO++.hpp"

using namespace YukiWorkshop;

std::vector<GPIO::Device> GPIO::all_devices() {
	std::vector<GPIO::Device> ret;

	for (uint32_t i=0; i<UINT32_MAX; i++) {
		GPIO::Device d;
		try {
			d.open(i);
			ret.emplace_back(std::move(d));
		} catch (...) {
			break;
		}
	}

	return ret;
}

GPIO::Device GPIO::find_device_by_name(const std::string& __name) {
	auto devs = all_devices();

	for (auto &it : devs) {
		if (it.name() == __name)
			return it;
	}

	throw std::logic_error("no device with this name");
}

GPIO::Device GPIO::find_device_by_label(const std::string& __label) {
	auto devs = all_devices();

	for (auto &it : devs) {
		if (it.label() == __label)
			return devs[0];
	}

	throw std::logic_error("no device with this label");
}

void GPIO::Device::get_device_info() {
	gpiochip_info cinfo;

	if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &cinfo)) {
		throw ExceptionWithErrno("failed to get device info");
	}

	name_ = cinfo.name;
	label_ = cinfo.label;
	lines_ = cinfo.lines;
}

void GPIO::Device::open(const std::string &__path) {
	if ((fd = ::open(__path.c_str(), O_RDWR)) == -1)
		throw ExceptionWithErrno("failed to open device");

	get_device_info();
	path_ = __path;
}

void GPIO::Device::open(uint32_t __id) {
	open(Utils::make_device_path(__id));
}

GPIO::LineSingle
GPIO::Device::line(uint32_t __line_number, GPIO::LineMode __mode, uint8_t __default_value, const std::string &__label) {
	gpiohandle_request req{};

	req.lineoffsets[0] = __line_number;
	req.default_values[0] = __default_value;
	req.flags = (uint32_t)__mode;
	strncpy(req.consumer_label, __label.c_str(), 31);
	req.lines = 1;

	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req))
		throw ExceptionWithErrno("failed to get line handle");

	return LineSingle(req.fd, 1);
}

GPIO::LineMultiple
GPIO::Device::line(const std::initializer_list<LineSpec> &__lss, GPIO::LineMode __mode, const std::string &__label) {
	gpiohandle_request req{};

	uint8_t usable_size = __lss.size() > 64 ? 64 : __lss.size();

	for (uint8_t i=0; i<usable_size; i++) {
		req.lineoffsets[i] = (__lss.begin()+i)->line_number;
		req.default_values[i] = (__lss.begin()+i)->default_value;
	}

	strncpy(req.consumer_label, __label.c_str(), 31);
	req.flags = (uint32_t)__mode;
	req.lines = usable_size;

	if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req))
		throw ExceptionWithErrno("failed to get line handle");

	return LineMultiple(req.fd, usable_size);
}

int GPIO::Device::on_event(uint32_t __line_number, GPIO::LineMode __line_mode, GPIO::EventMode __event_mode,
			   const std::function<void(EventType, uint64_t)>& __handler, const std::string &__label) {
	gpioevent_request req{};
	req.lineoffset = __line_number;
	req.handleflags = (uint32_t)__line_mode;
	req.eventflags = (uint32_t)__event_mode;
	strncpy(req.consumer_label, __label.c_str(), 31);

	if (!ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req)) {
		throw ExceptionWithErrno("failed to setup events");
	}

	events_map.insert({req.fd, __handler});
	if (epfd > 0) {
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = req.fd;

		epoll_ctl(epfd, EPOLL_CTL_ADD, req.fd, &ev);
	}

	return req.fd;
}

void GPIO::Device::cancel_event(int __event_handle) {
	if (epfd > 0)
		epoll_ctl(epfd, EPOLL_CTL_DEL, __event_handle, nullptr);

	events_map.erase(__event_handle);
}

void GPIO::Device::run_eventlistener() {
	eventlistener_run = true;

	epfd = epoll_create(42);

	for (auto &it : events_map) {
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = it.first;

		epoll_ctl(epfd, EPOLL_CTL_ADD, it.first, &ev);
	}

	int ep_rc;
	epoll_event evs[16];

	while ((ep_rc = epoll_wait(epfd, evs, 16, 1000)) != -1) {
		if (ep_rc > 0) {
			for (uint i=0; i<ep_rc; i++) {
				int cur_fd = evs[i].data.fd;

				gpioevent_data event;
				auto it = events_map.find(cur_fd);
				if (it != events_map.end() ||
				    read(cur_fd, &event, sizeof(gpioevent_data)) == sizeof(gpioevent_data))
					events_map[cur_fd]((EventType)event.id, event.timestamp);

			}
		}

		if (!eventlistener_run) {
			close(epfd);
			epfd = -1;
			break;
		}
	}

}

void GPIO::Device::stop_eventlistener() {
	eventlistener_run = false;
}

uint8_t GPIO::LineSingle::read() {
	gpiohandle_data data{};

	if (ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data))
		throw ExceptionWithErrno("failed to read value from line");

	return data.values[0];
}

void GPIO::LineSingle::write(uint8_t __value) {
	gpiohandle_data data{};
	data.values[0] = __value;

	if (ioctl(fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data))
		throw ExceptionWithErrno("failed to write value to line");
}

std::vector<uint8_t> GPIO::LineMultiple::read() {
	std::vector<uint8_t> ret(sizeof(gpiohandle_data));

	if (ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, ret.data()))
		throw ExceptionWithErrno("failed to read values from lines");

	ret.resize(size);
	return ret;
}

void GPIO::LineMultiple::write(const std::vector<uint8_t> &__values) {
	if (ioctl(fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, __values.data()))
		throw ExceptionWithErrno("failed to write values to lines");
}
