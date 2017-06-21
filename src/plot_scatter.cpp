#include "plot_scatter.h"
#include "gui_util.h"
#include <math.h>
#include <wx/textfile.h>
#include <algorithm>
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include "plot_base.h"


void PlotScatter::SetZAxisScale(PlotBase &base, bool is_autoscale, double zax_min, double zax_max)
{
    base.SetZRange(zax_min, zax_max, is_autoscale);
}

bool PlotScatter::Plot(PlotBase &base, vector<vector<double> > &data){
	
	/* 
	Create a scatter plot. The data points can be shaded according to some metric. 
	The 'data' object should be formatted as follows:

	[xpos, ypos, shading, size factor]

	[
		[x1,y1,z1,s1],
		[x2,y2,z2,s1],
		...
		[xn,yn,zn,sn]
	]
	*/

	//Use a memory DC with a bitmap, and catch the wxEraseBackground event to prevent flicker
	base._bitmap.Create(base._plotsize.GetWidth(), base._plotsize.GetHeight());
	wxMemoryDC _dc;
	_dc.SelectObject( base._bitmap );

	double maxval=-9.e9, minval=9.e9, zvaltot=0.;
	double maxsize=0;
	for(int i=0; i<(int)data.size(); i++){
		double d = data.at(i).at(2);
		if(d > maxval) maxval = d;
		if(d < minval) minval = d;
		zvaltot += d;

		d = data.at(i).at(3);
		if(d > maxsize) maxsize = d;
	}


    if( base._zaxmax > base._zaxmin && !base._z_autoscale){
		minval = base._zaxmin;
		maxval = base._zaxmax;
	}

	base.AxesSetup(_dc, minval, maxval);
	
	// -----------------------------------------------------
	bool has_data = true;

	_dc.SetPen( wxPen( base._gray, 1, wxSOLID ) );
	_dc.SetBrush( *wxTRANSPARENT_BRUSH );
	double zvalave = zvaltot / (double)data.size();
	//---- Draw the intensity ----
	
	/*double
		xaxspan = base._xaxmax - base._xaxmin,
		yaxspan = base._yaxmax - base._yaxmin;*/
	
	//if no data is provided, return here
	if(minval == maxval && maxval==0) has_data = false;

	////Impose user-specified min and max z values
	//if( base._zaxmax > base._zaxmin){
	//	minval = base._zaxmin;
	//	maxval = base._zaxmax;
	//}
	
	//Plot each node
	if(has_data){
		wxColour fcolor;
		//plot the points
		double zscale = 1./(maxval - minval);
		for(int i=0; i<(int)data.size(); i++){
			
			double x = (base._xaxmax - data.at(i).at(0))*base._ppx + base._origin[0];
			double y = -(base._yaxmin + data.at(i).at(1))*base._ppy + base._origin[1];
			double z = (data.at(i).at(2) - minval)*zscale;

			base.ColorGradientJet(fcolor, z);
			_dc.SetPen( wxPen(fcolor, 1, wxSOLID) );
			_dc.SetBrush( wxBrush(fcolor, wxSOLID) );

			int csize = 2+(int)(data.at(i).at(3)/(double)maxsize*8);
			_dc.DrawCircle(x,y,csize);

		}
	}
	
	
	
	_dc.SetPen( wxPen( base._gray, 2, wxSOLID ) );
	_dc.SetBrush( *wxTRANSPARENT_BRUSH );
	double 
		XX[] = {base._xaxmin, base._xaxmax, base._xaxmax, base._xaxmin},
		YY[] = {base._yaxmax, base._yaxmax, base._yaxmin, base._yaxmin};
	base.DrawScaledPolygon(_dc, base._ppx, base._ppy, base._origin, XX, YY, 4);

	

	//Draw the gradient bar
	if(has_data) base.DrawColorbar(_dc, minval, maxval, zvalave);

	_dc.SelectObject( wxNullBitmap );
	
	return true;
}

bool PlotScatter::Export(PlotBase &base, wxString filename, wxBitmapType type){
	
	wxImage image = base._bitmap.ConvertToImage();
	return image.SaveFile(filename, type);
}

bool PlotScatter::ExportDataTable(PlotBase &base, wxString &path, wxString &delim, vector<vector<double> > &data){
	/* 
	Write a data table with flux information to 'path' using data separated by 'delim'
	*/

	//Calculate min/max/ave values
	double maxval=-9.e9, minval=9.e9;
	int ndx = (int)data.size();
	int ndy = (int)data.front().size();
	double qtot = 0.;
	for(int i=0; i<ndx; i++){
		for(int j=0; j<ndy; j++){
			double d = data.at(i).at(j);
			qtot += d;
			if(d > maxval) maxval = d;
			if(d < minval) minval = d;
		}
	}
	double qave = qtot/(double)(ndx*ndy);

	//Check to make sure the table has data
	if(minval == maxval){
		wxMessageBox("The receiver fluxmap is empty! Could not export data.", "Export error", wxOK);
		return true;
	}
	//try opening the file
	wxTextFile fout(path);
	if(! (fout.Exists() ? fout.Open() : fout.Create()) ){
		return false;
	}

	fout.Clear();

	//Write header
	fout.AddLine("Flux intensity plot (units kW/m2)");
	

	//write summary stats
	fout.AddLine(wxString::Format("Max. flux%s%f%skW/m2", delim, maxval, delim));
	fout.AddLine(wxString::Format("Min. flux%s%f%skW/m2", delim, minval, delim));
	fout.AddLine(wxString::Format("Ave. flux%s%f%skW/m2", delim, qave, delim));

	//determine the best number formatting for the x axis labels
	wxString xfmt;
	int nxdec = gui_util::CalcBestSigFigs( fmax(fabs(base._xaxmax),fabs(base._xaxmin)) );
	xfmt.sprintf("%s%df", "%.", max(nxdec,0));
	
	//determine the best number formatting for the y axis labels
	wxString yfmt;
	int nydec = gui_util::CalcBestSigFigs( fmax(fabs(base._yaxmax),fabs(base._yaxmin)) );
	yfmt.sprintf("%s%df", "%.", nydec);

	//write x axis values
	fout.AddLine("");
	wxString xlab =base._xlab;
	xlab.Replace(delim, ";");
	fout.AddLine(wxString::Format("%s%s -->", delim, xlab));
	wxString ylab = base._ylab;
	ylab.Replace(delim, ";");
	wxString line = wxString::Format("%s%s", ylab, delim);
	wxString tmp;
	for(int i=0; i<ndx; i++)
		line.Append(wxString::Format(xfmt, base._xaxmax - (base._xaxmax-base._xaxmin)/(double)(ndx)*(double)i ) + delim);
	fout.AddLine(line);
	line.Clear();

	for(int j=0; j<ndy; j++){
		//add y axis value - proceed top down
		line = wxString::Format(yfmt, base._yaxmax - (base._yaxmax - base._yaxmin)/(double)(ndy)*(j)+0.5) + ",";
		for(int i=0; i<ndx; i++){
			line.Append( wxString::Format("%f%s", data.at(ndx - 1 - i).at(ndy - 1 - j), delim) );
		}
		fout.AddLine(line);
	}

	fout.Write();
	fout.Close();

	return true;


}
