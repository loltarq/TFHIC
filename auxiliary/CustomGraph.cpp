//#include "TString.h"
//#include "TFile.h"
//#include "TTree.h"
//#include "TCanvas.h"

using namespace std;

string dtos(double d) {
    std::stringstream ss;
    ss << d;

    return ss.str();
}

void CustomGraph()
{

    std::vector<string> filenames;
    std::vector<double> kscan = {1.,1.6,3.};

    filenames.push_back("ratios_CE_scan_k1.6.dat");
    filenames.push_back("ratios_CE_scan_k1.dat");
    filenames.push_back("ratios_CE_scan_k3.dat");

    vector<string> data[3];
    int counter = 0;

    for (auto filename : filenames)
    {
        ifstream inf(filename.c_str());
        string line;
        if (inf.good())
        {
            while (getline(inf, line))
            {
                if(line.size() == 0)
                    break;

                istringstream iss(line);
                string substring;
                while(iss >> substring)
                {
                        data[counter].push_back(substring);
                }
            }
        }
        counter++;
        inf.close();
    }



    for (int i = 0; i<3; i++)
    {
        vector<double> ratio;

        for (int j = 6; j < data[i].size(); j+=4)
        {
            //cout << data[i][j+1] << " " << data[i][j] << endl;
            ratio.push_back(stod(data[i][j+1])/stod(data[i][j]));
        }

        string outfilename = "ratios_CE_scan_k" + dtos(i) + ".dat";
        ofstream fout(outfilename.c_str(),ofstream::out | ofstream::trunc);

        fout << setw(15) << data[i][0] << setw(15) << data[i][1] << setw(15) << "H3La/He3|La/P" << endl;
        for (int j = 4; j < data[i].size(); j+=4)
        {
            fout << setw(15) << data[i][j] << setw(15) << data[i][j+1] << setw(15) << ratio[j/4 - 1] << endl;
        }
        fout.close();
    }



}
    