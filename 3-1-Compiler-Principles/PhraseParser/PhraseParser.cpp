#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <exception>
#include <iomanip>
#include <cstring>

namespace BOT_PhraseParser
{
	// Helper
	std::string SplitStr (std::string &inputStr,
						  const std::string &delim)
	{
		size_t pos = 0;
		if ((pos = inputStr.find (delim, 0)) != std::string::npos)
		{
			auto ret = inputStr.substr (0, pos);
			inputStr.erase (0, pos + delim.size ());
			return std::move (ret);
		}

		// the Last one
		auto ret = std::move (inputStr);
		inputStr = "";
		return std::move (ret);
	}

	struct Grammar
	{
		// S, N, T
		std::string startSymbol;
		std::unordered_set<std::string> nonTerminalSymbols;
		std::unordered_set<std::string> terminalSymbols;

		// NonTerminalSymbol -> { Production-Rule }
		// Production-Rule = [TerminalSymbol | NonTerminalSymbol]*
		std::unordered_map<
			std::string,
			std::vector<std::vector<std::string>>
		> productionRules;

		Grammar (std::istream &is)
		{
			static const std::string productionSymbol ("->");
			static const std::string whiteChar (" ");
			static const std::string pipeLine ("|");

			char buffer[1024];
			while (is.getline (buffer, 1024))
			{
				std::string productionRule (buffer);
				if (productionRule.empty ())
					continue;

				// Fix for Linux :-(
				if (productionRule.back () == '\r')
					productionRule.pop_back ();
				if (productionRule.empty ()) continue;

				// Split nonTerminal
				std::string nonTerminal = SplitStr (productionRule,
													productionSymbol);

				// Trim nonTerminal
				while (true)
				{
					auto seg = SplitStr (nonTerminal, whiteChar);
					if (!seg.empty ())
					{
						nonTerminal = std::move (seg);
						break;
					}
				}

				// Split productionRule
				while (!productionRule.empty ())
				{
					auto statement = SplitStr (productionRule, pipeLine);
					std::vector<std::string> symbolsOfStatement;
					while (!statement.empty ())
					{
						auto symbol = SplitStr (statement, whiteChar);
						if (!symbol.empty ())
							symbolsOfStatement.emplace_back (std::move (symbol));
					}
					productionRules[nonTerminal]
						.emplace_back (std::move (symbolsOfStatement));
				}

				// Set Start Symbol
				if (startSymbol.empty ())
					startSymbol = std::move (nonTerminal);
			}

			GetSymbols ();
		}

		void FixLeftRecursion ()
		{
			static const std::string epsilon ("epsilon");

			for (auto &rule : productionRules)
			{
				const auto &key = rule.first;
				auto hasLeftRecursion = false;

				for (const auto &statement : rule.second)
					if (statement.front () == key)
					{
						hasLeftRecursion = true;
						break;
					}

				if (!hasLeftRecursion)
					continue;

				auto fixedKey = key + "'";

				// A -> A alpha => A' -> alpha A'
				while (true)
				{
					auto isChanged = false;
					for (auto p = rule.second.begin ();
						 p != rule.second.end (); ++p)
					{
						if (p->front () == key)
						{
							auto &statement = *p;
							statement.erase (statement.begin ());
							statement.push_back (fixedKey);

							productionRules[fixedKey]
								.emplace_back (std::move (statement));
							rule.second.erase (p);

							isChanged = true;
							break;
						}
					}
					if (!isChanged) break;
				}

				// A -> beta => A -> beta A'
				for (auto &statement : rule.second)
					statement.emplace_back (fixedKey);

				// A' -> epsilon
				productionRules[fixedKey]
					.emplace_back (std::vector<std::string> { epsilon });
			}

			GetSymbols ();
		}

