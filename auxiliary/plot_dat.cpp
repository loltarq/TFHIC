// plot_dat.C
// Reads a whitespace-delimited .dat file with a header row.
// Features:
//  - listColumns(filepath)
//  - drawGraph(filepath, xCol, yCol [, canvName="c_graph", drawopt="APL"])
//  - drawMultiY(filepath, xCol, "y1,y2,..." [, canvName="c_multiy", baseDrawOpt="APL"])
// Notes:
//  - Column specifiers accept exact header names or "#index" (e.g. "#0").
//  - Axis titles come from the header names.

#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TAxis.h>
#include <TROOT.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>

namespace pd_internal {

inline std::string trim(const std::string& s) {
  auto b = std::find_if_not(s.begin(), s.end(), [](unsigned char c){ return std::isspace(c); });
  auto e = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c){ return std::isspace(c); }).base();
  if (b >= e) return std::string();
  return std::string(b, e);
}

inline void split_ws(const std::string& line, std::vector<std::string>& out) {
  out.clear();
  std::istringstream iss(line);
  std::string tok;
  while (iss >> tok) out.push_back(tok);
}

// Parse a column specifier which can be a name or "#index"
inline int colIndexFromSpec(const std::string& spec, const std::vector<std::string>& headers) {
  if (!spec.empty() && spec[0] == '#') {
    try {
      int idx = std::stoi(spec.substr(1));
      if (idx >= 0 && idx < (int)headers.size()) return idx;
    } catch (...) {}
    return -1;
  }
  auto it = std::find(headers.begin(), headers.end(), spec);
  if (it == headers.end()) return -1;
  return (int)std::distance(headers.begin(), it);
}

struct Table {
  std::vector<std::string> headers;
  std::vector<std::vector<double>> cols; // cols[c][row]
  size_t nrows = 0;
};

inline bool load_table(const std::string& path, Table& t, std::string& err) {
  std::ifstream in(path);
  if (!in) { err = "Cannot open file: " + path; return false; }

  std::string headerLine;
  if (!std::getline(in, headerLine)) { err = "File is empty."; return false; }
  headerLine = trim(headerLine);

  split_ws(headerLine, t.headers);
  if (t.headers.empty()) { err = "No headers found in first line."; return false; }

  const int ncol = (int)t.headers.size();
  t.cols.assign(ncol, std::vector<double>{});
  t.nrows = 0;

  std::string line; std::vector<std::string> toks;
  size_t lineno = 1;
  while (std::getline(in, line)) {
    lineno++;
    line = trim(line);
    if (line.empty()) continue;

    toks.clear();
    split_ws(line, toks);
    if ((int)toks.size() < ncol) continue;

    bool ok = true;
    for (int c = 0; c < ncol; ++c) {
      char* endp = nullptr;
      const std::string& s = toks[c];
      double v = std::strtod(s.c_str(), &endp);
      if (endp == s.c_str() || *endp != '\0') { ok = false; break; }
      t.cols[c].push_back(v);
    }
    if (ok) t.nrows++;
  }

  if (t.nrows == 0) { err = "No numeric data rows parsed."; return false; }
  return true;
}

inline void print_headers(const std::vector<std::string>& headers) {
  std::cout << "Columns (" << headers.size() << "):\n";
  for (size_t i = 0; i < headers.size(); ++i) {
    std::cout << "  [" << i << "] " << headers[i] << "\n";
  }
}

} // namespace pd_internal

// List available columns (names and indices)
void listColumns(const char* filepath) {
  using namespace pd_internal;
  Table t; std::string err;
  if (!load_table(filepath, t, err)) { std::cerr << "Error: " << err << "\n"; return; }
  print_headers(t.headers);
}

// Draw a TGraph for (x, y) given by header names or "#index"
TGraph* drawGraph(const char* filepath,
                  const char* xcolName,
                  const char* ycolName,
                  const char* canvName = "c_graph",
                  const char* drawopt  = "APL") {
  using namespace pd_internal;
  Table t; std::string err;
  if (!load_table(filepath, t, err)) { std::cerr << "Error: " << err << "\n"; return nullptr; }

  int xi = colIndexFromSpec(xcolName, t.headers);
  int yi = colIndexFromSpec(ycolName, t.headers);
  if (xi < 0 || yi < 0) {
    std::cerr << "Unknown column(s).\n"; print_headers(t.headers); return nullptr;
    }

  auto* g = new TGraph((int)t.nrows);
  for (int i = 0; i < (int)t.nrows; ++i) g->SetPoint(i, t.cols[xi][i], t.cols[yi][i]);

  g->SetTitle(Form("%s vs %s", t.headers[yi].c_str(), t.headers[xi].c_str()));
  g->GetXaxis()->SetTitle(t.headers[xi].c_str());
  g->GetYaxis()->SetTitle(t.headers[yi].c_str());

  TCanvas* c = (TCanvas*)gROOT->FindObject(canvName);
  if (!c) c = new TCanvas(canvName, canvName, 900, 650);
  c->cd(); g->Draw(drawopt); c->Update();
  return g;
}

