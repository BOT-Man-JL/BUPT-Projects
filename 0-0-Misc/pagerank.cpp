
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
// 0.295865 http://localhost/
// 0.288972 http://localhost/page1/
// 0.207581 http://localhost/page2/
// 0.207581 http://localhost/page2/page2-1/

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

// adjacency matrix: { from -> to }
template <typename K>
using Matrix = std::multimap<K, K>;

// vector: { key -> value }
template <typename K, typename V>
using Vector = std::map<K, V>;

constexpr auto kDampingFactor = .85;
constexpr auto kConvergenceEpsilon = 1e-6;

// Matrix -> (Matrix)^T
template <typename K>
Matrix<K> Transpose(const Matrix<K>& matrix) {
  using Edge = typename Matrix<K>::value_type;

  return std::accumulate(
      std::begin(matrix), std::end(matrix), Matrix<K>{},
      [](Matrix<K>& transposed, const Edge& edge) -> Matrix<K>& {
        transposed.emplace(edge.second, edge.first);
        return transposed;
      });
}

// Vector -> Vector / dimension(Vector)
template <typename K, typename V>
Vector<K, V> Uniform(Vector<K, V>&& vector) {
  using VectorKeyValuePair = typename Vector<K, V>::value_type;

  // dimension(ranks)
  auto dimension = [&vector]() -> size_t { return vector.size(); };

  // Vector -> Vector / dimension
  std::for_each(std::begin(vector), std::end(vector),
                [&dimension](VectorKeyValuePair& pair) -> void {
                  pair.second = static_cast<V>(1) / static_cast<V>(dimension());
                });
  return std::move(vector);
}

// Matrix -> Vector
template <typename K, typename V>
Vector<K, V> InitVector(const Matrix<K>& matrix) {
  using Edge = typename Matrix<K>::value_type;
  using IndexSet = std::set<K>;

  // Matrix -> IndexSet
  auto index_set = [](const Matrix<K>& matrix) -> IndexSet {
    return std::accumulate(
        std::begin(matrix), std::end(matrix), IndexSet{},
        [](IndexSet& index_set, const Edge& edge) -> IndexSet& {
          index_set.emplace(edge.first);
          index_set.emplace(edge.second);
          return index_set;
        });
  };

  // IndexSet -> Vector
  auto init_vector = [](const IndexSet& index_set) -> Vector<K, V> {
    return std::accumulate(
        std::begin(index_set), std::end(index_set), Vector<K, V>{},
        [](Vector<K, V>& init_vector, K index) -> Vector<K, V>& {
          init_vector.emplace(index, V{});
          return init_vector;
        });
  };

  return init_vector(index_set(matrix));
}

// Ranks -> next Ranks (override |temp_ranks|)
template <typename K, typename V>
Vector<K, V> StepPageRank(const Matrix<K>& matrix,
                          const Matrix<K>& transposed,
                          const Vector<K, V>& ranks,
                          Vector<K, V>&& temp_ranks) {
  using Edge = typename Matrix<K>::value_type;
  using VectorKeyValuePair = typename Vector<K, V>::value_type;
  assert(ranks.size() == temp_ranks.size());

  // ranks[index]
  auto rank = [&ranks](K index) -> V { return ranks.at(index); };

  // out_degree(index)
  auto out_degree = [&matrix](K index) -> size_t {
    return matrix.count(index);
  };

  // sigma {from \in ((Matrix)^T)[to]} (rank(from) / out_degree(from))
  auto incoming_rank = [&transposed, &rank, &out_degree](K index) -> V {
    return std::accumulate(
        transposed.lower_bound(index), transposed.upper_bound(index), V{},
        [&rank, &out_degree](V rank_sum, const Edge& edge) -> V {
          return rank_sum +
                 rank(edge.second) / static_cast<V>(out_degree(edge.second));
        });
  };

  // dimension(ranks)
  auto dimension = [&ranks]() -> size_t { return ranks.size(); };

  // ((1 - damping) / N + damping * incoming_rank(index))
  auto new_rank = [&incoming_rank, &dimension](K index) -> V {
    return (1 - kDampingFactor) / static_cast<V>(dimension()) +
           kDampingFactor * incoming_rank(index);
  };

  std::for_each(std::begin(temp_ranks), std::end(temp_ranks),
                [&new_rank](VectorKeyValuePair& pair) -> void {
                  pair.second = new_rank(pair.first);
                });
  return std::move(temp_ranks);
}

