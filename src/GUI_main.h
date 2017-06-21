#ifndef _P_GUI_
#define _P_GUI_ 1

/* 
----------------------------------------------------------------
			To do list 
----------------------------------------------------------------
Current list:
* cant panels in SolTrace
* heliostat positioning table - too many templates

* What's with the inversion of hermite images vs soltrace??
* No. runs toggle on user parametric tab
* Run user parametric, save heliostat data, cancel from dialog -> run button does not reset to start
* run with just a clear sky model
* multiple template issues
* land? kml



*/



#define _USE_WINDOW_BG	0	//Flag to indicate whether we should use the window BG. The wxNotebook has some trouble with color inheritance



#include <map>
#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wx/defs.h>
#include <wx/filedlg.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/statline.h>
#include <wx/bitmap.h>
#include <wx/splash.h>
#include <wx/gauge.h>
#include <wx/stdpaths.h>
#include <wx/timer.h>
#include <wx/progdlg.h>
#include <wx/clipbrd.h>
#include <wx/filepicker.h>
#include <wx/textfile.h>
#include <wx/tglbtn.h>
#include <wx/filename.h>

#include "string_util.h"
#include "interop.h"
#include "gui_util.h"
#include "parse_kml.h"
#include "solpos00.h"
#include "sort_method.h"
#include "Toolbox.h"
#include "mod_base.h"	//Use for variable structures
#include "InputControl.h"
#include "OutputControl.h"
#include "rename_dialog.h"
#include "rec_select_dialog.h"
#include "layout_sim_export_dialog.h"
#include "FieldPlot.h"
#include "FluxPlot.h"
#include "gui_about_dialog.h"
#include "kml_select_dialog.h"
#include "ImageCanvas.h"
#include "settings_dialog.h"
#include "param_dialog.h"
#include "pagepanel.h"
#include <wex/diurnal.h>

#include "definitions.h"
#include "SolarField.h"
#include "Heliostat.h"
#include "Flux.h"
#include "Receiver.h"
#include "Land.h"
#include "Financial.h"
#include "Ambient.h"
#include "LayoutSimulateThread.h"
#include "STSimulateThread.h"
#include "STObject.h"

#include "stapi.h"	//Soltrace
#include <shared/lib_weatherfile.h>
#include <wex/lkscript.h>

#pragma warning( disable : 4267 )

//menu id's
enum{
	ID_FILE_NEW=5000,
    ID_FILE_OPEN,
	ID_FILE_SCRIPT,
    ID_FILE_SCRIPT_OPEN,
	ID_FILE_RECENT,
    ID_FILE_RECENT_LOAD=5050,
	ID_FILE_SAVE,
	ID_FILE_SAVEAS,
	ID_FILE_EXIT,

	ID_TOOLS_SETTINGS,

	ID_HELP_ABOUT,
	ID_HELP_CONTENTS,
	ID_HELP_LICENSE,

    ID_RUN_LAYOUT,
    ID_RUN_REFRESH,
    ID_RUN_SIMULATE,
    ID_RUN_PARAMETRIC,
    ID_RUN_USERPARAMETRIC,
    ID_RUN_OPTIMIZE,

	ID_SIM_TIMER,
	ID_SPFRAME_WINDOW
};

//Icon id's
enum{
	//Sidebar icons
	ID_ICON_MAIN = 0,
	ID_ICON_CLIMATE,
	ID_ICON_FINANCE,
	ID_ICON_COST,
	ID_ICON_LAYOUTCONFIG,
	ID_ICON_LAYOUT,
	ID_ICON_LAYOUTCALC,
	ID_ICON_HELIOSTAT_FOLDER,
	ID_ICON_HELIOSTAT,
	ID_ICON_RECEIVER_FOLDER,
	ID_ICON_RECEIVER,
	ID_ICON_PLANT,
	ID_ICON_RESULTS,
	ID_ICON_SOLTRACE,
	ID_ICON_FLUX,
	ID_ICON_TABLE,
	ID_ICON_GEARS,
	ID_ICON_SIMULATIONS,
    ID_ICON_SIMULATIONS32,
	ID_ICON_OPTIMIZE,

