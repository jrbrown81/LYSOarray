#include <TROOT.h>
#include <TFile.h>
#include <TBranch.h>
#include <TString.h>
#include <TH2.h>
#include <TTree.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// const TString pathRoot_in_calib = "/shared/storage/physhad/Medical/LYSOarray/data/newActiveScatSetup/highGainCal";

vector<Double_t> calCoef[1024];

void writeFile(TString,Long64_t);
// void writeFile(Long64_t,Int_t);
Bool_t readCalFile(TString,Bool_t);

void rawDataToSortedScat(TString name="", Long64_t toProcess = 0, TString calFileName="calFile.csv", Bool_t recalibrate=0){

  Bool_t calibrated=0;

  if(recalibrate) {
		cout << "Forcing recalibration..." << endl;
		cout << "Calibraion warning will be suppressed." << endl;
		calibrated=readCalFile(calFileName, recalibrate);
		// calibrated=false;
	} else if(!calibrated) calibrated=readCalFile(calFileName, recalibrate);

  Int_t runs [1] = {1};
  if(name.Sizeof()!=0){
    cout << "Processing file: " << name << endl;
    writeFile(name,toProcess);
  }
  else {
    for(Int_t i=0; i<(sizeof(runs)/sizeof(*runs));i++){
      // cout << i << endl;
      name = Form("run%i_single.root",runs[i]);
      cout << "Processing file: " << name << endl;
      writeFile(name,toProcess);
      // writeFile(toProcess,runs[i]);
    }
  }
}

