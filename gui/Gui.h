#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <cstdarg>
#include <cstdio>

class Window;

namespace Gui {
	extern std::list<std::unique_ptr<Window>> windows;
	extern std::list<std::pair<std::string, int>> logs;
	extern std::mutex logsMutex;

	void mainLoop();
	bool hasOpenWindows();
	std::vector<std::pair<std::string, int>> getLogsSnapshot();

	inline void log(const char* fmt, ...) {
		char small_buf[512];
		va_list args;
		va_start(args, fmt);
		int needed = std::vsnprintf(small_buf, sizeof(small_buf), fmt, args);
		va_end(args);
		std::string str;
		if (needed < 0) {
			str = "<format error>";
		} else if (static_cast<size_t>(needed) < sizeof(small_buf)) {
			str.assign(small_buf, small_buf + needed);
		} else {
			std::vector<char> buf(static_cast<size_t>(needed) + 1);
			va_list args2;
			va_start(args2, fmt);
			std::vsnprintf(buf.data(), buf.size(), fmt, args2);
			va_end(args2);
			str.assign(buf.data(), buf.data() + needed);
		}
		std::lock_guard<std::mutex> lock(logsMutex);
		if (!logs.empty() && str == logs.back().first)
			logs.back().second++;
		else
			logs.emplace_back(std::move(str), 0);
	}

	void addWindow(Window* window);

	template <typename T>
	std::list<T*> getWindows() {
		std::list<T*> res;
		for (const std::unique_ptr<Window>& window: windows)
			if (const auto tWindow = dynamic_cast<T*>(window.get()))
				res.push_back(tWindow);
		return res;
	}

	template<typename T, typename... Args>
	T* getOrCreate(Args&&... args) {
		auto list = getWindows<T>();
		if (!list.empty()) {
			T* w = list.front();
			w->pOpen = true;
			w->shouldBringToFront = true;
			return w;
		}
		T* w = new T(std::forward<Args>(args)...);
		addWindow(w);
		w->shouldBringToFront = true;
		return w;
	}
}