	//other icons - correspond to image list position, not the wxART_<> values
	ID_ICON_INFORMATION,
	ID_ICON_QUESTION,
	ID_ICON_WARNING,
	ID_ICON_ERROR
};

class InputControl;
class OutputControl;
class AutoPilot_S;
class AutoPilot_MT;
typedef std::map<spbase*, InputControl*> input_map;
typedef std::map<spbase*, OutputControl*> output_map;

string to_string(wxString &wxs);
string to_varpath(string &group, int &ID, string &name);
string to_varpath(string &group, string &ID, string &name);
string to_varpath(char group[], int &ID, char name[]);
string to_varpath(char group[], char ID[], char name[]);
string to_varpath(char group[], string &ID, char name[]);

class SPFrame;
struct ST_FluxObj;
class STSimThread;
struct ST_System;



struct PageNames
{
	wxString
		climate,
		markets,
		costs,
		layout_control,
		heliostat_master,
		receiver_master,
		plant,
		simulations,
		simulations_layout,
		simulations_performance,
		simulations_parametrics,
		simulations_optimization,
		results,
		results_layout,
		results_flux,
		results_summary;
};


//---------------------------------------
class SPFrame : public wxFrame
{

private:
	gui_settings _gui_settings;
	PageNames pageNames;
	//Default themes and sizings
	int _tool_tip_delay, _n_threads, _n_threads_active, _nrecent_max, _trial_days;
	wxDateTime _trial_start;

	wxColour
		_helptext_colour,
		_background_colour,
		_colorTextBG,
		_colorTextFG;
	wxSize
		_default_input_size,
		_spin_ctrl_size,
		_grid_but_size;
	bool 
		_geom_modified,	//Flag indicating that the geometry displayed in the layout grid has not been updated
		_inputs_modified,	//Flag indicating whether any (relevant) input value has changed, requiring a File->Save prompt
		_in_layout_simulation,	//Flag indicating whether a layout simulation is currently running
		_in_flux_simulation,	//Flag indicating whether a flux simulation is currently running
		_in_optimize_simulation,	//Flag indicating whether an optimization simulation is currently running
		_in_param_simulation,	//Flag indicating whether a parametric simulation is currently running
		_in_user_param_simulation, //Flag indicating whether a user-parametric simulation is currently running
		_is_mt_simulation,	//Is the current simulation multithreaded?
		_cancel_simulation; //Flag indicating whether the user wants to cancel the simulation
	string 
		_software_version,	//Software version tag
		_contact_info;	//String containing contact info for bugs, etc. Used in crash messages.
		
    wxFileName
        _recent_file_list,	//File containing the list of recent files
		_settings_file,
		_working_dir, //The working directory
		_climate_dir,	//Directory with the climate files
		_install_dir, //the head installation directory
        _image_dir, //the directory containing resource images
		_local_dir,	//The location to store edited app files
		_help_dir,	//the location of the help directory
		_open_file,	//The currently opened file
		_backup_file, //name of the backup file
		_sim_log_file;	//Log file with notices, errors, etc

	vector<string> _sim_log_data;	//All data to be written to the log file at the end of the simulation

	//wxTreebook *_nb_main;
    PagePanel *_page_panel;

	/*var_set 
		_variables,
		_definitions;*/
    var_map 
        _variables;
	input_map _input_map;
	output_map _output_map;
	SolarField _SF;
	wxImageList *_imageList;
	wxGrid *_bounds_grid;
	wxSpinCtrl *_bounds_ct;

	wxTimer *_sim_timer;
	wxStopWatch _sim_watch;

	//menus
	wxMenu
		*recentMenu,
		*menuFile,
		*menuHelp,
		*menuTools,
        *menuRun;

    //overall
    wxBoxSizer *_main_sizer;

