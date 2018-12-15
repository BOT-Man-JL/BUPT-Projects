
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
#include <numeric>
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
  assert(!index_set.empty());

  return std::accumulate(
      std::begin(index_set), std::end(index_set), Ranks{},
      [init = static_cast<Rank>(1) / static_cast<Rank>(index_set.size())](
          Ranks& ranks, Index index) -> Ranks& {
        ranks.emplace(index, init);
        return ranks;
      });
}

// Ranks -> || Ranks ||
Rank GetNorm(const Ranks& ranks) {
  return std::accumulate(
      std::begin(ranks), std::end(ranks), static_cast<Rank>(0),
      [](Rank vector_norm, const Ranks::value_type& pair) -> Rank {
        return vector_norm + pair.second;
      });
}

// Ranks -> Ranks / || Ranks ||
Ranks NormalizeRanks(Ranks&& ranks) {
  std::for_each(
      std::begin(ranks), std::end(ranks),
      [vector_norm = GetNorm(ranks)](Ranks::value_type& pair) -> void {
        pair.second = pair.second / vector_norm;
      });
  return ranks;
}

// Ranks -> next Ranks (override |temp_ranks|)
Ranks StepPageRank(const Matrix& matrix,
                   const Matrix& transposed,
                   const size_t number_of_pages,
                   const Ranks& ranks,
                   Ranks&& temp_ranks) {
  assert(number_of_pages == ranks.size());
  assert(number_of_pages == temp_ranks.size());

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

  std::for_each(std::begin(temp_ranks), std::end(temp_ranks),
                [&new_rank](Ranks::value_type& pair) -> void {
                  pair.second = new_rank(pair.first);
                });
  return NormalizeRanks(std::move(temp_ranks));
}

// || Ranks - next Ranks || == 0
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

// -> next_ranks = step(ranks) (override |temp_ranks|)
// -> convergent(ranks, next_ranks) ? next_ranks : iterate(next_ranks)
Ranks IteratePageRank(const Matrix& matrix,
                      const Matrix& transposed,
                      const size_t N,
                      Ranks&& ranks,
                      Ranks&& temp_ranks) {
  // Use double buffer to avoid unnecessary copy for optimization
  return [&matrix, &transposed, N, &ranks](Ranks&& next_ranks) -> Ranks {

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
               : IteratePageRank(matrix, transposed, N, std::move(next_ranks),
                                 std::move(ranks));
  }(StepPageRank(matrix, transposed, N, ranks, std::move(temp_ranks)));
}

// Matrix -> Ranks
Ranks DoPageRank(const Matrix& matrix) {
  return [&matrix](const IndexSet& index_set) -> Ranks {
    return IteratePageRank(matrix, Transpose(matrix), index_set.size(),
                           InitRanks(index_set), InitRanks(index_set));
  }(GetIndexSet(matrix));
}

struct Line {
  std::string line;

  operator bool() const { return !line.empty(); }
  operator std::string() const { return line; }
};

std::istream& operator>>(std::istream& istr, Line& data) {
  return std::getline(istr, data.line);
}

// (Index x Url) & ((Index x Index))
struct InputRet {
  UrlIndexMap url_index_map;
  Matrix matrix;

  bool read_empty_line = false;
};

// istr => (Index x Url) & ((Index x Index))
InputRet Input(std::istream& istr) {
  auto read_sec1 = [](InputRet& ret, const std::string& line) -> InputRet& {
    Index index;
    Url url;
    std::istringstream(line) >> index >> url;
    ret.url_index_map.emplace(index, url);
    return ret;
  };

  auto read_sec2 = [](InputRet& ret, const std::string& line) -> InputRet& {
    Index index1;
    Index index2;
    std::istringstream(line) >> index1 >> index2;
    ret.matrix.emplace(index1, index2);
    return ret;
  };

  auto set_read_empty_line = [](InputRet& ret) -> InputRet& {
    ret.read_empty_line = true;
    return ret;
  };

  return std::accumulate(std::istream_iterator<Line>(istr),
                         std::istream_iterator<Line>(), InputRet{},
                         [&read_sec1, &read_sec2, &set_read_empty_line](
                             InputRet& ret, const Line& line) -> InputRet& {
                           return line ? (!ret.read_empty_line
                                              ? read_sec1(ret, line)
                                              : read_sec2(ret, line))
                                       : set_read_empty_line(ret);
                         });
}

// ((Rank x Url)) sorted by greater<Rank>
using OutputMap = std::multimap<Rank, Url, std::greater<Rank>>;

// (Index x Rank) x (Index x Url) -> ((Rank x Url))
OutputMap GetUrlRankMap(const UrlIndexMap& url_index_map, const Ranks& ranks) {
  assert(url_index_map.size() == ranks.size());
  assert(fabs(GetNorm(ranks) - static_cast<Rank>(1)) < kDeviationLimit);

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

  [](std::ifstream&& ifs, std::ofstream&& ofs) -> void {
    [](const InputRet& input, std::ostream& ostr) -> void {
      Output(ostr,
             GetUrlRankMap(input.url_index_map, DoPageRank(input.matrix)));
    }(Input(ifs), ofs.is_open() ? ofs : std::cout);
  }(std::ifstream(argv[1]), argc >= 3 ? std::ofstream(argv[2])
                                      : std::ofstream());
  return 0;
}
