#pragma once
#include <string>

class PopupHelper {
public:
	static void OpenPopup(const std::string& id);
	static bool ConfirmationDialog(const std::string& id, const std::string& text);
	static bool TextDialog(const std::string& id, std::string& currentText);
};