	//Climate page
	wxListCtrl *_climate_ctrl;
	wxTextCtrl *_climate_search;
	OutputControl 
		*_loc_city,
		*_loc_state,
		*_loc_timezone,
		*_loc_elevation,
		*_loc_latitude,
		*_loc_longitude;
	ArrayString
		_all_climate_files,
		_climate_files,
		_local_wfdat;	//A local variable to store the broken out weather file data
						//"[P]day,hour,month,DNI,Tdb,Pamb,Vwind[P]..."
	wxSpinCtrl *_user_sun_ct;
	wxGrid *_user_sun_grid;
	vector<wxTextCtrl*> _atm_coefs;
	//wxComboBox *_atm_combo;
	//wxStaticText *_atm_helptext;
	wxPanel *_atm_panel;

	//Objects for the master heliostat page
	wxButton *_heliotemp_add;
	wxButton *_heliotemp_del;
	wxButton *_heliotemp_state;
	wxButton *_heliotemp_rename;
	wxListCtrl *_heliostat_config;
	wxGrid *_temp_bounds;

	//Objects for the master receiver page
	wxButton 
		*_rec_add,
		*_rec_del,
		*_rec_state,
		*_rec_rename;
	wxListCtrl *_rec_config;
	vector<wxPanel*> _rec_pages;	//pointers to all of the receiver pages
	InputControl *_rec_which;	//Combo list of available receivers
	std::map<string, vector<wxTextCtrl*> > _hll_coefs, _hlw_coefs;

	//Objects for Layout page
	wxGrid *_design_grid;
	wxSpinCtrl *_design_ct;
	InputControl *_design_combo;
	wxPanel *_design_combo_panel;
	wxCheckBox *_design_simallhrs;
	wxSpinCtrl 
		*_design_ndays,
		*_design_nhours; 
	wxStaticText 
		*_design_ndays_lab,
		*_design_nhours_lab;
	wxListCtrl *_design_days_display;
	wxStaticText 
		*_design_helptext,
		*_design_steptext;
	wxBitmap _simimages[4];	//images for simulation button {normal,hover,click,cancel}
    wxBitmapButton *_layout_button;

	//objects for the simulations pages
	wxNotebook *_simbook;
	wxCheckBox *_do_positions;
	wxCheckBox *_do_optimize;
	wxTextCtrl *_layout_log;
	wxStaticText 
		*_layout_elapsed,
		*_layout_remain;
	wxGauge *_layout_gauge;
	wxGrid *_layout_grid;
	wxSpinCtrl *_layout_ct;
	wxButton
		*_layout_import,
		*_layout_export,
		*_layout_delete,
		*_layout_insert;
	wxBitmapButton
		*_layout_refresh;
	ImageCanvas
		*_refresh_warning;
	wxDirPickerCtrl
		*_dir_ctrl;
	LayoutSimThread *_simthread;
	STSimThread *_stthread;

	//Flux page
	wxComboBox
		*_rec_select;
	FluxPlot
		*_flux_frame;
	MyToggleButton
		*_scatter_btn;

	wxListCtrl
		*_flux_lc;
	wxCheckBox
		*_flux_lc_check;
	Hvector
		_flcsort;
	wxGauge *_flux_gauge;
	wxStaticText
		*_flux_progress;
    wxBitmapButton *_flux_button;

	//optimize page
	wxGauge *_optimize_summary_gauge;
	wxGauge *_optimize_detail_gauge;
	wxTextCtrl *_optimize_log;
	wxStaticText
		*_optimize_summary_progress,
		*_optimize_detail_progress,
		*_optimize_elapsed;
	AutoPilot_MT
		*_SFopt_MT;
	AutoPilot_S
		*_SFopt_S;
	bool _enable_detail_update_gauge;
    wxBitmapButton *_optimize_button;
    std::vector<double> _best_opt_result;
    wxGrid *_opt_var_grid;
    optimization _opt_data;    //use the parametric structure because it's already been coded

