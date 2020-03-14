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

#include <iostream>
#include <thread>

#include "GPIO++.hpp"

using namespace YukiWorkshop;

int main() {
	auto devs = GPIO::all_devices();

	for (size_t i=0; i<devs.size(); i++) {
		auto &d = devs[i];
		std::cout << "Device " << i << ": name: " << d.name() << " label: " << d.label() << " lines: " << d.num_lines() << "\n";

		for (auto &it : d.lines_by_num()) {
			std::cout << "Line " << it.first << ": name: " << it.second <<  "\n";
		}
	}

	try {
		GPIO::Device d = GPIO::find_device_by_label("INT3450:00");
		std::cout << "Device 0: name: " << d.name() << " label: " << d.label() << " lines: " << d.num_lines()
			  << "\n";

		auto line0 = d.line(0, GPIO::LineMode::Input);
		printf("Line 0: %s\n", line0.read() ? "HIGH" : "LOW");

		auto line1 = d.line(1, GPIO::LineMode::Output);
		line1.write(1);
	} catch (...) {

	}

	try {
		GPIO::Device d = GPIO::find_device_by_label("INT3450:00");
		std::cout << "Device 0: name: " << d.name() << " label: " << d.label() << " lines: " << d.num_lines()
			  << "\n";

		auto line0 = d.line(d.lines_by_name()[""], GPIO::LineMode::Input);
		printf("Line 0: %s\n", line0.read() ? "HIGH" : "LOW");

		auto line1 = d.line(1, GPIO::LineMode::Output);
		line1.write(1);

		d.on_event(2, GPIO::LineMode::Input, GPIO::EventMode::RisingEdge,
			   [](GPIO::EventType evtype, uint64_t evtime){
				   std::cout << "Hey man, somebody is in front of your door.\n";
			   }
		);

		std::thread t([&](){
			d.run_eventlistener();
		});

		d.stop_eventlistener();
		t.join();

	} catch (...) {

	}

	return 0;
}