	private:
		void GetSymbols ()
		{
			nonTerminalSymbols.clear ();
			terminalSymbols.clear ();

			auto isNT = [this] (const std::string &symbol)
			{
				return nonTerminalSymbols.find (symbol)
					!= nonTerminalSymbols.end ();
			};

			// NonTerminalSymbols
			for (const auto &rule : productionRules)
				nonTerminalSymbols.emplace (rule.first);

			// TerminalSymbols
			for (const auto &rule : productionRules)
				for (const auto &statement : rule.second)
					for (const auto &symbol : statement)
						if (!isNT (symbol))
							terminalSymbols.emplace (symbol);
		}
	};

	class LL1Grammar : public Grammar
	{
		// Helper
		struct StringPairHasher
		{
			size_t operator()(
				const std::pair<std::string, std::string> &obj) const
			{
				static const std::hash<std::string> hasher;
				return (hasher (obj.first) << 16) + hasher (obj.second);
			}
		};

		using FSet = std::unordered_map<
			std::string,
			std::unordered_set<std::string>
		>;

	public:
		// For LL1 Grammar Only
		// <NonTerminalSymbol, TerminalSymbol> -> Production Rule
		std::unordered_map <
			std::pair<std::string, std::string>,
			std::vector<std::string>,
			StringPairHasher
		> table;

		LL1Grammar (std::istream &is)
			: Grammar (is)
		{}

		void GetLL1Table (bool hasSync = true)
		{
			static const std::string sync ("sync");
			static const std::string epsilon ("epsilon");
			static const std::string dollar ("$");
			FSet firstSet, followSet;

			auto placeToSet = [] (FSet &set,
								  const std::string &key,
								  const std::string &value)
			{
				if (set[key].find (value) != set[key].end ())
					return false;

				set[key].emplace (value);
				return true;
			};

			auto allToSet = [&placeToSet] (FSet &set,
										   const std::string &src,
										   const std::string &dst)
			{
				bool isChanged = false;
				for (const auto &symbol : set[src])
					// always skip epsilon
					if (symbol != epsilon)
						isChanged = isChanged || placeToSet (
							set, dst, symbol);
				return isChanged;
			};

			auto isT = [this] (const std::string &symbol)
			{
				return terminalSymbols.find (symbol)
					!= terminalSymbols.end ();
			};

			auto hasEpsilon = [] (
				const std::unordered_set<std::string> &set)
			{
				return set.find (epsilon) != set.end ();
			};

			// Get First Set for Terminals
			for (const auto &symbol : terminalSymbols)
				placeToSet (firstSet, symbol, symbol);

			// Get First Set for NonTerminals
			while (true)
			{
				auto isChanged = false;
				for (const auto &rule : productionRules)
				{
					const auto &key = rule.first;
					for (const auto &statement : rule.second)
					{
						size_t i = 0;
						for (; i < statement.size (); i++)
						{
							isChanged = isChanged || allToSet (
								firstSet, statement[i], key);

							// break if hasn't epsilon
							if (!hasEpsilon (firstSet[statement[i]]))
								break;
						}

						// if all has epsilon
						if (i == statement.size ())
							isChanged = isChanged || placeToSet (
								firstSet, key, epsilon);
					}
				}
				if (!isChanged) break;
			}

			// Get Follow Set for NonTerminals
			followSet[startSymbol].emplace (dollar);
			while (true)
			{
				auto isChanged = false;
				for (const auto &rule : productionRules)
				{
					const auto &key = rule.first;
					for (const auto &statement : rule.second)
						for (size_t i = 0; i < statement.size (); i++)
						{
							const auto &curSymbol = statement[i];
							if (isT (curSymbol))
								continue;

							if (i != statement.size () - 1)
							{
								const auto &nxtSymbol = statement[i + 1];
								for (const auto &symbol : firstSet[nxtSymbol])
								{
									if (symbol == epsilon)
										continue;
									isChanged = isChanged || placeToSet (
										followSet, curSymbol, symbol);
								}

								if (hasEpsilon (firstSet[statement[i + 1]]))
									isChanged = isChanged || allToSet (
										followSet, key, curSymbol);
							}
							else
								isChanged = isChanged || allToSet (
									followSet, key, curSymbol);
						}
				}
				if (!isChanged) break;
			}

			auto addToTable = [this] (std::string nonTerminal,
									  std::string terminal,
									  std::vector<std::string> value)
			{
				auto entryPair = std::make_pair (
					std::move (nonTerminal),
					std::move (terminal)
				);
				auto &entry = table[entryPair];

				if (!entry.empty ())
					throw std::runtime_error ("Not LL1 Grammar");
				entry = std::move (value);
			};

			// Generate Table
			for (const auto &rule : productionRules)
			{
				const auto &key = rule.first;
				for (const auto &statement : rule.second)
					for (auto &symbol : firstSet[statement.front ()])
					{
						if (symbol != epsilon)
							addToTable (key, symbol, statement);
						else
							for (auto &symbol2 : followSet[key])
								addToTable (key, symbol2, statement);
					}
			}

			if (!hasSync)
				return;

			// Generate Sync
			for (const auto &nonTerminal : nonTerminalSymbols)
				for (const auto &symbol : followSet[nonTerminal])
				{
					try { addToTable (nonTerminal, symbol, { sync }); }
					catch (const std::exception&) {}
				}
		}
	};