	//Parametrics page
	wxPanel 
		*_sam_param_panel;
	wxBitmapButton
		*_sam_toggle_button,
		*_par_toggle_button;
	bool
		_sam_toggle_shown,
		_par_toggle_shown,
		_user_par_toggle_shown;
	wxBitmap 
		_toggle_expand,
		_toggle_collapse;
	//
	wxPanel
		*_par_panel;
	wxButton
		*_par_add_var,
		*_par_remove_var,
		*_par_edit_var,
		*_par_edit_linkages;
	wxListBox
		*_par_variables,
		*_par_values;
	parametric
		_par_data;
	wxBitmapButton
		*_par_button;
	SolarField _par_SF;
	wxGauge
		*_par_single_gauge,
		*_par_all_gauge;
	wxStaticText
		*_par_single_text,
		*_par_all_text;
	//
	wxPanel 
		*_user_par_panel;
	wxBitmapButton
		*_user_par_button,
		*_user_par_toggle_button;
	wxGrid
		*_user_par_grid;
	wxSpinCtrl 
		*_user_var_ct,
		*_user_sim_ct;

	wxGauge
		*_user_single_gauge,
		*_user_all_gauge;
	wxStaticText
		*_user_single_text,
		*_user_all_text;
	simulation_table
		_user_sim_table;
	//wxCheckBox
		//*_user_save_field_data,
		//*_user_save_system_summary;
	//Plotting and output pages
	wxComboBox
		*_plot_select;
	FieldPlot 
		*_plot_frame;
	MyToggleButton *_zoom_pan, *_zoom_rect, *_show_field_data;
    int _field_left_mouse_start[2];
    
    sim_results
		results;
	wxGrid *_results_grid;
    ST_System *_STSim;

	bool CheckLicense();

	//void CreateNotebook(bool is_fileopen_call = false);
    void CreateInputPages(wxWindow *parent, PagePanel *pagepanel, bool is_fileopen_call = false);
	void SetCaseName(wxString case_name = _T("New Case"));
	void bindControls();
    void SetSimulationStatus(bool in_sim, bool &sim_type_flag, wxBitmapButton *button);
	void CreateClimatePage(wxScrolledWindow *page);
	void CreateMasterHeliostatPage(wxScrolledWindow *parent);
	void CreateHeliostatPage(wxScrolledWindow *parent, int id);
	void CreateMasterReceiverPage(wxScrolledWindow *parent);
	void CreateReceiverPage(wxScrolledWindow *parent, int id);
	void CreateMarketPage(wxScrolledWindow *parent);
	void CreateCostPage(wxScrolledWindow *parent);
	void CreateLayoutPage(wxScrolledWindow *parent);
	void CreatePlantPage(wxScrolledWindow *parent);
	void CreateFieldPlotPage(wxScrolledWindow *parent, wxArrayStr &choices, int selection = 0);
	void CreateFluxPlotPage(wxScrolledWindow *parent);
	void CreateSimulationsPage(wxScrolledWindow *parent);
	void CreateSimulationsLayoutTab(wxScrolledWindow *parent);
	void CreateSimulationsFluxTab(wxScrolledWindow *parent);
	void CreateSimulationsParametricsTab(wxScrolledWindow *parent);
	void CreateResultsSummaryPage(wxScrolledWindow *parent, sim_results &results);
	void CreateOptimizationPage(wxScrolledWindow *parent);
#if _CUSTOM_REC == 1
	enum TUBE_MATERIAL {HAYNES_230, STAINLESS_316};
	enum CIRC_PROFILE {SINUSOIDAL, SQUARE};
	enum DESIGN_LOCATION {PEAK_FLUX, CIRC_DELTA};
	enum QUALITY_METHOD {GLOBAL, LOCAL_PANEL, LOCAL_HEADER};
	enum QUALITY_BASIS {AVERAGE, MEDIAN, PEAK, TUBE_DESIGN, SPECIFIED};
	enum HEADER_DOMAIN {HALF_ADJACENT, FULL_ADJACENT, UNIT_SURFACE};
	void CreateCustomReceiverPage(wxScrolledWindow *parent);
	void CustomRec_UpdateCalculatedMapValues(var_set &vset);
	void CreateNBBGeometry(SolarField &SF, var_map &varmap, ST_System &ST);
	void CustomProcessRayData(ST_System &ST, var_set &vset, double *xhits, double *yhits, double *zhits, int *emap, int *smap, int *rnum, int nrays, double qray);	
	void UpdateProcessResults(int nout, int ntot, wxString msg);
	void CalculateFluxStatistics(vector<ST_FluxObj> &fmaps, stat_object &flux_density);
	struct panel_pair;
	struct panel_set;
    struct flow_path;
    vector<panel_set*> _all_panels;
    //void CalculateFlowPathProfiles(var_set &vset, vector<panel_set> &panels, vector<flow_path> &fpaths );
    void CalculateFluxProfiles(vector<panel_set*> &panels);
	void CalculateFluxQuality(var_set &vset, vector<panel_set*> &panels, wxString result_dir);
	void CalculatePanelLinkages(vector<ST_FluxObj> &fmaps, var_set &vset, vector<panel_set*> &panels);
	double SpecheatCO2(double T /*deg K*/);
	void LocateTubeDesignPoint(int design_location, vector<panel_set*> &panels, int &ipanel, int &inode);
	bool AbsorberDesign(var_set &vset, vector<panel_set*> &panels, wxString result_dir);
	void SimulateTubePerformanceEES(var_set &vset, vector<panel_set*> &panels, wxString result_dir);
	void CreateFluxResultsWindow(var_set &vset, wxString result_dir);
	void OnTubeResView( wxCommandEvent &event );
#endif
#if _SANDBOX == 1
	bool Sandbox();
#endif

protected:
	/* Event handlers should be protected, others can be public */
    void TempEscape( wxKeyEvent &event);
	void OnFileOpen( wxCommandEvent &event);
	void OnFileScript( wxCommandEvent &event);
	void OnFileScriptOpen( wxCommandEvent &event);
	void OnFileRecent( wxCommandEvent &event);
    void OnFileRecentLoad( wxCommandEvent &event);
	void OnFileSaveAs( wxCommandEvent &event);
	void OnFileSave( wxCommandEvent &event);
	void OnFileNew( wxCommandEvent &event);
    void OnClose( wxCloseEvent &event );
	void OnToolsSettings( wxCommandEvent &event);
	void OnHelpAbout( wxCommandEvent &event);
	void OnHelpContents( wxCommandEvent &event);
	void OnHelpLicense( wxCommandEvent &event);
	void OnMenuRunLayout( wxCommandEvent &event);
    void OnMenuRunSimulation( wxCommandEvent &event);
    void OnMenuRunRefresh( wxCommandEvent &event);
    void OnMenuRunParametric( wxCommandEvent &event);
    void OnMenuRunUserParametric( wxCommandEvent &event);
    void OnMenuRunOptimize( wxCommandEvent &event);

