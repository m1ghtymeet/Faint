#include "ScriptEditor.h"
#include <Debug/Log.h>
#include <FileSystem/FileSystem.h>
#include <fstream>

namespace Moon::Editor {

    class LuaSyntaxHighlighter {
    public:
        static void ApplyLuaSyntax(TextEditor& editor) {
            TextEditor::LanguageDefinition lang = TextEditor::LanguageDefinition::Lua();

            // Customize for our engine's specific APIs
            SetupEngineKeywords(lang);
            SetupEngineIdentifiers(lang);
            SetupEngineRegex(lang);

            editor.SetLanguageDefinition(lang);
        }

        static void UpdateFromLuaState(TextEditor& editor, sol::state& luaState) {
            auto lang = editor.GetLanguageDefinition();
            UpdateIdentifiersFromState(lang, luaState);
            editor.SetLanguageDefinition(lang);
        }

    private:
        static void SetupEngineKeywords(TextEditor::LanguageDefinition& lang) {
            // Lua keywords
            lang.mKeywords = {
                "and", "break", "do", "else", "elseif", "end",
                "false", "for", "function", "goto", "if", "in",
                "local", "nil", "not", "or", "repeat", "return",
                "then", "true", "until", "while"
            };
        }

        static void SetupEngineIdentifiers(TextEditor::LanguageDefinition& lang) {
            // Engine-specific functions and types
            lang.mIdentifiers.clear();
            lang.mPreprocIdentifiers.clear();

            // Engine types
            AddIdentifier(lang, "Vector3", "3D vector type");
            AddIdentifier(lang, "Matrix4", "4x4 matrix type");
            AddIdentifier(lang, "Entity", "Game entity");
            AddIdentifier(lang, "Scene", "Game scene");
            AddIdentifier(lang, "Transform", "Transform component");
            AddIdentifier(lang, "RigidBody", "Rigidbody component");
            AddIdentifier(lang, "BoxCollider", "Box collider component");
            AddIdentifier(lang, "CharacterController", "Character controller component");
            AddIdentifier(lang, "AudioSource", "Audio source component");
            AddIdentifier(lang, "Text2D", "2D text component");
            AddIdentifier(lang, "Camera", "Camera component");
            AddIdentifier(lang, "Component", "Base component type");

            // Math functions
            AddIdentifier(lang, "Lerp", "Linear interpolation between two values");
            AddIdentifier(lang, "SmoothStep", "Smooth interpolation");
            AddIdentifier(lang, "Clamp", "Clamp value between min and max");
            AddIdentifier(lang, "MoveTowards", "Move towards target value");
            AddIdentifier(lang, "FInterpTo", "Frame rate independent interpolation");
            AddIdentifier(lang, "FInterpConstantTo", "Constant rate interpolation");

            // Vector3 operations
            AddIdentifier(lang, "length", "Get vector length");
            AddIdentifier(lang, "normalized", "Get normalized vector");
            AddIdentifier(lang, "normalize", "Normalize vector");
            AddIdentifier(lang, "dot", "Dot product");
            AddIdentifier(lang, "cross", "Cross product");
            AddIdentifier(lang, "distance", "Distance between two vectors");
            AddIdentifier(lang, "distanceSquared", "Squared distance between two vectors");

            // Entity operations
            AddIdentifier(lang, "GetName", "Get entity name");
            AddIdentifier(lang, "SetName", "Set entity name");
            AddIdentifier(lang, "GetTransform", "Get transform component");
            AddIdentifier(lang, "GetRigidBody", "Get rigidbody component");
            AddIdentifier(lang, "AddForce", "Apply force to rigidbody");
            AddIdentifier(lang, "AddTorque", "Apply torque to rigidbody");
            AddIdentifier(lang, "SetVelocity", "Set rigidbody velocity");
            AddIdentifier(lang, "GetVelocity", "Get rigidbody velocity");
            AddIdentifier(lang, "GetComponent", "Get component by type");
            AddIdentifier(lang, "AddComponent", "Add component to entity");
            AddIdentifier(lang, "RemoveComponent", "Remove component from entity");

            // Scene operations
            AddIdentifier(lang, "GetEntityByName", "Find entity by name");
            AddIdentifier(lang, "GetEntityByTag", "Find entity by tag");
            AddIdentifier(lang, "CreateEntity", "Create new entity");
            AddIdentifier(lang, "FindEntitiesWithTag", "Find all entities with tag");

            // Constants
            AddPreprocIdentifier(lang, "PI", "Pi constant");
            AddPreprocIdentifier(lang, "TAU", "Tau constant (2*PI)");
            AddPreprocIdentifier(lang, "DEG2RAD", "Degrees to radians conversion");
            AddPreprocIdentifier(lang, "RAD2DEG", "Radians to degrees conversion");
            AddPreprocIdentifier(lang, "EPSILON", "Floating point epsilon");
        }

