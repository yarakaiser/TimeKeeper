#include "App.h"
#include "MainFrame.h"
#include "wx/wx.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
	// Initialize the main frame
	MainFrame* mainFrame = new MainFrame("TimeKeeper");
	
	// Show the main frame
	mainFrame->Show(true);
	
	// Set the main frame as the top-level window
	SetTopWindow(mainFrame);
	
	return true;
}
