#define WIN32_LEAN_AND_MEAN
#include "MainFrame.h"
#include <wx/wx.h>
#include "WindowLogger.h"
#include "sqlite3.h"

//wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
//	EVT_BUTTON(ID_START_BUTTON, MainFrame::OnStart)
//	EVT_BUTTON(ID_RESET_BUTTON, MainFrame::OnReset)
//	//EVT_BUTTON(ID_SAVE_BUTTON, MainFrame::OnSave)  
//	EVT_TIMER(wxID_ANY, MainFrame::OnTimer)
//wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title)
{
	//Maximize(true);

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	m_notebook = new wxNotebook(this, wxID_ANY);
	wxPanel* panel = new wxPanel(m_notebook);  // Timer tab

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	// Zeitanzeige erstellen
	m_timeDisplay = new wxStaticText(panel, wxID_ANY, "00:00:00");
	wxFont font(48, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	m_timeDisplay->SetFont(font);

	// Buttons erstellen
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	m_startButton = new wxButton(panel, ID_START_BUTTON, "Start");
	m_resetButton = new wxButton(panel, ID_RESET_BUTTON, "Reset");

	//Bind buttons to events
	m_startButton->Bind(wxEVT_BUTTON, &MainFrame::OnStart, this);
	m_resetButton->Bind(wxEVT_BUTTON, &MainFrame::OnReset, this);

	
	buttonSizer->Add(m_startButton, 0, wxALL, 5);
	buttonSizer->Add(m_resetButton, 0, wxALL, 5);

	// Layout zusammenbauen
	mainSizer->AddStretchSpacer();
	mainSizer->Add(m_timeDisplay, 0, wxALIGN_CENTER | wxALL, 20);
	mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
	mainSizer->AddStretchSpacer();

	panel->SetSizer(mainSizer);
	m_notebook->AddPage(panel, "Timer");


	// Timer initialisieren
	m_timer = new wxTimer(this);
	// Bind timer
	Bind(wxEVT_TIMER, &MainFrame::OnTimer, this, m_timer->GetId());

	//Variablen initialisieren
	m_hours = 0;
	m_minutes = 0;
	m_seconds = 0;
	m_isRunning = false;

	//WindowLogger initialisieren
	m_windowLogger = new WindowLogger("windowlog.db");

	UpdateDisplay();

	// --- Logs Tab ---
	m_logPanel = new wxPanel(m_notebook);
	wxBoxSizer* logSizer = new wxBoxSizer(wxVERTICAL);

	// Date picker
	m_datePicker = new wxDatePickerCtrl(m_logPanel, wxID_ANY);
	wxBoxSizer* topBarSizer = new wxBoxSizer(wxHORIZONTAL);
	m_refresh = new wxButton(m_logPanel, wxID_ANY, "Refresh");
	m_refresh->Bind(wxEVT_BUTTON, &MainFrame::OnRefresh, this);
	topBarSizer->Add(m_datePicker, 1, wxALL | wxEXPAND, 5);
	topBarSizer->Add(m_refresh, 0, wxALL, 5);
	logSizer->Add(topBarSizer, 0, wxEXPAND | wxALL, 5);
	m_datePicker->Bind(wxEVT_DATE_CHANGED, &MainFrame::OnDateChanged, this);
	

	// List view
	m_logList = new wxListCtrl(m_logPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	m_logList->InsertColumn(0, "Window Title", wxLIST_FORMAT_LEFT, 300);
	m_logList->InsertColumn(1, "Time Spent", wxLIST_FORMAT_RIGHT, 100);
	logSizer->Add(m_logList, 1, wxALL | wxEXPAND, 10);

	m_logPanel->SetSizer(logSizer);
	m_notebook->AddPage(m_logPanel, "Logs");

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(m_notebook, 1, wxEXPAND);
	SetSizerAndFit(topSizer);

	// Load today's logs initially
	LoadLogsForDate(wxDateTime::Today().FormatISODate());

}

MainFrame::~MainFrame()
{
	if (m_windowLogger) {
		m_windowLogger->FlushCurrentSession();
		delete m_windowLogger;
	}
}


std::string GetActiveWindowTitle() {
	HWND hwnd = GetForegroundWindow();
	if (!hwnd) return "";

	char title[256];
	int length = GetWindowTextA(hwnd, title, sizeof(title));
	if (length > 0) {
		return std::string(title, length);
	}
	return "";
}

void MainFrame::OnDateChanged(wxDateEvent& event)
{
	wxDateTime selectedDate = event.GetDate();
	wxString dateStr = selectedDate.FormatISODate(); // "YYYY-MM-DD"
	LoadLogsForDate(dateStr);
}

void MainFrame::OnRefresh(wxCommandEvent& event)
{
	wxDateTime selectedDate = m_datePicker->GetValue();
	wxString dateStr = selectedDate.FormatISODate();
	LoadLogsForDate(dateStr);
}

void MainFrame::LoadLogsForDate(const wxString& date)
{
	m_logList->DeleteAllItems();

	sqlite3* db;
	if (sqlite3_open("windowlog.db", &db) != SQLITE_OK) {
		wxLogError("Cannot open DB");
		return;
	}

	const char* sql = R"(
        SELECT title, SUM(duration)
        FROM WindowLog
        WHERE DATE(start_time) = ?
        GROUP BY title
        ORDER BY SUM(duration) DESC;
    )";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, date.mb_str(), -1, SQLITE_STATIC);

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			int seconds = sqlite3_column_int(stmt, 1);

			int h = seconds / 3600;
			int m = (seconds % 3600) / 60;
			int s = seconds % 60;
			wxString timeStr = wxString::Format("%02d:%02d:%02d", h, m, s);

			long idx = m_logList->InsertItem(m_logList->GetItemCount(), title);
			m_logList->SetItem(idx, 1, timeStr);
		}

		sqlite3_finalize(stmt);
	}

	sqlite3_close(db);
}

