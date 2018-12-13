
// Functional PageRank implementation
//   by BOT Man & ZhangHan, 2018
//
// References:
// - http://ilpubs.stanford.edu:8090/422/1/1999-66.pdf
// - https://en.wikipedia.org/wiki/PageRank#Iterative
//
// Input:
//
// 1 http://localhost/
// 2 http://localhost/page1/
// 3 http://localhost/page2/
// 4 http://localhost/page2/page2-1/
//
// 1 1
// 1 2
// 2 1
// 2 3
// 2 4
// 3 1
// 3 3
// 3 4
// 4 1
// 4 3
//
// Output:
//
// 0.400453 http://localhost/
// 0.230248 http://localhost/page2/
// 0.207705 http://localhost/page1/
// 0.161595 http://localhost/page2/page2-1/

#include <assert.h>
#include <math.h>
#include <stddef.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

using Index = size_t;
using Matrix = std::multimap<Index, Index>;
using IndexSet = std::set<Index>;

using Rank = double;
using Ranks = std::map<Index, Rank>;

using Url = std::string;
using UrlIndexMap = std::map<Index, Url>;

constexpr auto kDampingFactor = .85;
constexpr auto kConvergenceEpsilon = 1e-6;
constexpr auto kDeviationLimit = 1e-6;

// Matrix -> (Matrix)^T
Matrix Transpose(const Matrix& matrix) {
  return std::accumulate(
      std::begin(matrix), std::end(matrix), Matrix{},
      [](Matrix& transposed, const Matrix::value_type& pair) -> Matrix& {
        transposed.emplace(pair.second, pair.first);
        return transposed;
      });
}

// Matrix -> IndexSet
IndexSet GetIndexSet(const Matrix& matrix) {
  return std::accumulate(
      std::begin(matrix), std::end(matrix), IndexSet{},
      [](IndexSet& index_set, const Matrix::value_type& pair) -> IndexSet& {
        index_set.emplace(pair.first);
        index_set.emplace(pair.second);
        return index_set;
      });
}

// IndexSet -> Ranks { 1/N }
Ranks InitRanks(const IndexSet& index_set) {
  return std::accumulate(
      std::begin(index_set), std::end(index_set), Ranks{},
      [init = static_cast<Rank>(1) / static_cast<Rank>(index_set.size())](
          Ranks& ranks, Index index) -> Ranks& {
        ranks.emplace(index, init);
        return ranks;
      });
}

// inplace update |next_ranks|
void StepPageRank(const Matrix& matrix,
                  const Matrix& transposed,
                  const size_t number_of_pages,
                  const Ranks& ranks,
                  Ranks& next_ranks) {
  // validate: size
  assert(number_of_pages == ranks.size());
  assert(number_of_pages == next_ranks.size());

  // validate: || ranks || == 1
  // sigma {rank \in ranks} (rank) < e
  assert(fabs(static_cast<Rank>(1) -
              std::accumulate(
                  std::begin(ranks), std::end(ranks), static_cast<Rank>(0),
                  [](Rank rank_sum, const Ranks::value_type& pair) -> Rank {
                    return rank_sum + pair.second;
                  })) < kDeviationLimit);

  // ranks[index]
  auto rank = [&ranks](Index index) -> Rank { return ranks.at(index); };

  // out_degree(index)
  auto out_degree = [&matrix](Index index) -> size_t {
    return matrix.count(index);
  };

  // sigma {from \in ((Matrix)^T)[to]} (rank(from) / out_degree(from))
  auto incoming_rank = [&transposed, &rank, &out_degree](Index index) -> Rank {
    return std::accumulate(
        transposed.lower_bound(index), transposed.upper_bound(index),
        static_cast<Rank>(0),
        [&rank, &out_degree](Rank rank_sum,
                             const Matrix::value_type& pair) -> Rank {
          return rank_sum +
                 rank(pair.second) / static_cast<Rank>(out_degree(pair.second));
        });
  };

  // ((1 - damping) / number_of_pages + damping * incoming_rank(index))
  auto new_rank = [&incoming_rank, number_of_pages](Index index) -> Rank {
    return (1 - kDampingFactor) / static_cast<Rank>(number_of_pages) +
           kDampingFactor * incoming_rank(index);
  };

  // output to each in |next_ranks| by |new_rank|
  std::for_each(std::begin(next_ranks), std::end(next_ranks),
                [&new_rank](Ranks::value_type& pair) -> void {
                  pair.second = new_rank(pair.first);
                });
}

