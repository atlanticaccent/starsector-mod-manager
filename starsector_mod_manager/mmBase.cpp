#include "mmBase.h"

mmBase::mmBase() : wxFrame(nullptr, wxID_ANY, "Starsector Mod Manager", wxDefaultPosition, wxSize(800, 600)) {
    mainPane = new wxPanel(this);
    mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    m_ctrl = new wxDataViewListCtrl(mainPane, MM_DATA_LIST_CTRL, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES);

    m_ctrl->AppendToggleColumn(wxT("Enabled"));
    m_ctrl->AppendTextColumn(wxT("Mod Name"));
    m_ctrl->AppendTextColumn(wxT("Version #"));
    m_ctrl->AppendTextColumn(wxT("Author"));
    m_ctrl->AppendTextColumn(wxT("Game Version #"));
    //m_ctrl->AppendTextColumn(wxT("Last Updated"));

    m_ctrl->Enable();

    topSizer->Add(m_ctrl, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* add = new wxButton(mainPane, MM_ADD, "Add Mod +");
    remove = new wxButton(mainPane, MM_REMOVE, "Remove Mod -");
    //wxButton* toggle = new wxButton(mainPane, MM_TOGGLE_ALL, "Toggle All");

    remove->Enable(false);

    buttonSizer->Add(add, 0, wxEXPAND | wxBOTTOM | wxALIGN_TOP, 5);
    buttonSizer->Add(remove, 0, wxEXPAND | wxBOTTOM | wxALIGN_TOP, 5);
    //buttonSizer->Add(toggle, 0, wxEXPAND | wxBOTTOM | wxALIGN_TOP, 5);
    topSizer->Add(buttonSizer, 0, wxALL, 5);
    mainSizer->Add(topSizer, 2, wxEXPAND | wxBOTTOM, 5);

    mod_description = new wxStaticText(mainPane, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    mainSizer->Add(mod_description, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);

    mainPane->SetSizer(mainSizer);

    m_pMenuBar = new wxMenuBar();

    m_pFileMenu = new wxMenu();
    m_pFileMenu->Append(MM_SETTINGS_MENU, _T("&Settings"));
    m_pFileMenu->AppendSeparator();
    m_pFileMenu->Append(wxID_EXIT, _T("&Quit"));
    m_pMenuBar->Append(m_pFileMenu, _T("&File"));
    
    SetMenuBar(m_pMenuBar);

    addMenu = new wxMenu();
    addMenu->Append(MM_ADD_FOLDER, "From Folder");
    addMenu->Append(MM_ADD_FOLDER, "From Archive");

    Bind(wxEVT_MENU, [=](wxCommandEvent&) { Close(true); }, wxID_EXIT);

    config = mmConfig();
    config.initialise();

    getAllMods();
}

bool mmBase::getAllMods() {
    fs::path install_dir(config["install_dir"].get<std::string>());
    fs::path mods_dir = install_dir / "mods";

    if (config["install_dir"] == "" || !fs::is_directory(install_dir) || !fs::is_directory(mods_dir)) return false;

    json enabled = json::parse(std::ifstream(mods_dir / "enabled_mods.json"));

    bool parse_error = false;
    for (auto& mod : fs::directory_iterator(mods_dir)) {
        auto info = fs::path(mod.path()) / "mod_info.json";
        if (fs::exists(info)) {
            std::ifstream info_file_stream(info);
            std::stringstream buffer;
            buffer << info_file_stream.rdbuf();
            //std::string temp = buffer.str();
            std::string temp = std::regex_replace(buffer.str(), std::regex("#.*\\\n"), "\n");
            std::string info_string = std::regex_replace(temp, std::regex(",(\\s*[\\}\\]])"), "$1");
            if (!json::accept(info_string, true)) wxMessageDialog(this, info_string).ShowModal();

            try {
                json mod_info = json::parse(info_string);
                wxVector<wxVariant> values;
                values.push_back(std::find(enabled["enabledMods"].begin(), enabled["enabledMods"].end(), mod_info["id"]) != enabled["enabledMods"].end());
                values.push_back(mod_info.value("name", "N/A"));
                values.push_back(mod_info.value("version", "N/A"));
                values.push_back(mod_info.value("author", "N/A"));
                values.push_back(mod_info.value("gameVersion", "N/A"));

                auto meta_data = new std::tuple<std::string, std::string, fs::path>(
                    mod_info["id"],
                    mod_info.value("description", "N/A"),
                    mod.path()
                );

                m_ctrl->AppendItem(values, (wxUIntPtr) meta_data);
            } catch (nlohmann::detail::type_error e) {
                parse_error = true;
            }
        }
    }
    m_ctrl->GetColumn(1)->SetWidth(wxCOL_WIDTH_AUTOSIZE);

    if (parse_error) wxMessageDialog(this, "There was an error parsing one or more mod_info.json files.").ShowModal();

    return true;
}

BEGIN_EVENT_TABLE(mmBase, wxFrame)
    EVT_MENU(MM_SETTINGS_MENU, mmBase::onSettings)

    EVT_BUTTON(MM_ADD, mmBase::onAddModClick)
    EVT_MENU(MM_ADD_FOLDER, mmBase::onAddModFolder)

    EVT_BUTTON(MM_REMOVE, mmBase::onRemoveModClick)

    EVT_DATAVIEW_ITEM_VALUE_CHANGED(MM_DATA_LIST_CTRL, mmBase::onListItemDataChange)
    EVT_DATAVIEW_SELECTION_CHANGED(MM_DATA_LIST_CTRL, mmBase::onListRowSelectionChange)
END_EVENT_TABLE()

void mmBase::onSettings(wxCommandEvent& event) {
    wxWindowPtr<mmSettings> settings(new mmSettings(this, config));

    settings->ShowWindowModalThenDo([this, settings](int _) {
        getAllMods();
    });
}

void mmBase::onListContextMenuDisplay(wxCommandEvent& event) {
}

void mmBase::onAddModClick(wxCommandEvent& event) {
    auto button = (wxButton*) event.GetEventObject();
    auto bottomLeftCoord = button->GetRect().GetBottomLeft();

    this->PopupMenu(addMenu, bottomLeftCoord);
}

void mmBase::onAddModFolder(wxCommandEvent& event) {
    wxDirDialog d(this, "Choose a directory", "/", 0, wxDefaultPosition);

    if (d.ShowModal() == wxID_OK) {
        fs::path mod_dir(d.GetPath().ToStdString());
        if (fs::exists(mod_dir / "mod_info.json")) {
            try {
                //#ifdef __linux__
                //fs::rename(mod_dir, fs::path(config["install_dir"].get<std::string>()) / "mods" / mod_dir.filename());
                //#elif _WIN32
                //a = MoveFile(mod_dir.c_str(), (fs::path(config["install_dir"].get<std::string>()) / "mods" / mod_dir.filename()).c_str());
                //#endif
                fs::copy(mod_dir, fs::path(config["install_dir"].get<std::string>()) / "mods" / mod_dir.filename(), fs::copy_options::recursive);
                fs::remove_all(mod_dir);

                getAllMods();

                json info;
                int i = 0;
                for (; i < m_ctrl->GetItemCount(); i++) {
                    info = json::parse(std::ifstream(fs::path(config["install_dir"].get<std::string>()) / "mods" / mod_dir.filename() / "mod_info.json"));
                    auto item_id = std::get<0>(*(std::tuple<std::string, std::string, fs::path>*) m_ctrl->GetItemData(m_ctrl->RowToItem(i)));
                    if (item_id == info["id"]) break;
                }

                wxMessageDialog check(this, "Activate '" + info["name"].get<std::string>() + "'?", wxMessageBoxCaptionStr, wxCENTRE | wxCANCEL | wxYES_NO);
                if (check.ShowModal() == wxID_YES) {
                    m_ctrl->SetToggleValue(true, i, 0);
                    return;
                }
            } catch (std::filesystem::filesystem_error e) {
                wxMessageDialog(this, e.what()).ShowModal();
            }
        } else wxMessageDialog(this, "Could not find 'mod_info.json' in the top level of the folder. \nMaybe check its subdirectories?").ShowModal();
    }
}

void mmBase::onRemoveModClick(wxCommandEvent& event) {
    int i = m_ctrl->GetSelectedRow();
    if (i == wxNOT_FOUND) return;

    auto name = m_ctrl->GetTextValue(i, 1);
    wxMessageDialog check(this, "Are you sure you want to delete '" + name + "'? \nThis cannot be undone!", wxMessageBoxCaptionStr, wxCENTRE | wxCANCEL | wxYES_NO | wxNO_DEFAULT);
    if (check.ShowModal() == wxID_YES) {
        auto meta_data = *(std::tuple<std::string, std::string, fs::path>*) m_ctrl->GetItemData(m_ctrl->RowToItem(m_ctrl->GetSelectedRow()));
        auto mod_dir = std::get<2>(meta_data);
        fs::remove_all(mod_dir);

        json enabled = json::parse(std::ifstream(fs::path(config["install_dir"].get<std::string>()) / "mods" / "enabled_mods.json"));
        auto exists_iter = std::find(enabled["enabledMods"].begin(), enabled["enabledMods"].end(), std::get<0>(meta_data));
        if (m_ctrl->GetToggleValue(i, 0)) {
            if (exists_iter != enabled["enabledMods"].end()) {
                enabled["enabledMods"].erase(exists_iter);
                std::ofstream(fs::path(config["install_dir"].get<std::string>()) / "mods" / "enabled_mods.json") << std::setw(4) << enabled;
            }
        }
        m_ctrl->DeleteItem(i);
        getAllMods();
    }
}

void mmBase::onToggleAllClick(wxCommandEvent& event) {
}

void mmBase::onListItemDataChange(wxDataViewEvent& event) {
    int i = m_ctrl->ItemToRow(event.GetItem());

    fs::path enabled_mods_file = fs::path(config["install_dir"].get<std::string>()) / "mods" / "enabled_mods.json";
    json enabled;
    std::ifstream(enabled_mods_file) >> enabled;
    auto meta_data = *(std::tuple<std::string, std::string, fs::path>*) m_ctrl->GetItemData(event.GetItem());

    auto exists_iter = std::find(enabled["enabledMods"].begin(), enabled["enabledMods"].end(), std::get<0>(meta_data));
    if (m_ctrl->GetToggleValue(i, 0)) {
        if (exists_iter == enabled["enabledMods"].end()) {
            if (m_ctrl->GetTextValue(i, 4) != CURRENT_SS_VERSION) {
                wxMessageDialog check(this, "This mod may be for a different version of Starsector. \nContinue?", wxMessageBoxCaptionStr, wxCENTRE | wxCANCEL | wxYES_NO | wxNO_DEFAULT);
                if (check.ShowModal() != wxID_YES) {
                    m_ctrl->SetToggleValue(false, i, 0);
                    return;
                }
            }
            enabled["enabledMods"].push_back(std::get<0>(meta_data));
        }
    } else if (exists_iter != enabled["enabledMods"].end()) {
        enabled["enabledMods"].erase(exists_iter);
    }

    std::ofstream(enabled_mods_file) << std::setw(4) << enabled;
}

void mmBase::onListRowSelectionChange(wxDataViewEvent& event) {
    remove->Enable(true);

    auto meta_data = *(std::tuple<std::string, std::string, fs::path>*) m_ctrl->GetItemData(event.GetItem());

    mod_description->SetLabel(std::get<1>(meta_data));
    mainSizer->Layout();
}
