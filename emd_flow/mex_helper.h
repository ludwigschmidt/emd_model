#ifndef __MEX_HELPER_H__
#define __MEX_HELPER_H__

#include <mex.h>
#include <matrix.h>
#include <cmath>
#include <vector>

bool get_double(const mxArray* raw_data, double* data) {
  int numdims = mxGetNumberOfDimensions(raw_data);
  const mwSize* dims = mxGetDimensions(raw_data);
  if (numdims != 2 || dims[0] != 1 || dims[1] != 1) {
    return false;
  }
  if (!mxIsClass(raw_data, "double")) {
    return false;
  }
  *data = static_cast<double*>(mxGetData(raw_data))[0];
  return true;
}

bool get_double_as_int(const mxArray* raw_data, int* data) {
  double tmp;
  if (get_double(raw_data, &tmp)) {
    *data = round(tmp);
    return true;
  } else {
    return false;
  }
}

bool get_bool(const mxArray* raw_data, bool* data) {
  int numdims = mxGetNumberOfDimensions(raw_data);
  const mwSize* dims = mxGetDimensions(raw_data);
  if (numdims != 2 || dims[0] != 1 || dims[1] != 1) {
    return false;
  }
  if (!mxIsClass(raw_data, "logical")) {
    return false;
  }
  *data = static_cast<bool*>(mxGetData(raw_data))[0];
  return true;
}

bool get_double_matrix(const mxArray* raw_data,
    std::vector<std::vector<double> >* data) {
  int numdims = mxGetNumberOfDimensions(raw_data);
  const mwSize* dims = mxGetDimensions(raw_data);
  if (numdims != 2) {
    return false;
  }
  if (!mxIsClass(raw_data, "double")) {
    return false;
  }
  size_t r = dims[0];
  size_t c = dims[1];
  double* data_linear = static_cast<double*>(mxGetData(raw_data));
  if (data->size() != r) {
    data->resize(r);
  }
  for (size_t ir = 0; ir < r; ++ir) {
    if ((*data)[ir].size() != c) {
      (*data)[ir].resize(c);
    }
    for (size_t ic = 0; ic < c; ++ic) {
      (*data)[ir][ic] = data_linear[ir + ic * r];
    }
  }
  return true;
}

void set_double(mxArray** raw_data, double data) {
  *raw_data = mxCreateDoubleMatrix(1, 1, mxREAL);
  *(static_cast<double*>(mxGetData(*raw_data))) = data;
}

void set_double_matrix(mxArray** raw_data,
    const std::vector<std::vector<double> >& data) {
  int numdims = 2;
  mwSize dims[2];
  size_t r = data.size();
  size_t c = data[0].size();
  dims[0] = r;
  dims[1] = c;
  *raw_data = mxCreateNumericArray(numdims, dims, mxDOUBLE_CLASS, mxREAL);
  double* result_linear = static_cast<double*>(mxGetData(*raw_data));

  for (size_t ir = 0; ir < r; ++ir) {
    for (size_t ic = 0; ic < c; ++ic) {
      result_linear[ir + ic * r] = data[ir][ic];
    }
  }
}
void set_double_matrix(mxArray** raw_data,
    const std::vector<std::vector<bool> >& data) {
  std::vector<std::vector<double> > tmp_data;
  size_t r = data.size();
  size_t c = data[0].size();
  tmp_data.resize(r);
  for (size_t ir = 0; ir < r; ++ir) {
    tmp_data[ir].resize(c);
    for (size_t ic = 0; ic < c; ++ic) {
      tmp_data[ir][ic] = data[ir][ic];
    }
  }
  set_double_matrix(raw_data, tmp_data);
}

#endif
