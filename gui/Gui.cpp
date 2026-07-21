#include "Gui.h"
#include "Window.h"
#include "CEWindow.h"
#include "ServerConnectWindow.h"
#include "ModulesWindow.h"
#include "LogWindow.h"
#include "../imgui/imgui.h"
#include <map>
#include <vector>

#ifdef HAVE_LUAJIT
#include "LuaScriptWindow.h"
#endif

namespace Gui {
	std::list<std::unique_ptr<Window>> windows;
	std::list<std::pair<std::string, int>> logs;
	std::mutex logsMutex;

	std::vector<std::pair<std::string, int>> getLogsSnapshot()
	{
		std::lock_guard<std::mutex> lock(logsMutex);
		return {logs.begin(), logs.end()};
	}

	void addWindow(Window* window)
	{
		static std::map<std::string, int> totalWindows;
		if (!window)
			return;

		int& count = totalWindows[window->name];
		if (count > 0)
			window->name = window->name + " " + std::to_string(count + 1);
		count++;

		windows.emplace_back(window);
	}

	bool hasOpenWindows()
	{
		for (const auto& window : windows) {
			if (window && window->pOpen)
				return true;
		}
		return false;
	}

	void mainLoop()
	{
		static bool bootstrapped = false;
		if (!bootstrapped) {
			if (windows.empty()) {
				Gui::addWindow(new CEWindow());
				Gui::addWindow(new ServerConnectWindow());
				Gui::addWindow(new LogWindow());
				
#ifdef HAVE_LUAJIT
				Gui::addWindow(new LuaScriptWindow());
#endif
			}
			Gui::log("欢迎使用 Cheat Turbine！");
			bootstrapped = true;
		}

		for (auto it = windows.begin(); it != windows.end(); ++it)
		{
			Window* w = it->get();
			if (!w) {
				continue;
			}
			// 只绘制打开的窗口，但不删除关闭的窗口（保留状态和指针有效性）
			if (w->pOpen) {
				(*w)();
			}
		}
	}
}