        static void SetupEngineRegex(TextEditor::LanguageDefinition& lang) {
            // Add custom regex patterns for better tokenization
            lang.mTokenRegexStrings.clear();

            // Numbers (including scientific notation)
            lang.mTokenRegexStrings.push_back(std::make_pair(
                R"([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)",
                TextEditor::PaletteIndex::Number
            ));

            // Single line comments
            lang.mTokenRegexStrings.push_back(std::make_pair(
                R"(--[^\n]*)",
                TextEditor::PaletteIndex::Comment
            ));

            // Strings (single and double quoted)
            lang.mTokenRegexStrings.push_back(std::make_pair(
                R"("(?:\\.|[^"\\])*")",
                TextEditor::PaletteIndex::String
            ));

            lang.mTokenRegexStrings.push_back(std::make_pair(
                R"('(?:\\.|[^'\\])*')",
                TextEditor::PaletteIndex::String
            ));

            // Preprocessor (constants)
            lang.mTokenRegexStrings.push_back(std::make_pair(
                R"(\b[A-Z][A-Z0-9_]+\b)",
                TextEditor::PaletteIndex::Preprocessor
            ));
        }

        static void AddIdentifier(TextEditor::LanguageDefinition& lang, const std::string& name, const std::string& declaration = "") {
            TextEditor::Identifier id;
            id.mDeclaration = declaration;
            lang.mIdentifiers[name] = id;
        }

        static void AddPreprocIdentifier(TextEditor::LanguageDefinition& lang, const std::string& name, const std::string& declaration = "") {
            TextEditor::Identifier id;
            id.mDeclaration = declaration;
            lang.mPreprocIdentifiers[name] = id;
        }

        static void UpdateIdentifiersFromState(TextEditor::LanguageDefinition& lang, sol::state& luaState) {
            // Extract global variables and functions from Lua state
            sol::table globals = luaState.globals();

            for (auto& pair : globals) {
                if (pair.first.is<std::string>()) {
                    std::string name = pair.first.as<std::string>();

                    // Skip internal Lua globals
                    if (name.find("_") == 0) continue;
                    if (name == "string" || name == "table" || name == "math" ||
                        name == "io" || name == "os" || name == "debug") continue;

                    // Add to appropriate category based on type
                    if (pair.second.is<sol::function>()) {
                        AddIdentifier(lang, name, "Lua function");
                    }
                    else if (pair.second.is<sol::table>()) {
                        AddIdentifier(lang, name, "Lua table");
                    }
                    else {
                        AddPreprocIdentifier(lang, name, "Global variable");
                    }
                }
            }
        }
    };
}

Moon::Editor::ScriptEditor::ScriptEditor(const std::string& p_name, bool p_opened, const PanelWindowSettings& p_windowSettings) :
	PanelWindow(p_name, p_opened, p_windowSettings) {
	SetupEditor();

    m_functionDocs = {
            {"Vector3", "3D vector with x, y, z components\nUsage: local v = Vector3(1, 2, 3)"},
            {"Lerp", "Linearly interpolates between two values\nUsage: Lerp(a, b, t)"},
            {"GetTransform", "Gets the transform component of an entity\nUsage: local transform = entity:GetTransform()"},
            {"AddForce", "Applies force to a rigidbody\nUsage: rigidbody:AddForce(Vector3(0, 10, 0))"},
            {"CreateEntity", "Creates a new entity in the scene\nUsage: local entity = scene:CreateEntity('NewEntity')"},
            {"GetEntityByName", "Finds entity by name\nUsage: local entity = scene:GetEntityByName('Player')"},
            {"SetVelocity", "Sets rigidbody velocity\nUsage: rigidbody:SetVelocity(Vector3(5, 0, 0))"},
            {"Play", "Plays audio source\nUsage: audioSource:Play()"},
            {"MoveTowards", "Moves current towards target\nUsage: MoveTowards(current, target, maxDelta)"}
    };
}

void Moon::Editor::ScriptEditor::SetupEditor() {
    //FT_PROFILE_FUNCTION();

    // Apply Lua syntax highlighting
    LuaSyntaxHighlighter::ApplyLuaSyntax(m_editor);

    // Set editor options
    m_editor.SetShowWhitespaces(m_showWhitespaces);
    m_editor.SetTabSize(4);
    m_editor.SetReadOnly(false);
    m_editor.SetColorizerEnable(m_syntaxHighlighting);

    // Set custom palette for better visibility
    auto palette = m_editor.GetDarkPalette();
    // Customize colors if needed
    m_editor.SetPalette(palette);

    UpdateAutoComplete();
}

void Moon::Editor::ScriptEditor::LoadFile(const std::string& filepath) {
    //FT_PROFILE_FUNCTION();

    if (!FileSystem::FileExists(filepath, true)) {
        HZ_CORE_ERROR("Lua file not found: {}", filepath);
        return;
    }

    try {
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            m_editor.SetText(buffer.str());
            m_currentFile = filepath;
            m_textChanged = false;
            m_errorMarkers.clear();
            m_editor.SetErrorMarkers(m_errorMarkers);
            HZ_CORE_INFO("Loaded Lua script: {}", filepath);
        }
    }
    catch (const std::exception& e) {
        HZ_CORE_ERROR("Failed to load Lua file {}: {}", filepath, e.what());
    }
}