void writeFile(TString name, Long64_t toProcess){
// void writeFile(Long64_t toProcess,Int_t runNum){

  // TString calibFileName = Form("%s/calFile.csv",
  //                              pathRoot_in_calib.Data());
  //
  // ifstream calibFile(calibFileName);
  //
  // std::string line, val;
  // std::vector<std::vector<double>> calibArray;
  //
  // while (std::getline (calibFile, line)) {
  //   std::vector<double> v;
  //   std::stringstream s (line);
  //   while (getline (s, val, ','))
  //   v.push_back (std::stod (val));
  //   calibArray.push_back (v);
  // }

int channel2column[128] = {8, 7, 5, 7, 6, 8, 5, 7, 6, 4, 8, 5, 7, 6, 8, 5, 7, 8,
                            8, 7, 8, 7, 7, 8, 5, 6, 5, 6, 6, 6, 5, 5, 6, 4, 3, 3,
                            4, 3, 3, 4, 3, 4, 1, 2, 1, 2, 2, 1, 1, 2, 4, 1, 2, 2,
                            1, 1, 2, 3, 3, 4, 1, 3, 4, 2, 1, 2, 4, 2, 3, 1, 4, 2,
                            3, 5, 1, 4, 2, 3, 1, 4, 2, 1, 1, 2, 1, 2, 2, 1, 4, 3,
                            4, 3, 3, 3, 4, 4, 3, 5, 6, 6, 5, 6, 6, 5, 6, 5, 8, 7,
                            8, 7, 7, 8, 8, 7, 5, 8, 7, 7, 8, 8, 7, 6, 6, 5, 8, 6,
                            5, 7};

int channel2row[128] = {7, 7, 6, 8, 8, 5, 8, 6, 7, 5, 8, 5, 5, 6, 6, 7, 2, 1, 2,
                       1, 4, 4, 3, 3, 1, 1, 2, 5, 2, 3, 3, 4, 4, 4, 4, 3, 3, 2,
                       5, 2, 1, 1, 4, 4, 3, 2, 3, 2, 1, 1, 8, 5, 5, 6, 6, 7, 7,
                       8, 6, 7, 8, 7, 6, 8, 10, 10, 11, 9, 9, 12, 9, 11, 10, 12,
                       9, 12, 12, 11, 11, 10, 15, 16, 15, 16, 13, 13, 14, 14, 16,
                       16, 15, 12, 15, 14, 14, 13, 13, 13, 13, 14, 14, 15, 12, 15,
                       16, 16, 13, 13, 14, 15, 14, 15, 16, 16, 9, 12, 12, 11, 11,
                       10, 10, 9, 11, 10, 9, 10, 11, 9};

Double_t pitch = 3.36; // mm

// TString fname = Form("run%i_single.root",runNum);
// TFile *f = new TFile(fname);
TFile *f = new TFile(name);
TTree *t1 = (TTree*)f->Get("data");

  // Int_t nbOfEntries = t1->GetEntries();
  // cout << "Number of time windows = " << nbOfEntries << endl;

Float_t         step1;	// value of parameter 1 (when doing parameter scans)
Float_t         step2;	// value of parameter 2 (when doing parameter scans)
Long64_t        time;	// time of event, in picoseconds
UInt_t          channelID;
Float_t         tot;		// In QDC mode this is the integration time in nanoseconds
Float_t         energy;
UShort_t        tacID;
Int_t           xi;
Int_t           yi;
Float_t         x;
Float_t         y;
Float_t         z;
Float_t         tqT;	// the fine timing of the chn (crossing time threshold), in TDC clock units (200 MHz => 5ns
Float_t         tqE;

// List of branches
TBranch        *b_step1;   //!
TBranch        *b_step2;   //!
TBranch        *b_time;   //!
TBranch        *b_channelID;   //!
TBranch        *b_tot;   //!
TBranch        *b_energy;   //!
TBranch        *b_tacID;   //!
TBranch        *b_xi;   //!
TBranch        *b_yi;   //!
TBranch        *b_x;   //!
TBranch        *b_y;   //!
TBranch        *b_z;   //!
TBranch        *b_tqT;   //!
TBranch        *b_tqE;   //!

t1->SetBranchAddress("step1", &step1, &b_step1);
t1->SetBranchAddress("step2", &step2, &b_step2);
t1->SetBranchAddress("time", &time, &b_time);
t1->SetBranchAddress("channelID", &channelID, &b_channelID);
t1->SetBranchAddress("tot", &tot, &b_tot);
t1->SetBranchAddress("energy", &energy, &b_energy);
t1->SetBranchAddress("tacID", &tacID, &b_tacID);
t1->SetBranchAddress("xi", &xi, &b_xi);
t1->SetBranchAddress("yi", &yi, &b_yi);
t1->SetBranchAddress("x", &x, &b_x);
t1->SetBranchAddress("y", &y, &b_y);
t1->SetBranchAddress("z", &z, &b_z);
t1->SetBranchAddress("tqT", &tqT, &b_tqT);
t1->SetBranchAddress("tqE", &tqE, &b_tqE);

  Long64_t eventCounter=0;
  Int_t AM_e;
  Float_t E;
  Int_t index=0;
  Long64_t EventTime;
  Int_t          hits=0;
  Int_t          hitsAM0=0;
  Int_t          hitsAM1=0;

  // vector<Float_t> energyVector;
  vector<Float_t> energyVectorAM0;
  vector<Float_t> energyVectorAM1;
  vector<Int_t> amVector;
  vector<Int_t> channelVector;
  vector<Float_t> xVector;
  vector<Float_t> yVector;
  vector<Float_t> timeVector;
  vector<Float_t> timeDiff_v;

  Double_t energyAM0=0;
  Double_t energyAM1=0;
  Double_t energyScat=0;

  TString rfileOut = "sortedData_" + name;
  // TString rfileOut = Form("sortedData_run%i_20ns_sample.root",runNum);
  TFile *f2 = new TFile(rfileOut,"recreate");
  TTree *tree2 = new TTree("tree2","LYSO Tree");
     // tree2->Branch("AM",&AM_e,"AM/I");
     // tree2->Branch("channelID",&channelID,"channelID/I");
     // tree2->Branch("E",&E,"E/F");
     // tree2->Branch("time",&time,"time/I");
     // tree2->Branch("xi",&xi);
     // tree2->Branch("yi",&yi);
     tree2->Branch("EventTime",&EventTime);
     tree2->Branch("hits",&hits);
     tree2->Branch("hitsAM0",&hitsAM0);
     tree2->Branch("hitsAM1",&hitsAM1);
     tree2->Branch("energyAM0",&energyAM0);
     tree2->Branch("energyAM1",&energyAM1);
     tree2->Branch("energyScat",&energyScat);
     // tree2->Branch("energyVector",&energyVector);
     tree2->Branch("energyVectorAM0",&energyVectorAM0);
     tree2->Branch("energyVectorAM1",&energyVectorAM1);
     tree2->Branch("amVector",&amVector);
     tree2->Branch("channelVector",&channelVector);
     tree2->Branch("xVector",&xVector);
     tree2->Branch("yVector",&yVector);
     tree2->Branch("timeVector",&timeVector);
     tree2->Branch("timeDiff_v",&timeDiff_v);

    // TTree *treeEvents =new TTree("treeEvents","Practice Tree");
    //   treeEvents->Branch("Energy", &energy);
    //   treeEvents->Branch("EventTime", &EventTime);

Long64_t TimeWindow = 20.; //Units in nanoseconds
typedef struct event_struct {
  Float_t x;
  Float_t y;
  Float_t e;
}
  event_t;

event_t thisEvent;

Long64_t nentries = t1->GetEntriesFast();
if (toProcess == 0) toProcess = nentries;

cout << "Entries Found " << nentries << endl;
if (toProcess != 0)
  cout << "Processing " << toProcess << " entries " << endl;
else cout << "Processing all entries " << endl;
Long64_t jentry = 0;
for (jentry=0; jentry<toProcess;jentry++) {
  if(jentry!=0 && jentry%100000==0) cout << (double)jentry/(double)toProcess*100 << "% done \r" << flush;
  // if(jentry!=0 && jentry%1000000==0) cout << (double)jentry/(double)toProcess*100 << " percent done \r" << endl;
  t1->GetEntry(jentry);
  if(jentry==0) EventTime=time;

  thisEvent.x = -1.;
  thisEvent.y = -1.;
  thisEvent.e=0;

  // for(int i=0;i<calCoef[channelID].size();i++) eCal+=calCoef[channelID][i]*pow(energy,i);
  for(int i=0;i<calCoef[channelID].size();i++) thisEvent.e+=calCoef[channelID][i]*pow(energy,i);

    // thisEvent.e=calibArray[channelID-384][0]*energy*energy
    //               + calibArray[channelID-384][1]*energy
    //               + calibArray[channelID-384][2];

    if(channelID<128){
      if(channelID==21) AM_e=2;
       thisEvent.x = channel2column[channelID]-1;
       thisEvent.y = channel2row[channelID]-1;
    }
    else if(channelID<256){
       thisEvent.x = channel2column[channelID-128]-1;
       thisEvent.y = channel2row[channelID-128]-1;
    }
    else if(channelID<384){
       AM_e = 0;
       thisEvent.x = channel2column[channelID-256]-1;
       thisEvent.y = channel2row[channelID-256]-1;
    }
    else if(channelID<512) {
       AM_e = 0;
       thisEvent.x = channel2column[channelID-384]-1;
       thisEvent.y = channel2row[channelID-384]-1;
    }
    else if(channelID<640) {
       AM_e = 1;
       thisEvent.x = channel2column[channelID-512]-1;
       thisEvent.y = channel2row[channelID-512]-1;
    }
    else if(channelID<768) {
       AM_e = 1;
       thisEvent.x = channel2column[channelID-640]-1;
       thisEvent.y = channel2row[channelID-640]-1;
    }
    else if(channelID<896) {
       thisEvent.x = channel2column[channelID-768]-1;
       thisEvent.y = channel2row[channelID-768]-1;
    }
    else if(channelID<1024) {
       thisEvent.x = channel2column[channelID-896]-1;
       thisEvent.y = channel2row[channelID-896]-1;
    }
    eventCounter++;

  E = thisEvent.e;
  xi = thisEvent.x;
  yi = thisEvent.y;
  //tree2->Fill();

  //cout << "E value" << E << endl;
  //cout << "xi value" << xi << endl;
  //cout << "yi value" << yi << endl;
  // if(time<EventTime+TimeWindow*1000){
    // energyVector.push_back(E);
    // amVector.push_back(AM_e);
    // channelVector.push_back(channelID);
    // xVector.push_back(xi);
    // yVector.push_back(yi);
    // timeVector.push_back(time);
    // hits++;
    // if(AM_e==0) energyAM0+=E;
    // if(AM_e==1) energyAM1+=E;
  // } else {
  if(time>(EventTime+TimeWindow*1000)){
    // treeEvents->Fill();
    // tree2->Fill();
    tree2->Fill();
    // energyVector.clear();
    energyVectorAM0.clear();
    energyVectorAM1.clear();
    amVector.clear();
    channelVector.clear();
    xVector.clear();
    yVector.clear();
    timeVector.clear();
    timeDiff_v.clear();
    hits=0;
    hitsAM0=0;
    hitsAM1=0;
    energyAM0=0;
    energyAM1=0;
    energyScat=0;
    EventTime=time;
    // energyVector.push_back(E);
    // amVector.push_back(AM_e);
    // channelVector.push_back(channelID);
    // xVector.push_back(xi);
    // yVector.push_back(yi);
    // timeVector.push_back(time);
    // hits++;
    // if(AM_e==0) energyAM0+=E;
    // if(AM_e==1) energyAM1+=E;
  }
  // energyVector.push_back(E);
  if(AM_e==0) energyVectorAM0.push_back(E);
  else if(AM_e==1) energyVectorAM1.push_back(E);
  amVector.push_back(AM_e);
  channelVector.push_back(channelID);
  xVector.push_back(xi);
  yVector.push_back(yi);
  timeVector.push_back(time);
  timeDiff_v.push_back(time-EventTime);
  hits++;
  if(AM_e==0) {
    energyAM0+=E;
    hitsAM0++;
  } else if(AM_e==1) {
    energyAM1+=E;
    hitsAM1++;
  } else if(AM_e==2) energyScat=E;
  // f2->cd();
  // treeEvents->Write();

  } // End of loop over entries
  tree2->Write();
  f2->Close();
  cout << "Output written to file: " << rfileOut << endl;
}

