#pragma once
#include <wx/wx.h>
#include <wx/timer.h>
#include "WindowLogger.h"
#include "sqlite3.h"
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>


class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
    ~MainFrame();

    //wxDECLARE_EVENT_TABLE();



private:
    wxStaticText* m_timeDisplay;
    wxButton* m_startButton;
	//wxButton* m_stopButton;
	wxButton* m_resetButton;
    wxButton* m_refresh;
    wxTimer* m_timer;
    WindowLogger* m_windowLogger;

    wxNotebook* m_notebook;

    // Logs tab controls
    wxPanel* m_logPanel;
    wxDatePickerCtrl* m_datePicker;
    wxListCtrl* m_logList;


    int m_minutes;
    int m_seconds;
	int m_hours;
	bool m_isRunning;

    void OnDateChanged(wxDateEvent& event);
    void LoadLogsForDate(const wxString& date);


    void OnStart(wxCommandEvent& event);
	void OnReset(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void UpdateDisplay();
    
    enum {
        ID_START_BUTTON = 1,
	    ID_RESET_BUTTON = 2
    };
    
};