// Draw multiple Y columns vs a fixed X using TMultiGraph
void drawGraphMulti(const char* filepath,
                const char* xcolName,
                const char* ycolsCSV,
                const char* canvName = "c_multiy",
                const char* baseDrawOpt  = "APL",
                bool logY = false) {
  using namespace pd_internal;
  Table t; std::string err;
  if (!load_table(filepath, t, err)) { std::cerr << "Error: " << err << "\n"; return; }

  int xi = colIndexFromSpec(xcolName, t.headers);
  if (xi < 0) { std::cerr << "Unknown X column.\n"; print_headers(t.headers); return; }

  // Parse CSV of Y columns
  std::vector<std::string> yspecs;
  { std::string s = ycolsCSV, item; std::istringstream iss(s);
    while (std::getline(iss, item, ',')) { item = pd_internal::trim(item); if (!item.empty()) yspecs.push_back(item); } }
  if (yspecs.empty()) { std::cerr << "No Y columns provided (CSV empty).\n"; return; }

  TCanvas* c = (TCanvas*)gROOT->FindObject(canvName);
  if (!c) c = new TCanvas(canvName, canvName, 950, 680);
  c->cd();

  auto* mg = new TMultiGraph();
  mg->SetTitle(Form("Multiple series vs %s; %s; Value",
                    t.headers[xi].c_str(), t.headers[xi].c_str()));

  TLegend* leg = new TLegend(0.13, 0.75, 0.45, 0.93);
  leg->SetBorderSize(0);

  int colorIndex = 1;
  for (const auto& ys : yspecs) {
    int yi = colIndexFromSpec(ys, t.headers);
    if (yi < 0) { std::cerr << "Warning: unknown Y column '" << ys << "'. Skipping.\n"; continue; }

    auto* g = new TGraph((int)t.nrows);
    for (int i = 0; i < (int)t.nrows; ++i) g->SetPoint(i, t.cols[xi][i], t.cols[yi][i]);

    g->SetLineColor(colorIndex);
    g->SetMarkerColor(colorIndex);
    g->SetMarkerStyle(20);
    g->SetName(Form("g_%s_vs_%s", t.headers[yi].c_str(), t.headers[xi].c_str()));

    mg->Add(g, "LP");
    leg->AddEntry(g, t.headers[yi].c_str(), "lp");

    colorIndex++; if (colorIndex == 5) colorIndex = 6; // skip yellow if you like
  }

  if (!(mg->GetListOfGraphs() && mg->GetListOfGraphs()->GetSize() > 0)) {
    std::cerr << "No valid Y columns to draw.\n"; return;
  }

  // Build axes once so we can override the auto-range
  mg->Draw("A");

  // Compute positive ymin/ymax across all graphs to avoid auto-padding below zero
  double ymin = 1e300, ymax = -1e300, x, y;
  TIter it(mg->GetListOfGraphs()); TObject* obj;
  while ((obj = it())) {
    auto* gr = dynamic_cast<TGraph*>(obj);
    if (!gr) continue;
    for (int i = 0; i < gr->GetN(); ++i) {
      gr->GetPoint(i, x, y);
      if (std::isfinite(y)) {
        if (y > 0.0 && y < ymin) ymin = y;
        if (y > ymax) ymax = y;
      }
    }
  }
  if (ymin < 1e300) {
    mg->SetMinimum(std::max(1e-12, 0.8 * ymin)); // strictly > 0
    mg->SetMaximum(1.2 * ymax);
  }
  if (logY) gPad->SetLogy(1);

  // Label axes, draw legend, finalize
  mg->GetXaxis()->SetTitle(t.headers[xi].c_str());
  mg->GetYaxis()->SetTitle("Value");
  leg->Draw();
  c->Modified(); c->Update();
}