void Moon::Editor::ScriptEditor::SaveFile() {
    //FT_PROFILE_FUNCTION();

    if (m_currentFile.empty()) {
        HZ_CORE_WARN("No file path specified for saving");
        return;
    }

    try {
        std::ofstream file(m_currentFile);
        if (file.is_open()) {
            file << m_editor.GetText();
            m_textChanged = false;

            if (m_onSaveCallback) {
                m_onSaveCallback(m_currentFile);
            }

            HZ_CORE_INFO("Saved Lua script: {}", m_currentFile);
        }
    }
    catch (const std::exception& e) {
        HZ_CORE_ERROR("Failed to save Lua file {}: {}", m_currentFile, e.what());
    }
}

void Moon::Editor::ScriptEditor::SaveFileAs(const std::string& filepath) {
    m_currentFile = filepath;
    SaveFile();
}

void Moon::Editor::ScriptEditor::ReloadSyntax() {
    if (m_luaState) {
        LuaSyntaxHighlighter::UpdateFromLuaState(m_editor, *m_luaState);
    }
    UpdateAutoComplete();
}

void Moon::Editor::ScriptEditor::ExecuteScript() {
    //FT_PROFILE_FUNCTION();

    if (!m_luaState) {
        HZ_CORE_ERROR("No Lua state available for script execution");
        return;
    }

    // Clear previous errors
    m_errorMarkers.clear();
    m_lastError.clear();

    try {
        std::string script = m_editor.GetText();
        auto result = m_luaState->script(script);
        HZ_CORE_INFO("Lua script executed successfully");
    }
    catch (const sol::error& e) {
        m_lastError = e.what();
        HZ_CORE_ERROR("Lua script error: {}", m_lastError);

        // Try to parse error line number
        // Lua errors typically look like: [string "chunk"]:line: message
        std::regex errorPattern(R"(:(\d+):)");
        std::smatch matches;
        if (std::regex_search(m_lastError, matches, errorPattern) && matches.size() > 1) {
            try {
                int lineNumber = std::stoi(matches[1].str()) - 1; // Convert to 0-based
                m_errorMarkers[lineNumber] = m_lastError;
                m_editor.SetErrorMarkers(m_errorMarkers);
            }
            catch (...) {
                // If line number parsing fails, just show the error
            }
        }

        ShowError(m_lastError);
    }
}

void Moon::Editor::ScriptEditor::_Draw_Impl() {
    //FT_PROFILE_FUNCTION();

    HandleShortcuts();
    DrawMenuBar();

    // Editor content
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    m_editor.Render("LuaEditor", availableSize, true);

    // Check for text changes
    if (m_editor.IsTextChanged() && !m_textChanged) {
        m_textChanged = true;
    }

    DrawAutoCompleteSuggestions();
    DrawStatusBar();
}

void Moon::Editor::ScriptEditor::HandleShortcuts() {
    // Handle keyboard shortcuts
    ImGuiIO& io = ImGui::GetIO();

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        if (!m_currentFile.empty()) {
            SaveFile();
        }
    }

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
        // TODO: Open file dialog
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
        ExecuteScript();
    }
}

