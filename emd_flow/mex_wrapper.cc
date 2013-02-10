#include <vector>

#include <math.h>
#include <matrix.h>
#include <mex.h>

#include "mex_helper.h"
#include "emd_flow.h"
#include "emd_flow_network_factory.h"

using namespace std;

void output_function(const char* s) {
  mexPrintf(s);
  mexEvalString("drawnow;");
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs < 3) {
    mexErrMsgTxt("At least three input argument required (amplitudes, sparsity,"
        " EMD budget.");
  }
  if (nlhs > 4) {
    mexErrMsgTxt("Too many output arguments.");
  }
  
  vector<vector<double> > a;
  if (!get_double_matrix(prhs[0], &a)) {
    mexErrMsgTxt("Amplitudes need to be a two-dimensional double array.");
  }

  int k = 0;
  if (!get_double_as_int(prhs[1], &k)) {
    mexErrMsgTxt("Sparsity has to be a double scalar.");
  }

  int emd_budget = 0;
  if (!get_double_as_int(prhs[2], &emd_budget)) {
    mexErrMsgTxt("EMD budget has to be a double scalar.");
  }

  bool verbose = false;
  if (nrhs == 4) {
    if (!get_bool(prhs[3], &verbose)) {
      mexErrMsgTxt("Verbose flag has to be a boolean scalar.");
    }
 
 }
  vector<vector<bool> > result;
  int emd_cost;
  double amp_sum;
  double final_lambda;

  emd_flow(a, k, emd_budget, &result, &emd_cost, &amp_sum, &final_lambda,
      EMDFlowNetworkFactory::kShortestAugmentingPath, output_function, verbose);
//      EMDFlowNetworkFactory::kLemonCapacityScaling, output_function, verbose);

  if (nlhs >= 1) {
    set_double_matrix(&(plhs[0]), result);
  }

  if (nlhs >= 2) {
    set_double(&(plhs[1]), emd_cost);
  }

  if (nlhs >= 3) {
    set_double(&(plhs[2]), amp_sum);
  }

  if (nlhs >= 4) {
    set_double(&(plhs[3]), final_lambda);
  }

  /*
  mexPrintf("r = %d, c = %d, k = %d, EMD budget = %d\n", r, c, k, emd_budget);
  for (int ii = 0; ii < r; ++ii) {
    for (int jj = 0; jj < c; ++jj) {
      mexPrintf("%lf ", a[ii][jj]);
    }
    mexPrintf("\n");
  }
  */
}
