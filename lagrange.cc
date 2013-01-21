#include <vector>
#include <cstdio>
#include <cmath>

#include <lemon/list_graph.h>
#include <lemon/capacity_scaling.h>
#include <boost/program_options.hpp>


using namespace lemon;
using namespace std;
namespace po = boost::program_options;


// rows, columns
int r, c;
// sparsity
int k;
// EMD bound
int emd_bound;
// amplitudes
std::vector<std::vector<double> > a;
// nodes corresponding to the matrix entries
std::vector<std::vector<ListDigraph::Node> > innode;
std::vector<std::vector<ListDigraph::Node> > outnode;
// arcs from innodes to outnodes
std::vector<std::vector<ListDigraph::Arc> > nodearcs;
// arcs corresponding to the EMD
std::vector<std::vector<std::vector<ListDigraph::Arc> > > colarcs;
// graph
ListDigraph g;
ListDigraph::ArcMap<int > capacity(g);
ListDigraph::ArcMap<double> cost(g);

typedef CapacityScaling<ListDigraph, int, double> AlgType;

void apply_lambda(double lambda);
int extract_emd_cost(const AlgType& alg);
double extract_amp_sum(const AlgType& alg);

int main(int argc, char** argv)
{
  po::options_description desc("Allowed options");
  desc.add_options()
      ("matrix_output", po::value<string>(), "File for binary output matrix")
      ("square_amplitudes", "Square all input amplitudes");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm); 

  scanf("%d %d %d %d", &r, &c, &k, &emd_bound);

  a.resize(r);
  for (int ii = 0; ii < r; ++ii) {
    a[ii].resize(c);
    for (int jj = 0; jj < c; ++jj) {
      scanf("%lg", &(a[ii][jj]));
    }
  }

  if (vm.count("square_amplitudes")) {
    fprintf(stderr, "Squaring all amplitudes ...\n");
    for (int ii = 0; ii < r; ++ii) {
      for (int jj = 0; jj < c; ++jj) {
        a[ii][jj] = a[ii][jj] * a[ii][jj];
      }
    }
  }

  fprintf(stderr, "r = %d,  c = %d,  k = %d,  emd_bound = %d\n", r, c, k,
      emd_bound);

  // source
  ListDigraph::Node s = g.addNode();
  // sink
  ListDigraph::Node t = g.addNode();

  // create two nodes for each entry in the matrix
  innode.resize(r);
  outnode.resize(r);
  for (int ii = 0; ii < r; ++ii) {
    innode[ii].resize(c);
    outnode[ii].resize(c);
    for (int jj = 0; jj < c; ++jj) {
      innode[ii][jj] = g.addNode();
      outnode[ii][jj] = g.addNode();
    }
  }

  // add arcs from innodes to outnodes
  nodearcs.resize(r);
  for (int ii = 0; ii < r; ++ii) {
    nodearcs[ii].resize(c);
    for (int jj = 0; jj < c; ++jj) {
      nodearcs[ii][jj] = g.addArc(innode[ii][jj], outnode[ii][jj]);
      cost[nodearcs[ii][jj]] = - abs(a[ii][jj]);
      capacity[nodearcs[ii][jj]] = 1;
    }
  }

  // add arcs from source to column 1
  for (int ii = 0; ii < r; ++ii) {
    ListDigraph::Arc a = g.addArc(s, innode[ii][0]);
    cost[a] = 0;
    capacity[a] = 1;
  }

  // add arcs from column c to sink
  for (int ii = 0; ii < r; ++ii) {
    ListDigraph::Arc a = g.addArc(outnode[ii][c - 1], t);
    cost[a] = 0;
    capacity[a] = 1;
  }

  // add arcs between columns
  colarcs.resize(r);
  for (int row = 0; row < r; ++row) {
    colarcs[row].resize(c - 1);
    for (int col = 0; col < c - 1; ++col) {
      colarcs[row][col].resize(r);
      for (int dest = 0; dest < r; ++dest) {
        colarcs[row][col][dest] =
            g.addArc(outnode[row][col], innode[dest][col + 1]);
        cost[colarcs[row][col][dest]] = abs(row - dest);
        capacity[colarcs[row][col][dest]] = 1;
      }
    }
  }

  fprintf(stderr, "The graph has %d nodes and %d arcs.\n", countNodes(g),
      countArcs(g));

  // cost scaling
  AlgType alg(g);
  alg.upperMap(capacity);
  alg.stSupply(s, t, k);

  // make lambda larger until we find a solution that fits into the EMD budget
  fprintf(stderr, "Finding large enough value of lambda ...\n");
  double lambda_high = 0.01;
  while (true) {
    apply_lambda(lambda_high);
    alg.costMap(cost);

    alg.run();
    int emd_cost = extract_emd_cost(alg);

    fprintf(stderr, "l: %lf  emd: %d\n", lambda_high, emd_cost);

    if (emd_cost <= emd_bound) {
      break;
    } else {
      lambda_high = lambda_high * 2;
    }
  }

  // binary search on lambda
  fprintf(stderr, "Binary search on lambda ...\n");
  double lambda_low = 0;
  double lambda_eps = 0.00001;
  while(lambda_high - lambda_low > lambda_eps) {
    double cur_lambda = (lambda_high + lambda_low) / 2;
    apply_lambda(cur_lambda);
    alg.costMap(cost);

    alg.run();
    int emd_cost = extract_emd_cost(alg);

    fprintf(stderr, "l_cur: %lf  (l_low: %lf, l_high: %lf)  "
        "emd: %d\n", cur_lambda, lambda_low, lambda_high, emd_cost);

    if (emd_cost <= emd_bound) {
      lambda_high = cur_lambda;
    } else {
      lambda_low = cur_lambda;
    }
  }

  apply_lambda(lambda_high);
  alg.costMap(cost);

  alg.run();
  int emd_cost = extract_emd_cost(alg);
  double amp_sum = extract_amp_sum(alg);

  fprintf(stderr, "Final l: %lf, total cost: %lf, amp sum: %lf, EMD cost: %d\n",
      lambda_high, alg.totalCost(), amp_sum, emd_cost);

  for (int jj = 0; jj < c; ++jj) {
    fprintf(stderr, "col %d:\n", jj + 1);
    for (int ii = 0; ii < r; ++ii) {
      if (alg.flow(nodearcs[ii][jj]) > 0) {
        fprintf(stderr, " row %d, amplitude %lf\n", ii + 1, a[ii][jj]);
      }
    }
  }

  printf("%lf\n", amp_sum);

  if (vm.count("matrix_output")) {
    string output_file_name = vm["matrix_output"].as<string>();
    FILE* output_file = fopen(output_file_name.c_str(), "w");
    for (int ii = 0; ii < r; ++ii) {
      for (int jj = 0; jj < c; ++jj) {
        if (alg.flow(nodearcs[ii][jj]) > 0) {
          fprintf(output_file, "1 ");
        } else {
          fprintf(output_file, "0 ");
        }
      }
      fprintf(output_file, "\n");
    }
    fclose(output_file);
  }

  return 0;
}

