#include "mmBase.h"

mmBase::mmBase() : wxFrame(nullptr, wxID_ANY, "Starsector Mod Manager") {
    wxPanel* mainPane = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

    m_ctrl = new wxDataViewListCtrl(mainPane, MM_LISTITEMDATACHANGE);

    m_ctrl->AppendToggleColumn(wxT("Enabled"));
    m_ctrl->AppendTextColumn(wxT("Mod Name"));
    m_ctrl->AppendTextColumn(wxT("Version Number"));
    m_ctrl->AppendTextColumn(wxT("Last Updated"));

    m_ctrl->Enable();

    topSizer->Add(m_ctrl, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* add = new wxButton(mainPane, MM_ADD, "Add Mod +");
    wxButton* remove = new wxButton(mainPane, MM_REMOVE, "Remove Mod -");
    wxButton* toggle = new wxButton(mainPane, MM_TOGGLEALL, "Toggle All");

    buttonSizer->Add(add, 0, wxEXPAND | wxBOTTOM | wxALIGN_TOP, 5);
    buttonSizer->Add(remove, 0, wxEXPAND | wxBOTTOM | wxALIGN_TOP, 5);
    buttonSizer->Add(toggle, 0, wxEXPAND | wxBOTTOM | wxALIGN_TOP, 5);
    topSizer->Add(buttonSizer, 0, wxALL, 5);

    mainSizer->Add(topSizer, 2, wxEXPAND | wxBOTTOM, 5);
    mainSizer->Add(bottomSizer, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);

    mainPane->SetSizer(mainSizer);
}

BEGIN_EVENT_TABLE(mmBase, wxFrame)
    EVT_DATAVIEW_ITEM_VALUE_CHANGED(MM_LISTITEMDATACHANGE, mmBase::onListItemDataChange)
END_EVENT_TABLE()

void mmBase::onListContextMenuDisplay(wxCommandEvent& event) {
}

void mmBase::onAddModClick(wxCommandEvent& event) {
}

void mmBase::onRemoveModClick(wxCommandEvent& event) {
}

void mmBase::onToggleAllClick(wxCommandEvent& event) {
}

void mmBase::onListItemDataChange(wxDataViewEvent& event) {
    int i = m_ctrl->ItemToRow(event.GetItem());
    std::string string;
    string.append(m_ctrl->GetTextValue(i, 1));
    string.append(" ");
    string.append(m_ctrl->GetToggleValue(i, 0) ? "Active" : "Inactive");
    string.append("\n");
    wxWindowPtr<wxDialog> dlg(new wxMessageDialog(this, string));
    dlg->ShowModal();
}