	struct LexParser
	{
		std::vector<std::string> tokens;

		LexParser (std::istream &is,
				   const std::unordered_set<std::string> &puncSet)
		{
			while (true)
			{
				std::string seg;
				char ch;

				if (!is.get (ch))
					// Eof
					break;
				seg += ch;
				int nch = is.peek ();

				// Blank
				// Fix for Linux :-(
				if (ch == ' ' || ch == '\r')
					continue;

				// Number
				if (ch == '.' || isdigit (ch))
				{
					auto state = 0;
					if (ch == '.') state = 1;

					while (true)
					{
						auto isEnd = false;
						char nch = is.peek ();
						switch (state)
						{
							// (digit)+
						case 0:
							if (isdigit (nch))
								;
							else if (nch == '.')
								state = 2;
							else
								isEnd = true;
							break;
							// .
						case 1:
							if (isdigit (nch))
								state = 2;
							else
								isEnd = true;
							break;
							// (digit)* . (digit)+ | (digit)+ . (digit)*
						case 2:
							if (isdigit (nch))
								;
							else
								isEnd = true;
							break;
						}
						if (isEnd) break;

						// Get Char
						seg += (char) is.get ();
					}

					if (state == 0 || state == 2)
						tokens.emplace_back ("num");
					continue;
				}

				// Punc
				if (puncSet.find (seg) != puncSet.end ())
					tokens.emplace_back (seg);
			}
		}
	};

	class PhraseParser
	{
	public:
		PhraseParser (std::istream &is)
			: _grammar (is)
		{
			_grammar.FixLeftRecursion ();
			_grammar.GetLL1Table ();
		}

		void PrintGrammar (std::ostream &os)
		{
			os << "\nGrammar:\n";

			os << "Starting Symbol:\n\t" << _grammar.startSymbol;

			os << "\nNon Terminal Symbols:\n\t";
			for (const auto &symbol : _grammar.nonTerminalSymbols)
				os << symbol << " ";

			os << "\nTerminal Symbols:\n\t";
			for (const auto &symbol : _grammar.terminalSymbols)
				os << symbol << " ";

			os << "\nProduction Rules:\n";
			for (const auto &rule : _grammar.productionRules)
			{
				os << "\t" << rule.first << " -> ";
				for (size_t i = 0; i < rule.second.size (); i++)
				{
					const auto &statement = rule.second[i];
					for (size_t j = 0; j < statement.size (); j++)
					{
						os << statement[j];
						if (j != statement.size () - 1)
							os << " ";
					}
					if (i != rule.second.size () - 1)
						os << " | ";
					else
						os << std::endl;
				}
			}
		}

		void PrintLL1Table (std::ostream &os)
		{
			os << "\nLL1 Table:\n";
			for (const auto &entry : _grammar.table)
			{
				os << std::setw (15) << std::left
					<< "<" + entry.first.first + ", "
					+ entry.first.second + ">"
					<< entry.first.first << " -> ";

				const auto &statement = entry.second;
				for (size_t j = 0; j < statement.size (); j++)
				{
					os << statement[j];
					if (j != statement.size () - 1)
						os << " ";
					else
						os << std::endl;
				}
			}
		}