void Moon::Editor::ScriptEditor::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // TODO: File open dialog
            }
            if (ImGui::MenuItem("Save", "Ctrl+S", false, !m_currentFile.empty())) {
                SaveFile();
            }
            if (ImGui::MenuItem("Save As...")) {
                // TODO: File save dialog
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close")) {
                SetOpened(false);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, m_editor.CanUndo())) {
                m_editor.Undo();
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, m_editor.CanRedo())) {
                m_editor.Redo();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                m_editor.Cut();
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                m_editor.Copy();
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                m_editor.Paste();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {
                m_editor.SelectAll();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Line Numbers", nullptr, &m_showLineNumbers)) {
                // TextEditor شما این feature رو داره
            }
            if (ImGui::MenuItem("Syntax Highlighting", nullptr, &m_syntaxHighlighting)) {
                m_editor.SetColorizerEnable(m_syntaxHighlighting);
            }
            if (ImGui::MenuItem("Whitespaces", nullptr, &m_showWhitespaces)) {
                m_editor.SetShowWhitespaces(m_showWhitespaces);
            }
            if (ImGui::MenuItem("Auto Complete", nullptr, &m_autoCompleteEnabled)) {
                // Handle auto-complete toggle
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Script")) {
            if (ImGui::MenuItem("Execute", "F5")) {
                ExecuteScript();
            }
            if (ImGui::MenuItem("Reload Syntax")) {
                ReloadSyntax();
            }
            ImGui::EndMenu();
        }

        // Show error indicator if there's an error
        if (!m_lastError.empty()) {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 150);
            ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Error");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(m_lastError.c_str());
                ImGui::EndTooltip();
            }
        }

        ImGui::EndMenuBar();
    }
}

void Moon::Editor::ScriptEditor::DrawAutoCompleteSuggestions() {
    if (!m_autoCompleteEnabled) return;

    // Get current cursor position and text
    auto cursorPos = m_editor.GetCursorPosition();
    auto lineText = m_editor.GetCurrentLineText();

    // Simple auto-complete trigger
    if (!lineText.empty() && cursorPos.mColumn > 0) {
        size_t charIndex = static_cast<size_t>(cursorPos.mColumn - 1);
        if (charIndex < lineText.length()) {
            char prevChar = lineText[charIndex];

            if (prevChar == '.' || (isalpha(prevChar) && cursorPos.mColumn >= 2)) {
                // Show suggestions popup
                if (ImGui::BeginPopupContextWindow("AutoComplete")) {
                    ImGui::Text("Suggestions");
                    ImGui::Separator();

                    for (const auto& suggestion : m_suggestions) {
                        ImGui::PushID(suggestion.name.c_str());

                        // Different icons for different types
                        const char* icon = "Cube";
                        if (suggestion.type == "function") icon = "Function";
                        else if (suggestion.type == "variable") icon = "Variable";
                        else if (suggestion.type == "keyword") icon = "Key";

                        std::string displayText = std::string(icon) + " " + suggestion.name;

                        if (ImGui::Selectable(displayText.c_str())) {
                            m_editor.InsertText(suggestion.name);
                        }

                        if (ImGui::IsItemHovered() && !suggestion.description.empty()) {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted(suggestion.description.c_str());
                            ImGui::EndTooltip();
                        }

                        ImGui::PopID();
                    }

                    ImGui::EndPopup();
                }
            }
        }
    }
}

void Moon::Editor::ScriptEditor::DrawStatusBar() {
    ImGui::Separator();

    auto cursorPos = m_editor.GetCursorPosition();

    ImGui::BeginGroup();
    ImGui::Text("Line %d, Column %d", cursorPos.mLine + 1, cursorPos.mColumn + 1);
    ImGui::SameLine();
    ImGui::Text("| %s", m_currentFile.empty() ? "Unsaved" : m_currentFile.c_str());
    ImGui::SameLine();
    ImGui::Text("| %s", m_textChanged ? "Modified" : "Saved");
    ImGui::EndGroup();

    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    if (ImGui::Button("Run")) {
        ExecuteScript();
    }

    // Show error in status bar if exists
    if (!m_lastError.empty()) {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Error: %s", m_lastError.c_str());
    }
}

void Moon::Editor::ScriptEditor::UpdateAutoComplete() {
    m_suggestions.clear();

    auto& lang = m_editor.GetLanguageDefinition();

    // Add keywords
    for (const auto& keyword : lang.mKeywords) {
        m_suggestions.push_back({ keyword, "keyword", "Lua keyword" });
    }

    // Add identifiers (functions and types)
    for (const auto& [name, identifier] : lang.mIdentifiers) {
        std::string description = m_functionDocs.count(name)
            ? m_functionDocs[name]
            : identifier.mDeclaration;
        m_suggestions.push_back({ name, "function", description });
    }

    // Add preprocessor identifiers (constants)
    for (const auto& [name, identifier] : lang.mPreprocIdentifiers) {
        m_suggestions.push_back({ name, "variable", identifier.mDeclaration });
    }

    // Sort alphabetically
    std::sort(m_suggestions.begin(), m_suggestions.end(),
        [](const AutoCompleteSuggestion& a, const AutoCompleteSuggestion& b) {
            return a.name < b.name;
        });
}

void Moon::Editor::ScriptEditor::ShowError(const std::string& error) {
    // Error is already stored in m_lastError and shown in UI
    // You could add additional error reporting here
}