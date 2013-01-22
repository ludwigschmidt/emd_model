#include <vector>
#include <math.h>
#include <matrix.h>
#include <mex.h>

using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs != 3) {
    mexErrMsgTxt("Three input argument required (amplitudes, sparsity, EMD "
        "budget.");
  }
  
  if (nlhs > 4) {
    mexErrMsgTxt("Too many output arguments.");
  }

  int numdims = 0, r = 0, c = 0;
  const mwSize* dims;

  numdims = mxGetNumberOfDimensions(prhs[0]);
  dims = mxGetDimensions(prhs[0]);
  if (numdims != 2) {
    mexErrMsgTxt("Amplitudes need to be a two-dimensional array.");
  }
  r = dims[0];
  c = dims[1];
  double* a_linear = mxGetPr(prhs[0]);
  vector<vector<double> > a;
  a.resize(r);
  for (int ir = 0; ir < r; ++ir) {
    a[ir].resize(c);
    for (int ic = 0; ic < c; ++ic) {
      a[ir][ic] = a_linear[ir + ic * r];
    }
  }

  numdims = mxGetNumberOfDimensions(prhs[1]);
  dims = mxGetDimensions(prhs[1]);
  if (numdims != 2 || dims[0] != 1 || dims[1] != 1) {
    mexErrMsgTxt("Sparsity has to be a scalar.");
  }
  int k = mxGetPr(prhs[1])[0];

  numdims = mxGetNumberOfDimensions(prhs[2]);
  dims = mxGetDimensions(prhs[2]);
  if (numdims != 2 || dims[0] != 1 || dims[1] != 1) {
    mexErrMsgTxt("EMD budget has to be a scalar.");
  }
  int emd_budget = mxGetPr(prhs[2])[0];

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
