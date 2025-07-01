#include "include/io_utils.h"
#include "include/blastwave_utils.h"

void test() {
  auto infos = get_integration_info();
  auto hPt = computePtSpectrum(infos[0][0], 1.0);
  hPt->Draw();
}