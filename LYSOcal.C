#include <TROOT.h>
#include <TFile.h>
#include <TLatex.h>
#include <TSpectrum.h>
#include <TFitResult.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TGraph.h>
#include <Riostream.h>
#include "fitPeak2.C"

void LYSOcal(TString filename,Double_t fitMin=14, Double_t fitMax=18,
  Double_t thresh=0.0005, Int_t nToFit=3, TString myOpt="Q")
{

  TFile* file = TFile::Open(filename);
  file->cd();

  TString str2=filename;
  str2.ReplaceAll("_out.root","_fits");
  gSystem->MakeDirectory(str2);
  gSystem->cd(str2);

  TString str=filename;
  str.ReplaceAll("_out.root","_linearity.csv");
  ofstream out(str);

  TCanvas* lin_c;//=new TCanvas("lin_c","Linearity fits",0,0,800,800);
  TH1I *histo;
  TFitResultPtr myFit;
  Double_t cent;
  Double_t sigma;
  TSpectrum* spec;
  Int_t nFound;
  Double_t* xPos;	// array of peaks found, ordered by amplitude

  vector<Double_t> ordered;	// vector of peaks found re-ordered by energy (largest first)
  Double_t energyArray[4]={1275,511,307,202};//,0};
  vector<Double_t> energyVec;
  if(nToFit==4) for(int i=0;i<nToFit;i++) energyVec.push_back(energyArray[i]);
  if(nToFit==3) for(int i=1;i<nToFit+1;i++) energyVec.push_back(energyArray[i]);
  if(nToFit==1) for(int i=1;i<nToFit+1;i++) energyVec.push_back(energyArray[i]);

  char answer;
  int	fit=0;
  int peaksLeft=0;
  int nUsed=0;
  double min, max;
  TLine* tl=new TLine(0,0,1,1);
  TGraph *gr;
  TF1* linFunc = new TF1("linFunc","pol2",0,8192);
  Float_t p0,p1,p2;
  TH1F* p0_h=new TH1F("p0_h","p0 coefficient",1024,0,1024);
  TH1F* p1_h=new TH1F("p1_h","p1 coefficient",1024,0,1024);
  TH1F* p2_h=new TH1F("p2_h","p2 coefficient",1024,0,1024);

  for(int chnID=0; chnID<1024; chnID++) {
    cout << "Processing channel " << chnID << "...";
    histo = (TH1I*)gDirectory->Get(Form("qdc%i_h",chnID));
    if(histo->GetEntries()>0) {
      lin_c=new TCanvas("lin_c","Linearity fits",0,0,800,800);

      myFit = fitPeak2(histo,fitMin,fitMax,myOpt,"","");
      cent=myFit->Parameter(3);
      sigma=myFit->Parameter(4);
      lin_c->Clear();
      lin_c->cd(1);
    	gPad->SetLogy(1);
    	histo->Draw();
    	spec=new TSpectrum();
    	histo->GetXaxis()->UnZoom();
      nFound=spec->Search(histo,sigma,"",thresh);
      cout << nFound << " peaks found..." << endl;

      vector<Double_t> qdcToUse;		// array of peaks to use for calibration and linearity correction
      vector<Double_t> EtoUse;

     	xPos=spec->GetPositionX();	// array of peaks found, ordered by amplitude
    	vector<Double_t> ordered;	// array of peaks found re-ordered by energy (largest first)
    	Int_t index[nFound];
    	fit=0;
      peaksLeft=nFound;
    	nUsed=0;

    	TMath::Sort(nFound,xPos,index);

    	for(int i=0;i<nFound;i++) ordered.push_back(xPos[index[i]]);
    	if(myOpt.Contains("I")) {
    		cout << endl << "First guess at peaks to use for calibration: " << endl;
    		cout << "Index	Energy (keV)	Channel" << endl;
    		for(int i=0;i<nToFit;i++) cout << i << "	" << energyVec[i] << "		" << ordered[i] << endl;
    	}

    	histo->GetXaxis()->SetRangeUser(0,ordered[0]*1.1);
    	gPad->Update();
    	if(myOpt.Contains("V")) cout << endl << gPad->GetUymax() << " " << pow(10,gPad->GetUymax()) << endl;
    	min=pow(10,gPad->GetUymin());
    	max=pow(10,gPad->GetUymax());
    	gPad->Update();
    	tl=new TLine(0,0,1,1);
      tl->SetLineColor(2);
      tl->SetLineStyle(2);
      tl->SetLineWidth(2);

// interactively find the right peaks
      if(myOpt.Contains("I")) {
        cout << "Accept these peaks (y/n/q): ";
      	cin >> answer;

      	while(answer!='q') {
      		if(answer=='y') {
            for(int i=0;i<nToFit;i++) {
              qdcToUse.push_back(ordered[i]);
              EtoUse.push_back(energyVec[i]);
              nUsed=nToFit;
            }
      			cout << "Using these peaks..." << endl;
      			answer='q';
      		} else if(answer=='n') {
      			cout << "Which peaks to use..." << endl;

      			for(int i=0; i<nToFit; i++)
      			{
      				cout << "Use " << energyVec[i] << " keV? (y/n): ";
      				cin >> answer;
      				if(answer=='n') continue;
      				else if(answer=='y') {
      					while(answer!='q') {
                // if(fit>=nFound) {
                  if(peaksLeft<1) {
                    cout << "No peaks left. Exiting." << endl;
                    break; // break out of 'for(int i=0; i<4; i++)'
                  }
      						tl->DrawLine(ordered[fit],min,ordered[fit],max);
      						gPad->Update();
      						cout << "	Use channel: " << ordered[fit] <<" ? (y/n): ";
      						cin >> answer;
      						if(answer=='y') {
                    peaksLeft--;
      							qdcToUse.push_back(ordered[fit]);
      							EtoUse.push_back(energyVec[i]);
      							fit++;
      							nUsed++;
      							break;
      						}
      						else if(answer=='n') {
                    peaksLeft--;
      							fit++;
      							continue;
      						}
      						else if(answer=='q') break;
      						else cout << "try again" << endl;
      					}
      					if(answer=='q') break;
      					else continue;
      				}
      				else if(answer=='q') break;
      			}
      			cout << endl << "New peaks to use for calibration: " << endl;
      			cout << "Energy (keV)	Channel" << endl;
      			for(int i=0;i<nUsed;i++) cout << EtoUse[i] << "		" << qdcToUse[i] << endl;
      			break;
      		} // end of 'if(answer=='n')'
      		else {
      			cout << "Try again (y/n): ";
      			cin >> answer;
      		}
      	} // end of 'while(answer!='q')'
        // if(answer=='q') continue;
      } // end of interctive bit

      lin_c->Clear();
      lin_c->Divide(1,3);

     	lin_c->cd(1);
    	histo->GetXaxis()->SetRangeUser(fitMin*0.75,fitMax*1.25);
    	histo->DrawCopy();

    	lin_c->cd(2);
    	gPad->SetLogy(1);
    	histo->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
    	histo->DrawCopy();
    	for(int i=0;i<nUsed;i++) tl->DrawLine(qdcToUse[i],min,qdcToUse[i],max);

      gr=new TGraph(qdcToUse.size(),&qdcToUse[0],&EtoUse[0]);
    	lin_c->cd(3);
    	gr->GetXaxis()->SetLimits(0,qdcToUse[0]*1.1);
    	gr->Draw("a*");
    	gr->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
      gr->GetYaxis()->SetRangeUser(0,EtoUse[0]*1.1);
    	gr->SetTitle("Calibration");
    	gr->GetXaxis()->SetTitle("channel");
    	gr->GetYaxis()->SetTitle("energy (keV)");
      TF1* linFunc = new TF1("linFunc","pol1",0,8192);
      // TF1* linFunc = new TF1("linFunc","pol2",0,8192);
      if(nToFit==1) linFunc->FixParameter(0,0);
    	gr->Fit("linFunc","q");
    	gr->Draw("a*");
      gPad->Update();

      // cout << endl << "Writing results to file '" << name << "_linearity.dat'" << endl;
      p0=linFunc->GetParameter(0);
      p1=linFunc->GetParameter(1);
      // p2=linFunc->GetParameter(2);
      p2=0;
      out << chnID << "," << p0 << "," << p1 << "," << p2 << endl;
      lin_c->SaveAs(Form("qdc_%i_linearity.pdf",chnID));
      p0_h->Fill(chnID,p0);
      p1_h->Fill(chnID,p1);
      p2_h->Fill(chnID,p2);

      cout << "Press any 'y' to continue: ";
      cin >> answer;
    } else cout << "No histogram." << endl;
// end of if entires>0

  } // end of loop over channels
  out.close();

  TCanvas* coef_c=new TCanvas("coef_c","Calibration Coeffiecnts");
  coef_c->Divide(1,3);
  coef_c->cd(1);
  p0_h->Draw("");
  coef_c->cd(2);
  p1_h->Draw("");
  coef_c->cd(3);
  p2_h->Draw("");
  coef_c->SaveAs("calibrationCoefficients.pdf");

} // end of LYSOcal

void Usage()
{
	cout << endl << "LYSOcal(TString filename, Double_t fitMin, Double_t fitMax, Double_t thresh=0.0005, Int_t nToFit=4, TString myOpt=\"Q\") \n\n"
		<< "Use 'fitMin' and 'fitMax' to set range to fit 511 keV peak in 'histo'. \n"
		<< "Use 'nToFit' to limit number of peaks to use for linearity correction. \n"
    << "'nToFit=3' will skip the 1275 keV peak. 'nToFit=1' uses only 511 keV and fixes the intercept at 0. \n"
		<< "'thresh' can be used to adjust minimum amplitude of peaks that will be fitted.\n"
    << "Use 'myOpt=\"I\"' to interactively select the correct peaks." << endl;
}

void usage() { Usage();}
void Help() { Usage();}
void help() { Usage();}