// void readCalFile(TString calFileName, Bool_t recalibrate)
Bool_t readCalFile(TString calFileName, Bool_t recalibrate)
{
// Read calibration file
	string line, word;
	Int_t col=0;
	ifstream calFile(calFileName);

	Int_t chn=-1;
	Int_t nCalChns=0;
	// for(int i=0;i<1024;i++) calCoef[i][1]=0;

	if(calFile.is_open()){
		cout << "Reading calibration from file: " << calFileName << endl;
		while(getline(calFile,line)){
			nCalChns++;
			istringstream iss(line);
			while(getline(iss,word,',')){
				if(col==0) {
					chn=(Int_t)stoi(word);
					if(calCoef[chn].size()!=0) {
						if(!recalibrate) cerr << "Warning! Channel " << chn << " calibration is being overwritten." << endl;
						calCoef[chn].clear();
					}
				}
				else calCoef[chn].push_back((Double_t)stof(word));
				// else calCoef[chn][col-1]=(Double_t)stof(word);
				col++;
			}
			chn=-1;
			col=0;
		}
		cout << nCalChns << " calibration channels read." << endl;
	} else {
    cout << "Calibtaion file: " << calFileName << " not found!" << endl;
    return 0;
  }
	// for(int i=0;i<1024;i++) {
	// 	if(calibrated[i]==1) {
	// 		cout << i << " ";
	// 		for(int j=0;j<3;j++) cout << calCoef[i][j] << " ";
	// 		cout << endl;
	// 	}
	// }
	// calibrated=1;
  return 1;
}
