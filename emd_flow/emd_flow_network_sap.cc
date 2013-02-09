#include "emd_flow_network_sap.h"

#include <cmath>
#include <cstdio>

using namespace std;

EMDFlowNetworkSAP::EMDFlowNetworkSAP(
    const std::vector<std::vector<double> >& amplitudes) : a_(amplitudes) {
  r_ = amplitudes.size();
  c_ = amplitudes[0].size();

  a_.resize(r_);
  for (int row = 0; row < r_; ++row) {
    a_[row].resize(c_);
    for (int col = 0; col < c_; ++col) {
      a_[row][col] = amplitudes[row][col];
    }
  }

  // source and sink
  s_ = 0;
  t_ = 1;

  // potentials
  int num_nodes = 2 + 2 * r_ * c_;
  potential_.resize(num_nodes);
  outgoing_edges_.resize(num_nodes);

  // add arcs from source to column 1
  for (int ii = 0; ii < r_; ++ii) {
    EdgeIndex next_edge_index = e_.size();
    Edge forward(innode_index(ii, 0), 1, 0.0, true, next_edge_index + 1);
    Edge backward(s_, 0, 0.0, false, next_edge_index);
    e_.push_back(forward);
    e_.push_back(backward);
    outgoing_edges_[s_].push_back(next_edge_index);
    outgoing_edges_[innode_index(ii, 0)].push_back(next_edge_index + 1);
  }

  // add arcs from column c to sink
  for (int ii = 0; ii < r_; ++ii) {
    EdgeIndex next_edge_index = e_.size();
    Edge forward(t_, 1, 0.0, true, next_edge_index + 1);
    Edge backward(outnode_index(ii, c_ - 1), 0, 0.0, false, next_edge_index);
    e_.push_back(forward);
    e_.push_back(backward);
    outgoing_edges_[outnode_index(ii, c_ - 1)].push_back(next_edge_index);
    outgoing_edges_[t_].push_back(next_edge_index + 1);
  }

  // add arcs from innodes to outnodes
  node_edges_.resize(r_);
  for (int ii = 0; ii < r_; ++ii) {
    node_edges_[ii].resize(c_);
    for (int jj = 0; jj < c_; ++jj) {
      EdgeIndex next_edge_index = e_.size();
      Edge forward(outnode_index(ii, jj), 1, -abs(a_[ii][jj]), true,
          next_edge_index + 1);
      Edge backward(innode_index(ii, jj), 0, abs(a_[ii][jj]), false,
          next_edge_index);
      e_.push_back(forward);
      e_.push_back(backward);
      node_edges_[ii][jj] = next_edge_index;
      outgoing_edges_[innode_index(ii, jj)].push_back(next_edge_index);
      outgoing_edges_[outnode_index(ii, jj)].push_back(next_edge_index + 1);
    }
  }

  // add arcs between columns
  emd_edges_.resize(r_);
  for (int row = 0; row < r_; ++row) {
    emd_edges_[row].resize(c_ - 1);
    for (int col = 0; col < c_ - 1; ++col) {
      emd_edges_[row][col].resize(r_);
      for (int dest = 0; dest < r_; ++dest) {
        EdgeIndex next_edge_index = e_.size();
        Edge forward(innode_index(dest, col + 1), 1, 0.0, true,
            next_edge_index + 1);
        Edge backward(outnode_index(row, col), 0, 0.0, false,
            next_edge_index);
        e_.push_back(forward);
        e_.push_back(backward);
        emd_edges_[row][col][dest] = next_edge_index;
        outgoing_edges_[outnode_index(row, col)].push_back(next_edge_index);
        outgoing_edges_[innode_index(dest, col + 1)].push_back(
            next_edge_index + 1);
      }
    }
  }

  set_sparsity(0);

  printf("Node indices:\n");
  printf("  Source: %lu, sink: %lu\n", s_, t_);
  for (int col = 0; col < c_; ++col) {
    for (int row = 0; row < r_; ++row) {
      printf("  Entry %d,%d: innode: %lu, outnode: %lu\n", row, col,
          innode_index(row, col), outnode_index(row, col));
    }
  }

  printf("Edges:\n");
  for (size_t ii = 0; ii < outgoing_edges_.size(); ++ii) {
    for (size_t jj = 0; jj < outgoing_edges_[ii].size(); ++jj) {
      EdgeIndex curi = outgoing_edges_[ii][jj];
      Edge cur = e_[curi];
      printf("  Edge %lu: from: %lu, to: %lu, cap: %d, cost: %f, forward: %d, "
          "opposite: %lu\n", curi, ii, cur.to, cur.capacity, cur.cost,
          cur.is_forward, cur.opposite);
    }
  }
}

void EMDFlowNetworkSAP::set_sparsity(int k) {
  k_ = k;
}

void EMDFlowNetworkSAP::run_flow(double lambda) {
  // TODO
}

int EMDFlowNetworkSAP::get_EMD_used() {
  int emd_cost = 0;
  for (int row = 0; row < r_; ++row) {
    for (int col = 0; col < c_ - 1; ++col) {
      for (int dest = 0; dest < r_; ++dest) {
        if (e_[emd_edges_[row][col][dest]].capacity == 0) {
          emd_cost += abs(row - dest);
        }
      }
    }
  }
  return emd_cost;
}

double EMDFlowNetworkSAP::get_supported_amplitude_sum() {
  double amp_sum = 0;
  for (int row = 0; row < r_; ++row) {
    for (int col = 0; col < c_; ++col) {
      if (e_[node_edges_[row][col]].capacity == 0) {
        amp_sum += abs(a_[row][col]);
      }
    }
  }
  return amp_sum;
}

void EMDFlowNetworkSAP::get_support(std::vector<std::vector<bool> >* support) {
  support->resize(r_);
  for (int row = 0; row < r_; ++row) {
    (*support)[row].resize(c_);
    for (int col = 0; col < c_; ++col) {
      (*support)[row][col] = (e_[node_edges_[row][col]].capacity == 0);
    }
  }
}

int EMDFlowNetworkSAP::get_num_nodes() {
  return potential_.size();
}

int EMDFlowNetworkSAP::get_num_edges() {
  return e_.size();
}

int EMDFlowNetworkSAP::get_num_columns() {
  return c_;
}

int EMDFlowNetworkSAP::get_num_rows() {
  return r_;
}