void apply_lambda(double lambda) {
  for (int row = 0; row < r; ++row) {
    for (int col = 0; col < c - 1; ++col) {
      for (int dest = 0; dest < r; ++dest) {
        cost[colarcs[row][col][dest]] = lambda * abs(row - dest);
      }
    }
  }
}

int extract_emd_cost(const AlgType& alg) {
  int emd_cost = 0;
  for (int row = 0; row < r; ++row) {
    for (int col = 0; col < c - 1; ++col) {
      for (int dest = 0; dest < r; ++dest) {
        if (alg.flow(colarcs[row][col][dest]) > 0) {
          emd_cost += abs(row - dest);
          if (alg.flow(colarcs[row][col][dest]) != 1) {
            printf("ERROR: nonzero flow on a column edge is not 1.\n");
          }
        }
      }
    }
  }
  return emd_cost;
}

double extract_amp_sum(const AlgType& alg) {
  double amp_sum = 0;
  for (int row = 0; row < r; ++row) {
    for (int col = 0; col < c; ++col) {
      if (alg.flow(nodearcs[row][col]) > 0) {
        amp_sum += abs(a[row][col]);
        if (alg.flow(nodearcs[row][col]) != 1) {
          printf("ERROR: nonzero flow on a node edge is not 1.\n");
        }
      }
    }
  }
  return amp_sum;
}