// || ranks - next_ranks || == 0
bool IsConvergent(const Ranks& ranks, const Ranks& next_ranks) {
  assert(ranks.size() == next_ranks.size());

  // next_ranks[index]
  auto next_rank = [&next_ranks](Index index) -> Rank {
    return next_ranks.at(index);
  };

  // (rank)^2
  auto square = [](Rank rank) -> Rank { return rank * rank; };

  // sigma {index \in IndexSet} ((rank(index) - next_rank(index))^2) < e
  return std::accumulate(
             std::begin(ranks), std::end(ranks), static_cast<Rank>(0),
             [&square, &next_rank](Rank norm,
                                   const Ranks::value_type& pair) -> Rank {
               return norm + square(pair.second - next_rank(pair.first));
             }) < kConvergenceEpsilon;
}

// convergent ? ranks : iterate(ranks)
Ranks& IteratePageRank(const Matrix& matrix,
                       const Matrix& transposed,
                       const size_t number_of_pages,
                       Ranks& ranks,
                       Ranks& next_ranks) {
  StepPageRank(matrix, transposed, number_of_pages, ranks, next_ranks);

#ifdef DEBUG
  std::cout << "pre: ";
  for (const auto& pair : ranks) {
    std::cout << pair.second << " ";
  }
  std::cout << "\nnxt: ";
  for (const auto& pair : next_ranks) {
    std::cout << pair.second << " ";
  }
  std::cout << "\n\n";
#endif  // DEBUG

  return IsConvergent(ranks, next_ranks)
             ? next_ranks
             : IteratePageRank(matrix, transposed, number_of_pages, next_ranks,
                               ranks);
}

// Matrix -> Ranks
Ranks DoPageRank(const Matrix& matrix) {
  return [&matrix](const IndexSet& index_set) -> Ranks {
    // Non-functional:
    // Use double buffer to avoid unnecessary copy for optimization
    Ranks ranks1 = InitRanks(index_set);
    Ranks ranks2 = InitRanks(index_set);
    return IteratePageRank(matrix, Transpose(matrix), index_set.size(), ranks1,
                           ranks2);
  }(GetIndexSet(matrix));
}

struct Line {
  std::string line;
  operator std::string() const { return line; }
};

std::istream& operator>>(std::istream& istr, Line& data) {
  return std::getline(istr, data.line);
}

// (Index x Url) & ((Index x Index))
using InputRet = std::pair<UrlIndexMap, Matrix>;

// istr => (Index x Url) & ((Index x Index))
InputRet Input(std::istream& istr) {
  return std::accumulate(
      std::istream_iterator<Line>(istr), std::istream_iterator<Line>(),
      InputRet{},
      [meet_empty_line = false](InputRet& ret,
                                const Line& line) mutable -> InputRet& {
        if (line.line.empty()) {
          meet_empty_line = true;
          return ret;
        }
        if (!meet_empty_line) {
          Index index;
          Url url;
          std::istringstream(line) >> index >> url;
          ret.first.emplace(index, url);
        } else {
          Index index1;
          Index index2;
          std::istringstream(line) >> index1 >> index2;
          ret.second.emplace(index1, index2);
        }
        return ret;
      });
}

// (Rank x Url) sorted by greater<Rank>
using OutputMap = std::map<Rank, Url, std::greater<Rank>>;

// (Index x Rank) x (Index x Url) -> (Rank x Url)
OutputMap NormalizeOutput(const UrlIndexMap& url_index_map,
                          const Ranks& ranks) {
  // ranks[index]
  auto rank = [&ranks](Index index) -> Rank { return ranks.at(index); };

  return std::accumulate(
      std::begin(url_index_map), std::end(url_index_map), OutputMap{},
      [&rank](OutputMap& ret,
              const UrlIndexMap::value_type& url_index) -> OutputMap& {
        ret.emplace(rank(url_index.first), url_index.second);
        return ret;
      });
}

namespace std {
std::ostream& operator<<(std::ostream& ostr,
                         const OutputMap::value_type& data) {
  return ostr << data.first << " " << data.second;
}
}  // namespace std

// (Rank x Url) => ostr
void Output(std::ostream& ostr, const OutputMap& url_rank_map) {
  std::copy(std::begin(url_rank_map), std::end(url_rank_map),
            std::ostream_iterator<OutputMap::value_type>(ostr, "\n"));
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "usage: ./pagerank CRAWLER_OUTPUT [PAGERANK_OUTPUT]\n";
    return 1;
  }

  auto ifs = std::ifstream(argv[1]);
  auto ofs = argc >= 3 ? std::ofstream(argv[2]) : std::ofstream();
  if (!ifs.is_open()) {
    std::cerr << "failed to open " << argv[1] << "\n";
    return 1;
  }

  [&ofs](const InputRet& input) -> void {
    Output(ofs.is_open() ? ofs : std::cout,
           NormalizeOutput(input.first, DoPageRank(input.second)));
  }(Input(ifs));
  return 0;
}