	void OnFocusEvent( wxFocusEvent &event);
	void OnComboEvent( wxCommandEvent &event);
	void OnSimTimerCount( wxTimerEvent &event);

	void OnDispatchGrid( wxCommandEvent &event);

	void OnBoundsImport( wxCommandEvent &event);
	void OnBoundsExport( wxCommandEvent &event);
	void OnBoundsCount( wxCommandEvent &event);
	void OnBoundsEdit( wxGridEvent &event);
	void OnDesignImport( wxCommandEvent &event);
	void OnDesignExport( wxCommandEvent &event);
	void OnDesignCount( wxCommandEvent &event);
	void OnDesignEdit( wxGridEvent &event);
	void OnDesignSelect( wxCommandEvent &event);
	void OnDesignSimhrsChanged( wxCommandEvent &event);
	void OnDesignNdaysChanged( wxCommandEvent &event);
	void OnDesignNhrsChanged( wxCommandEvent &event);
	
	void OnChangeClimateFile( wxListEvent &event);
	void OnRefreshClimateList( wxCommandEvent &event);
	void OnOpenClimateFolder( wxCommandEvent &event);
	void OnClimateSearchText( wxCommandEvent &event);
	void OnSunImport( wxCommandEvent &event);
	void OnSunExport( wxCommandEvent &event);
	void OnSunCount( wxCommandEvent &event);
	void OnSunEdit( wxCommandEvent &event);
	void OnAtmUpdate( wxCommandEvent &event);
	void OnAttenFocus( wxFocusEvent &event);
	
