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
  Double_t thresh=0.0005, Int_t nToFit=4, TString myOpt="Q")
{

  TFile* file = TFile::Open(filename);
  file->cd();

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

  Double_t energyArray[4]={1275,511,307,202};//,0};
  vector<Double_t> energyVec;
	vector<Double_t> ordered;	// array of peaks found re-ordered by energy (largest first)
  for(int i=0;i<nToFit;i++) energyVec.push_back(energyArray[i]);

  char answer;
  int	fit=0;
  int nUsed=0;
  double min, max;
  TLine* tl=new TLine(0,0,1,1);
  TGraph *gr;
  TF1* linFunc = new TF1("linFunc","pol2",0,8192);
  Double_t FWHM;
  TH1I* res_h=new TH1I("res_h","Linearity Corrected Resolution",100,0,100);

	Double_t toUse[nFound];		// array of peaks to use for calibration and linearity correction
	Double_t EtoUse[nFound];

  TString str2=filename;
  // TString str2=fChain->GetCurrentFile()->GetName();
  str2.ReplaceAll("_out.root","_fits");
  gSystem->MakeDirectory(str2);
  gSystem->cd(str2);

  for(int chnID=0; chnID<1024; chnID++) {
  // for(int chnID=96; chnID<100; chnID++) {
    cout << "Processing channel " << chnID << "...";
    histo = (TH1I*)gDirectory->Get(Form("qdc%i_h",chnID));
    // histo->Draw();
    if(histo->GetEntries()>0) {
      lin_c=new TCanvas("lin_c","Linearity fits",0,0,800,800);
      // cout << "\n--------------------------\n" << histo->GetName() << endl;

      myFit = fitPeak2(histo,fitMin,fitMax,myOpt,"","");
      cent=myFit->Parameter(3);
      sigma=myFit->Parameter(4);
      lin_c->Clear();
      lin_c->cd(1);
    	gPad->SetLogy(1);
    	histo->Draw();
    	spec=new TSpectrum();
    	// histo->GetXaxis()->SetRangeUser(8,100);
    	histo->GetXaxis()->UnZoom();
      nFound=spec->Search(histo,sigma,"",thresh);

     	xPos=spec->GetPositionX();	// array of peaks found, ordered by amplitude
    	vector<Double_t> ordered;	// array of peaks found re-ordered by energy (largest first)
    	Int_t index[nFound];
    	fit=0;
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
      // lin_c->Clear();
    	// lin_c->Divide(1,3);
      // lin_c->cd(1);
      // gPad->SetLogy(1);
      // histo->Draw();

// interactively find the right peaks
      if(myOpt.Contains("I")) {
        cout << "Accept these peaks (y/n/q): ";
      	cin >> answer;

      	while(answer!='q') {
      		if(answer=='y') {
      			cout << "Using these peaks..." << endl;
      			answer='q';
      		} else if(answer=='n') {
      			cout << "Which peaks to use..." << endl;

      			for(int i=0; i<4; i++)
      			{
      				cout << "Use " << energyVec[i] << " keV peak? (y/n): ";
      				cin >> answer;
      				if(answer=='n') continue;
      				else if(answer=='y') {
      					if(fit>=nFound) {
      						cout << "No peaks left. Exiting." << endl;
      						break; // break out of 'for(int i=0; i<4; i++)'
      					}
      					while(answer!='q') {
      						tl->DrawLine(ordered[fit],min,ordered[fit],max);
      						gPad->Update();
      						cout << "	Use: " << ordered[fit] <<" ? (y/n): ";
      						cin >> answer;
      						if(answer=='y') {
      							toUse[nUsed]=ordered[fit];
      							EtoUse[nUsed]=energyVec[i];
      							fit++;
      							nUsed++;
      							break;
      						}
      						else if(answer=='n') {
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
      			for(int i=0;i<nUsed;i++) cout << EtoUse[i] << "		" << toUse[i] << endl;
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
    	histo->GetXaxis()->SetRangeUser(0,ordered[0]*1.1);
    	histo->DrawCopy();
    	for(int i=0;i<nUsed;i++) tl->DrawLine(toUse[i],min,toUse[i],max);

    	if(nUsed==0) gr=new TGraph(energyVec.size(),&ordered[0],&energyVec[0]);
      else gr=new TGraph(nUsed,toUse,EtoUse);
    	lin_c->cd(3);
    	if(nUsed==0) gr->GetXaxis()->SetLimits(0,ordered[0]*1.1);
    	else gr->GetXaxis()->SetLimits(0,toUse[0]*1.1);
    	gr->Draw("a*");
    	if(nUsed==0) gr->GetXaxis()->SetRangeUser(0,ordered[0]*1.1);
    	else gr->GetXaxis()->SetRangeUser(0,toUse[0]*1.1);
    	gr->GetYaxis()->SetRangeUser(0,1500);
    	gr->SetTitle("Calibration");
    	gr->GetXaxis()->SetTitle("channel");
    	gr->GetYaxis()->SetTitle("energy (keV)");
      TF1* linFunc = new TF1("linFunc","pol2",0,8192);
    	gr->Fit("linFunc","q");
    	gr->Draw("a*");
      gPad->Update();

      // cout << endl << "Writing results to file '" << name << "_linearity.dat'" << endl;
      out << chnID << "," << linFunc->GetParameter(0) << "," << linFunc->GetParameter(1) << "," << linFunc->GetParameter(2) << endl;
      // out << "FWHM (% of 662keV)" << endl << FWHM << endl;
      // out << "Energy (keV)	Channel" << endl;
      lin_c->SaveAs(Form("qdc_%i_linearity.pdf",chnID));

      cout << "Press any 'y' to continue: ";
      cin >> answer;
    } else cout << "No histogram." << endl;
// end of if entires>0

//    delete lin_c;
  } // end of loop over channels
  out.close();

  // cout << "Save result to file '" << name << "_linearity.dat' and " << name << "_linearity.pdf'? (y/n): ";
  // answer=0;
  // while(answer!='n') {
  //   cin >> answer;
  //   if(answer=='y') {
  //     ofstream out(name+"_linearity.dat");
  //     cout << endl << "Writing results to file '" << name << "_linearity.dat'" << endl;
  //     out << "FWHM (% of 662keV)" << endl << FWHM << endl;
  //     out << "Energy (keV)	Channel" << endl;
  //     if(nUsed==0) for(int i=0;i<nToFit;i++) out << energy[i] << "		" << ordered[i] << endl;
  //     else for(int i=0;i<nUsed;i++) out << EtoUse[i] << "		" << toUse[i] << endl;
  //     out.close();
  //
  //     lin_c->SaveAs(name+"_linearity.pdf");
  //     break;
  //   }
  //   cout << "Try again. Save results? (y/n): ";
  // }

} // end of LYSOcal

void Usage()
{
	cout << endl << "LYSOcal(TString filename, Double_t fitMin, Double_t fitMax, int nToFit=4, , Int_t nToFit=4, TString myOpt=\"Q\") \n\n"
		<< "Use 'fitMin' and 'fitMax' to set range to fit 511 keV peak in 'histo'. \n"
		<< "Use 'nToFit' to limit number of peaks to use for linearity correction (highest energy peaks will be used). \n"
		<< "'thresh' can be used to adjust minimum amplitude of peaks that will be fitted.\n"
    << "Use 'myOpt=\"I\"' to interactively select the correct peaks." << endl;
}

void usage() { Usage();}
void Help() { Usage();}
void help() { Usage();}
