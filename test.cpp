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
#include "GPIO++.hpp"

using namespace YukiWorkshop;

int main() {
	auto devs = GPIO::all_devices();

	for (size_t i=0; i<devs.size(); i++) {
		auto &d = devs[i];
		std::cout << "Device " << i << ": name: " << d.name() << " label: " << d.label() << " lines: " << d.lines() << "\n";
	}

	try {
		GPIO::Device d = GPIO::find_device_by_label("INT3450:00");
		std::cout << "Device 0: name: " << d.name() << " label: " << d.label() << " lines: " << d.lines()
			  << "\n";

		auto line0 = d.line(1, GPIO::LineMode::Input);
		printf("Line 1: %s\n", line0.read() ? "HIGH" : "LOW");

		auto line1 = d.line(1, GPIO::LineMode::Output);
		line1.write(1);
	} catch (...) {

	}

	return 0;
}