	void OnHeliotempAdd( wxCommandEvent &event);
	void OnHeliotempDel( wxCommandEvent &event);
	void OnHeliotempState( wxCommandEvent &event);
	void OnHeliotempRename( wxCommandEvent &event);
	void OnHeliotempSelect( wxListEvent &event);
	void OnHeliotempDeselect( wxListEvent &event);
	void OnHeliotempItemEdit( wxListEvent &event);
	void OnTempBoundsEdit( wxCommandEvent &event);
	void OnHeliotempOrderUp( wxCommandEvent &event);
	void OnHeliotempOrderDown( wxCommandEvent &event);
	
	void OnReceiverAdd( wxCommandEvent &event);
	void OnReceiverDel( wxCommandEvent &event);
	void OnReceiverState( wxCommandEvent &event);
	void OnReceiverRename( wxCommandEvent &event);
	void OnReceiverSelect( wxListEvent &event);
	void OnReceiverDeselect( wxListEvent &event);
	
	void OnHeatLossLoadFocus( wxFocusEvent &event);
	void OnHeatLossWindFocus( wxFocusEvent &event);

	void OnDoLayout( wxCommandEvent &event);			//*** full version only
	void OnLayoutImport( wxCommandEvent &event);			//*** full version only
	void OnLayoutExport( wxCommandEvent &event);			//*** full version only
	void OnLayoutDelete( wxCommandEvent &event);
	void OnLayoutInsert( wxCommandEvent &event);
	void OnLayoutCount( wxCommandEvent &event);
	void OnLayoutEdit( wxGridEvent &event);
	void OnLayoutRefresh( wxCommandEvent &event);
	void OnPlotSelectCombo( wxCommandEvent &event );
    void OnPlotDataToggle( wxCommandEvent &event );
	void OnPlotFontIncrease( wxCommandEvent &event);
	void OnPlotFontDecrease( wxCommandEvent &event);
	void OnBitmapSave( wxCommandEvent &event);
    void OnFieldPlotZoomIn( wxCommandEvent &event);
    void OnFieldPlotZoomOut( wxCommandEvent &event);
    void OnFieldPlotZoomRect( wxCommandEvent &event);
    void OnFieldPlotZoomPan( wxCommandEvent &event);
    void OnFieldPlotZoomOriginal( wxCommandEvent &event);
    void OnFieldPlotMouseLeftDown( wxMouseEvent &event);
    void OnFieldPlotMouseLeftUp( wxMouseEvent &event);
    void OnFieldPlotMouseWheel( wxMouseEvent &event);
    void OnFieldPlotMouseCenterUp( wxMouseEvent &event);
    void OnFieldPlotMouseCenterDown( wxMouseEvent &event);
    void OnFieldPlotMotion( wxMouseEvent &event);

	void OnPlotResIncrease( wxCommandEvent &event);
	void OnPlotResDecrease( wxCommandEvent &event);
	void OnPlotResReset( wxCommandEvent &event);
	void OnFluxBitmapSave( wxCommandEvent &event);			//*** full version only
	void OnFluxTableSave( wxCommandEvent &event);			//*** full version only
	void OnLayoutSimulationExport( wxCommandEvent &event);			//*** full version only
	void OnFluxLCSort( wxCommandEvent &event);
	void OnSTRayDataButton( wxCommandEvent &event);
	void OnReceiverFluxSelect( wxCommandEvent &event);
	void OnScatterToggle( wxCommandEvent &event);
	void OnColormapSelect( wxCommandEvent &event);
    void OnFluxPlotBinChangeX( wxFocusEvent &event);
    void OnFluxPlotBinChangeY( wxFocusEvent &event);
    void OnFluxPlotBinChange( int axis );

	
    void OnDoPerformanceSimulation( wxCommandEvent &event);			//*** full version only
	void OnStinputExport( wxCommandEvent &event);			//*** full version only
	void OnSAMInputButton( wxCommandEvent &event);
	void OnSAMDirSelect( wxCommandEvent &event);
	void OnToggleParametric( wxCommandEvent &event);