		void Parse (std::istream &is,
					std::ostream &os)
		{

			char buffer[1024];
			while (is.getline (buffer, 1024))
			{
				if (strlen (buffer) == 0 || buffer[0] == '\r')
					continue;

				os << "\nParse $" << buffer << "$ :\n";
				std::stringstream iStream (buffer);
				_Parse (iStream, os);
			}
		}

	private:
		LL1Grammar _grammar;

		void _Parse (std::istream &is,
					 std::ostream &os)
		{
			static const std::string sync ("sync");
			static const std::string epsilon ("epsilon");
			static const std::string dollar ("$");

			const auto &table = _grammar.table;
			std::vector<std::string> tokenFlow
				= LexParser (is, _grammar.terminalSymbols).tokens;
			std::stack<std::string> stack;

			// Init input & stack
			tokenFlow.emplace_back (dollar);
			stack.push (dollar);
			stack.push (_grammar.startSymbol);

			auto isNT = [this] (const std::string &symbol)
			{
				return _grammar.nonTerminalSymbols.find (symbol)
					!= _grammar.nonTerminalSymbols.end ();
			};

			auto isT = [this] (const std::string &symbol)
			{
				return _grammar.terminalSymbols.find (symbol)
					!= _grammar.terminalSymbols.end ();
			};

			// Run
			for (auto pToken = tokenFlow.begin ();
				 !stack.empty () && pToken != tokenFlow.end ();)
			{
				auto top = stack.top ();
				const auto &token = *pToken;

				if (isNT (top))
				{
					try
					{
						auto pair = std::make_pair (top, token);
						const auto &rule = table.at (pair);

						stack.pop ();
						if (rule.front () != sync)
						{
							// Reversal Push
							for (int i = rule.size () - 1; i >= 0; i--)
								if (rule[i] != epsilon)
									stack.push (rule[i]);

							// Output
							os << top << " -> ";
							for (size_t i = 0; i < rule.size (); i++)
							{
								os << rule[i];
								if (i != rule.size () - 1)
									os << " ";
							}
						}
						else
							os << "Err: " << top + " | " + token
							<< std::setw (20) << "Pop " + top;
					}
					catch (...)
					{
						++pToken;
						os << "Err: " << top + " | " + token
							<< std::setw (20) << "Skip " + token;
					}
				}
				else if (isT (top) || top == dollar)
				{
					++pToken;
					if (top == token)
					{
						stack.pop ();
						if (token == dollar)
							break;

						// Skip os << \n
						continue;
					}
					else
						os << "Err: " << top + " | " + token
						<< std::setw (20) << "Skip " + token;
				}
				else
					// Fatal Error
					throw std::runtime_error (
						"Invalid Stack State: " + top);

				os << std::endl;
			}
		}
	};
}

int main (int argc, char *argv[])
{
	try
	{
		using namespace BOT_PhraseParser;

		if (argc < 3)
			throw std::runtime_error ("Too few Arguments");

		std::ifstream ifsGrammar (argv[1]);
		std::ofstream ofsGrammar (argv[1] + std::string (".output.txt"));
		std::ifstream ifsInput (argv[2]);
		std::ofstream ofsOutput (argv[2] + std::string (".output.txt"));

		if (!ifsGrammar.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[1]);
		if (!ofsGrammar.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[1] + ".output.txt");
		if (!ifsInput.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[2]);
		if (!ofsOutput.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[2] + ".output.txt");

		// Generate parser from ifsGrammar
		PhraseParser parser (ifsGrammar);

		// Print Parser Rules into ofsGrammar
		parser.PrintGrammar (ofsGrammar);
		parser.PrintLL1Table (ofsGrammar);

		// Parse ifs into ofs :-)
		parser.Parse (ifsInput, ofsOutput);
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what () << std::endl;
		return 1;
	}
	return 0;
}