// || Ranks - next Ranks || == 0
template <typename K, typename V>
bool IsConvergent(const Vector<K, V>& ranks, const Vector<K, V>& next_ranks) {
  using VectorKeyValuePair = typename Vector<K, V>::value_type;
  assert(ranks.size() == next_ranks.size());

  // next_ranks[index]
  auto next_rank = [&next_ranks](K index) -> V { return next_ranks.at(index); };

  // (rank)^2
  auto square = [](V rank) -> V { return rank * rank; };

  // sigma {index \in IndexSet} ((rank(index) - next_rank(index))^2) < e
  return std::accumulate(
             std::begin(ranks), std::end(ranks), V{},
             [&square, &next_rank](V vector_norm,
                                   const VectorKeyValuePair& pair) -> V {
               return vector_norm + square(pair.second - next_rank(pair.first));
             }) < kConvergenceEpsilon;
}

// Ranks -> step(Ranks) (override |temp_ranks|)
//       -> convergent ? Ranks : iterate(Ranks)
template <typename K, typename V>
Vector<K, V> IteratePageRank(const Matrix<K>& matrix,
                             const Matrix<K>& transposed,
                             Vector<K, V>&& ranks,
                             Vector<K, V>&& temp_ranks) {
  // Use double buffer to avoid unnecessary copy for optimization
  return [&matrix, &transposed,
          &ranks](Vector<K, V>&& next_ranks) -> Vector<K, V> {
    return IsConvergent(ranks, next_ranks)
               ? std::move(next_ranks)
               : IteratePageRank(matrix, transposed, std::move(next_ranks),
                                 std::move(ranks));
  }(StepPageRank(matrix, transposed, ranks, std::move(temp_ranks)));
}

using Index = size_t;
using Rank = double;
using Url = std::string;

Vector<Index, Rank> DoPageRank(const Matrix<Index>& matrix) {
  return IteratePageRank(matrix, Transpose(matrix),
                         Uniform(InitVector<Index, Rank>(matrix)),
                         InitVector<Index, Rank>(matrix));
}

struct Line {
  std::string line;

  operator bool() const { return !line.empty(); }
  operator std::string() const { return line; }
};

std::istream& operator>>(std::istream& istr, Line& data) {
  return std::getline(istr, data.line);
}

// (Index -> Url) x ((Index -> Index))
struct InputRet {
  Vector<Index, Url> urls;
  Matrix<Index> matrix;

  bool read_empty_line = false;
};

// istr => (Index -> Url) x ((Index -> Index))
InputRet Input(std::istream& istr) {
  auto read_sec1 = [](InputRet& ret, const std::string& line) -> InputRet& {
    Index index;
    Url url;
    std::istringstream(line) >> index >> url;
    ret.urls.emplace(index, url);
    return ret;
  };

  auto read_sec2 = [](InputRet& ret, const std::string& line) -> InputRet& {
    Index index1;
    Index index2;
    std::istringstream(line) >> index1 >> index2;
    // ignore self-loop
    if (index1 != index2)
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

// ((Rank -> Url)) sorted by greater<Rank>
using OutputMap = std::multimap<Rank, Url, std::greater<Rank>>;

// (Index -> Rank) x (Index -> Url) -> ((Rank -> Url))
OutputMap GetUrlRankMap(const Vector<Index, Url>& urls,
                        const Vector<Index, Rank>& ranks) {
  using IndexUrlPair = Vector<Index, Url>::value_type;
  assert(urls.size() == ranks.size());

  // ranks[index]
  auto rank = [&ranks](Index index) -> Rank { return ranks.at(index); };

  return std::accumulate(
      std::begin(urls), std::end(urls), OutputMap{},
      [&rank](OutputMap& ret, const IndexUrlPair& pair) -> OutputMap& {
        ret.emplace(rank(pair.first), pair.second);
        return ret;
      });
}

namespace std {
std::ostream& operator<<(std::ostream& ostr,
                         const OutputMap::value_type& data) {
  return ostr << data.first << " " << data.second;
}
}  // namespace std

// ((Rank -> Url)) => ostr
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
      Output(ostr, GetUrlRankMap(input.urls, DoPageRank(input.matrix)));
    }(Input(ifs), ofs.is_open() ? ofs : std::cout);
  }(std::ifstream(argv[1]), argc >= 3 ? std::ofstream(argv[2])
                                      : std::ofstream());
  return 0;
}