	void OnParAddVar( wxCommandEvent &event);
	void OnParRemoveVar( wxCommandEvent &event);
	void OnParEditVar( wxCommandEvent &event);
	void OnParEditLinkages( wxCommandEvent &event);
	void OnParVarlistSelected( wxCommandEvent &event);
	void OnParVarDoubleClick( wxCommandEvent &event);
	void OnParametricSimulate( wxCommandEvent &event);			//*** full version only
	void OnUserVarCount( wxCommandEvent &event);
	void OnUserSimCount( wxCommandEvent &event);
	void OnUserParImport( wxCommandEvent &event);
	void OnUserParExport( wxCommandEvent &event);			//*** full version only
	void OnUserParVariables( wxCommandEvent &event);
	void OnUserParSimulate( wxCommandEvent &event);			//*** full version only
	void OnUserParEditTable( wxGridEvent &event);

	void OnDoOptimizationSimulation( wxCommandEvent &event);	//*** full version only
	void OnClearOptimizationLog( wxCommandEvent &event);
	void OnSaveOptimizationLog( wxCommandEvent &event);
	void OnApplyOptimizationResult( wxCommandEvent &event);
	void OnDisableDetailUpdate( wxCommandEvent &event);

    void OnOptimizeAddVar( wxCommandEvent &event);
	void OnOptimizeRemoveVar( wxCommandEvent &event);
	void OnOptimizeGridChange( wxGridEvent &event);


	void OnSimulationResultsExport( wxCommandEvent &event);			//*** full version only
	void OnSimulationResultsCopy( wxCommandEvent &event);			//*** full version only

public:
	SPFrame(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);
    ~SPFrame();
    static SPFrame &Instance();
	//virtual ~SPFrame();
	wxSize getDefaultInputSize(){return _default_input_size;}
    bool CloseProject( bool force = false);
	void ShowContextHelp();

    SolarField *GetSolarFieldObject();
    var_map *GetVariablesObject();
    sim_results *GetResultsObject();
    void SetThreadCount(int nthread);
    int GetThreadCount();
    ArrayString *GetLocalWeatherDataObject();
    FluxPlot *GetFluxPlotObject();
    FieldPlot *GetFieldPlotObject();
    parametric *GetParametricDataObject();
    optimization *GetOptimizationDataObject();
    std::string GetVersionInfo();


    void UpdateHelioUITemplates();
    void UpdateReceiverUITemplates();
	void UpdateInputValues();
	void UpdateCalculatedGUIValues();
	
	bool CreateRestorePoint();
	void PreSimulation();
	void PostSimulation();
	wxGauge *chooseProgressGauge();
	wxStaticText *chooseProgressText();

	void Save(int save_config);
	void Open(wxString file = wxEmptyString, bool quiet=false);
	void NewFile();
	void UpdateRecents();
	void AddRecentFile(wxFileName &fname);
	bool GetInputModState(){return _inputs_modified;}
	bool ModCheckPrompt();
	
	void UpdateLayoutGrid();
	void UpdateLayoutLog(string msg, bool clear = false);
	
	void PlotFontIncrease( wxString type);
	void PlotFontDecrease( wxString type);
	
	void ExportGrid( wxGrid *grid );
	void ImportGrid( wxGrid *grid );
	void GridCount( wxSpinCtrl *sc, wxGrid *grid );
	
	void setSunGrid();
	void setHelioTemplateGrid();
	void UpdateDesignSelect(int sel, var_map &vset);

	void LoadClimateFiles();
	void UpdateClimateListValues();
	void UpdateClimateFile( wxString &file, var_map &V, bool mapval_only=false);

	void setBoundsGrid(var_map &V); //string &incs, string &excs);	
	void BoundsImport();
	void BoundsExport();

	void SetGeomState(bool state);
	bool GetGeomState();