void MainFrame::OnStart(wxCommandEvent& evt) {
	if (!m_isRunning) {
		m_timer->Start(1000); // Timer startet mit 1-Sekunden-Intervall
		m_startButton->SetLabel("Stop");

		//Log the initial active window immediately
		if (m_windowLogger) {
			std::string title = GetActiveWindowTitle();
			//std::cout << "Active window title: " << title << std::endl;
			if (!title.empty()) {
				m_windowLogger->LogWindowChange(title);
			}
		}
	}
	else {
		m_timer->Stop();
		m_startButton->SetLabel("Start");
	}
	m_isRunning = !m_isRunning;
}

void MainFrame::OnReset(wxCommandEvent& evt) {
	m_timer->Stop();
	m_isRunning = false;
	m_startButton->SetLabel("Start");
	m_hours = 0;
	m_minutes = 0;
	m_seconds = 0;
	UpdateDisplay();
}



//void MainFrame::OnSave(wxCommandEvent& evt) {
//	// Hier könnte der Code zum Speichern der Zeit in einer Datenbank stehen
//	// Zum Beispiel mit SQLite: sqlite3_exec(db, "INSERT INTO times (hours, minutes, seconds) VALUES (?, ?, ?)", ...);
//	//wxMessageBox("Time saved!", "Info", wxOK | wxICON_INFORMATION, this);
//}

void MainFrame::OnTimer(wxTimerEvent& evt) {
	m_seconds++;

	if (m_seconds > 59) {
		m_minutes++;
		m_seconds = 0;
	}

	if (m_minutes > 59) {
		m_hours++;
		m_minutes = 0;
	}

	UpdateDisplay();

	if (m_windowLogger && m_isRunning) {
		static std::string lastTitle;
		std::string title = GetActiveWindowTitle();
		if (!title.empty() && title != lastTitle) {
			if (title.find("TimeKeeper") != std::string::npos) {
				m_windowLogger->LogWindowChange(title);
				lastTitle = title;
			}
		}
	}
}

void MainFrame::UpdateDisplay() {
	m_timeDisplay->SetLabel(wxString::Format("%02d:%02d:%02d",
		m_hours, m_minutes, m_seconds));
}



