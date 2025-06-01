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
	//m_saveButton = new wxButton(panel, ID_SAVE_BUTTON, "Save");

	//Bind buttons to events
	m_startButton->Bind(wxEVT_BUTTON, &MainFrame::OnStart, this);
	m_resetButton->Bind(wxEVT_BUTTON, &MainFrame::OnReset, this);

	
	buttonSizer->Add(m_startButton, 0, wxALL, 5);
	buttonSizer->Add(m_resetButton, 0, wxALL, 5);
	//buttonSizer->Add(m_saveButton, 0, wxALL, 5);

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
}

void MainFrame::LoadLogsForDate(const wxString& date)
{
}

void MainFrame::OnStart(wxCommandEvent& evt) {
	if (!m_isRunning) {
		m_timer->Start(1000); // Timer startet mit 1-Sekunden-Intervall
		m_startButton->SetLabel("Stop");

		//Log the initial active window immediately
		if (m_windowLogger) {
			std::string title = GetActiveWindowTitle();
			std::cout << "Active window title: " << title << std::endl;
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
			m_windowLogger->LogWindowChange(title);
			lastTitle = title;
		}
	}
}

void MainFrame::UpdateDisplay() {
	m_timeDisplay->SetLabel(wxString::Format("%02d:%02d:%02d",
		m_hours, m_minutes, m_seconds));
}