	void DoResultsPage(sim_results &results);
	void SAMInputParametric();
	void SAMInputParametric2();
	void LayoutSimulationExport(SolarField &SF, wxString &fname, vector<bool> &options, wxString &header, wxString &delim, bool quiet = false);
	void CreateResultsTable(sim_result &result, grid_emulator &table);
	void CreateParametricsTable(parametric &par, sim_results &results, grid_emulator &table);
	
	void StartSimTimer();
	void StopSimTimer();
	void SimProgressUpdate(simulation_info* siminfo);
	void SimProgressUpdateMT(int n_complete, int n_tot);
	void SimProgressBase(int n_complete, int n_tot, string *notices, wxGauge *active_gauge, wxStaticText *active_label, wxString label_text = "Simulation");
	void SimErrorHandler(simulation_error *simerror);
	void OptimizeProgressSummaryUpdate(simulation_info *siminfo);
	void OptimizeProgressDetailUpdate(simulation_info *siminfo);
    void OptReloadVariableList();
    void CheckOptGridRange();

	bool DoManagedLayout(SolarField &SF, var_map &vset);
	bool DoPerformanceSimulation(SolarField &SF, var_map &vset, Hvector &helios);
	bool SolTraceFluxSimulation(SolarField &SF, var_map &vset, Hvector &helios);
    bool SolTraceFluxBinning(SolarField &SF);
	bool HermiteFluxSimulationHandler(SolarField &SF, Hvector &helios);
	void UpdateFluxLC(int sortkey=0);
	int SolTraceProgressUpdate(st_uint_t ntracedtotal, st_uint_t ntraced, st_uint_t ntotrace, st_uint_t curstage, st_uint_t nstages, void *data);
	void SolTraceFileExport(string fname);
	void FluxProgressBase(int n_complete, int n_tot, wxGauge *active_gauge, wxStaticText *active_label, wxString label_text = "Calculation");
	bool GetSelectedHeliostats(Hvector &helios, bool select_enabled);

	void ParReloadVariableList();
	void ParRefreshValuesList();
	void ParametricSimulate(parametric &P);
	void ParProgressUpdate(int n_complete, int n_tot);
	void UserParProgressUpdate(int n_complete, int n_tot);
	void UpdateParValueTable();
	void SyncParValuesTable();
	void UserParImport();
    bool ParCheckSetup(parametric &par_data, var_map &vset);
    bool ParCheckSetup(ArrayString &par_vars, var_map &vset);

	void WriteVariablesToFile(wxTextFile &tfile, var_map &vset);
	void LogFileVariableDump(var_map &vset);
	void LogFileVariableDump(var_map &vset, wxArrayStr &messages);
	int PopMessage(wxString message, wxString title = wxEmptyString, long style = wxOK|wxICON_INFORMATION);
	
};

//--------------- Callback methods for the simulation engines --------------------------
static int STCallback(st_uint_t ntracedtotal, st_uint_t ntraced, st_uint_t ntotrace, st_uint_t curstage, st_uint_t nstages, void *data)
{
	SPFrame *frame = static_cast<SPFrame*>( data );
	if(frame != NULL) return frame->SolTraceProgressUpdate(ntracedtotal, ntraced, ntotrace, curstage, nstages, data);
	else return 0;
};

static bool SolarFieldInfoCallback(simulation_info *siminfo, void *data)
{
	SPFrame *frame = static_cast<SPFrame*>( data );
	if(frame != NULL) frame->SimProgressUpdate(siminfo);
	return true;
};

static void SolarFieldErrorCallback(simulation_error *simerror, void *data)
{
	SPFrame *frame = static_cast<SPFrame*>( data );
	if(frame != NULL) frame->SimErrorHandler(simerror);
};

static bool SolarFieldOptimizeSummaryCallback(simulation_info *siminfo, void *data)
{
	SPFrame *frame = static_cast<SPFrame*>( data );
	if(frame != NULL) frame->OptimizeProgressSummaryUpdate(siminfo);
	return true;
};

static bool SolarFieldOptimizeDetailCallback(simulation_info *siminfo, void *data)
{
	SPFrame *frame = static_cast<SPFrame*>( data );
	if(frame != NULL) frame->OptimizeProgressDetailUpdate(siminfo);
	return true;
};

#endif
