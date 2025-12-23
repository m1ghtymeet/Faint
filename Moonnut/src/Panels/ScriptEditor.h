#pragma once

#include "../UI/Panels/PanelWindow.h"
#include "TextEditor.h"
#include <sol/sol.hpp>

namespace Moon::Editor {
	class ScriptEditor : public PanelWindow {
	public:
		ScriptEditor(
			const std::string& p_name,
			bool p_opened,
			const PanelWindowSettings& p_windowSettings
		);

        void SetLuaState(sol::state* luaState) { m_luaState = luaState; }
        void LoadFile(const std::string& filepath);
        void SaveFile();
        void SaveFileAs(const std::string& filepath);
        void ReloadSyntax();
        void ExecuteScript();
        bool HasUnsavedChanges() const { return m_textChanged; }
        std::string GetCurrentFile() const { return m_currentFile; }

        // »—«? integration »« ”?” „ ›«?·
        void SetOnSaveCallback(std::function<void(const std::string&)> callback) {
            m_onSaveCallback = callback;
        }

    protected:
        virtual void _Draw_Impl() override;

    private:
        void DrawMenuBar();
        void DrawStatusBar();
        void DrawAutoCompleteSuggestions();
        void SetupEditor();
        void ShowError(const std::string& error);
        void UpdateAutoComplete();
        void HandleShortcuts();

    private:
        TextEditor m_editor;
        sol::state* m_luaState = nullptr;
        std::string m_currentFile;
        bool m_textChanged = false;
        bool m_autoCompleteEnabled = true;
        bool m_syntaxHighlighting = true;
        bool m_showWhitespaces = false;
        bool m_showLineNumbers = true;

        std::function<void(const std::string&)> m_onSaveCallback;

        struct AutoCompleteSuggestion {
            std::string name;
            std::string type; // "function", "variable", "type", "keyword"
            std::string description;
        };
        std::vector<AutoCompleteSuggestion> m_suggestions;

        std::unordered_map<std::string, std::string> m_functionDocs;

        // Error tracking
        std::string m_lastError;
        TextEditor::ErrorMarkers m_errorMarkers;
	};
}