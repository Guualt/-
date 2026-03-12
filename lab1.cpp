#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>

using namespace std;

enum class TokenType
{
	Keyword,
	Identifier,
	IntLiteral,
	Operator,
	Delimiter,
	EndOfFile,
	Error
};

struct Token
{
	TokenType type;
	string lexeme;
	int line;
	int col;
};

static const unordered_set<string> kKeywords = 
{
	"int", "if", "else", "while", "return", "void"
};

static bool isIdStart(char c)
{
	return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}
static bool isIdPart(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

class Lexer
{
public:
	explicit Lexer(const string& input)
		: s(input), i(0), line(1), col(1) {}

	vector<Token> tokenize()
	{
		vector<Token> out;
		while (true)
		{
			skipWhitespaceAndComments();

			if (peek() == '\0')
			{
				out.push_back(makeToken(TokenType::EndOfFile, "EOF"));
				break;
			}

			char c = peek();
			if (isIdStart(c))
			{
				out.push_back(readIdentifierOrKeyword());
			}
			else if (isdigit(static_cast<unsigned char>(c)))
			{
				out.push_back(readInt());
			}
			else 
			{
				out.push_back(readOpOrDelimOrError());
			}
		}
		return out;
	}

private:
	const string& s;
	size_t i;
	int line, col;

	char peek(size_t offset = 0) const
	{
		size_t p = i + offset;
		if (p >= s.size()) return '\0';
		return s[p];
	}

	char get()
	{
		char c = peek();
		if (c == '\0') return c;
		i++;
		if (c == '\n') { line++; col = 1; }
		else col++;
		return c;
	}

	Token makeToken(TokenType t, const string& lex) const
	{
		
		return Token{ t, lex, line, col };
	}

	void skipWhitespaceAndComments() 
	{
		while (true) 
		{
			// whitespace
			while (isspace(static_cast<unsigned char>(peek()))) get();

			// // comment
			if (peek() == '/' && peek(1) == '/')
			{
				while (peek() != '\0' && peek() != '\n') get();
				continue;
			}
			break;
		}
	}

	Token readIdentifierOrKeyword()
	{
		int startLine = line, startCol = col;
		string lex;
		lex.push_back(get());
		while (isIdPart(peek())) lex.push_back(get());

		if (kKeywords.count(lex)) return Token{ TokenType::Keyword, lex, startLine, startCol };
		return Token{ TokenType::Identifier, lex, startLine, startCol };
	}

	Token readInt()
	{
		int startLine = line, startCol = col;
		string lex;
		while (isdigit(static_cast<unsigned char>(peek()))) lex.push_back(get());
		return Token{ TokenType::IntLiteral, lex, startLine, startCol };
	}

	Token readOpOrDelimOrError()
	{
		int startLine = line, startCol = col;
		char c = get();

		auto op2 = string() + c + peek(); // potential 2-char operator
		if (op2 == "==" || op2 == "!=" || op2 == "<=" || op2 == ">=")
		{
			get();
			return Token{ TokenType::Operator, op2, startLine, startCol };
		}

		// 1-char operators
		if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>')
		{
			return Token{ TokenType::Operator, string(1,c), startLine, startCol };
		}

		// delimiters
		if (c == ';' || c == ',' || c == '(' || c == ')' || c == '{' || c == '}') 
		{
			return Token{ TokenType::Delimiter, string(1,c), startLine, startCol };
		}

		// error
		return Token{ TokenType::Error, string("Unexpected char: ") + c, startLine, startCol };
	}
};

static string tokenTypeName(TokenType t)
{
	switch (t) {
	case TokenType::Keyword: return "KEYWORD";
	case TokenType::Identifier: return "IDENT";
	case TokenType::IntLiteral: return "INT";
	case TokenType::Operator: return "OP";
	case TokenType::Delimiter: return "DELIM";
	case TokenType::EndOfFile: return "EOF";
	case TokenType::Error: return "ERROR";
	}
	return "UNKNOWN";
}

int main() 
{
	// 1) 从文件读（你也可以改成 cin）
	ifstream fin("input.txt");
	if (!fin)
	{
		cerr << "Cannot open input.txt. Put input.txt near the .exe (or project folder).\n";
		return 1;
	}
	string content, line;
	while (std::getline(fin, line))
	{
		content += line;
		content.push_back('\n');
	}

	Lexer lexer(content);
	auto tokens = lexer.tokenize();

	// 2) 输出到屏幕 + 文件 output.txt
	ofstream fout("output.txt");
	for (const auto& tk : tokens)
	{
		string row = tokenTypeName(tk.type) + "\t" + tk.lexeme +
			"\t(line=" + to_string(tk.line) + ", col=" + to_string(tk.col) + ")";
		cout << row << "\n";
		fout << row << "\n";
	}

	return 0;
}
