# 词法分析器设计与实现

这是我在**编译原理**课程里完成的一个小项目，内容是为一个简化版 C 语言子集实现词法分析器（Lexer）。

项目的目标比较直接：从 `input.txt` 里读取源程序字符流，识别出关键字、标识符、整数字面量、运算符、分隔符这些 Token，同时跳过空白符和单行注释，并把结果输出到屏幕和 `output.txt` 中。

这个项目更偏基础实现，没有用生成器工具，而是自己按状态转移逻辑一步一步写出来的，适合拿来理解词法分析器到底是怎么工作的。

---

## 项目功能

目前支持的内容包括：

- 关键字识别：`int`、`if`、`else`、`while`、`return`、`void`
- 标识符识别：字母 / 下划线开头，后面可以跟字母、数字、下划线
- 整数字面量识别：如 `123`
- 运算符识别：
  - 单字符：`+ - * / = < >`
  - 双字符：`== != <= >=`
- 分隔符识别：`; , ( ) { }`
- 跳过空白符：空格、换行、制表符
- 跳过单行注释：`//`
- 错误处理：遇到非法字符时给出错误类型和位置

---

## 实现思路

这个词法分析器是按“**扫描指针 + 分类处理**”的思路写的。

程序不断读取当前字符，先跳过无关内容（空白和注释），然后根据字符类型决定接下来调用哪个识别函数：

- 如果是字母或下划线，就去识别标识符或关键字
- 如果是数字，就去识别整数
- 否则就尝试识别运算符、分隔符，识别不了就按错误处理

主流程在 `tokenize()` 里面，大致就是这个逻辑：

```cpp
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
```

这种写法的好处是比较直观，读代码的时候基本能直接对应到词法分析的几个核心步骤。

---

## 核心数据结构

项目里定义了 `TokenType` 和 `Token`，用来描述一个单词的类别、内容和位置信息。

```cpp
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
```

这里我把 `line` 和 `col` 也记录下来了，主要是为了后面输出错误信息时能定位到具体位置，这样调试会方便很多。

---

## 标识符 / 关键字识别

标识符和关键字本质上共用一套规则：先按标识符读完整个串，再去关键字表里查一下是不是保留字。

```cpp
Token readIdentifierOrKeyword()
{
    int startLine = line, startCol = col;
    string lex;
    lex.push_back(get());
    while (isIdPart(peek())) lex.push_back(get());

    if (kKeywords.count(lex)) return Token{ TokenType::Keyword, lex, startLine, startCol };
    return Token{ TokenType::Identifier, lex, startLine, startCol };
}
```

这种处理方式比较常见，也比较省事，不需要给每个关键字单独写分支。

---

## 注释和空白符处理

在正式识别 Token 之前，程序会先把空白符和单行注释跳过去，不让这些内容进入真正的词法分析流程。

```cpp
void skipWhitespaceAndComments()
{
    while (true)
    {
        while (isspace(static_cast<unsigned char>(peek()))) get();

        if (peek() == '/' && peek(1) == '/')
        {
            while (peek() != '\0' && peek() != '\n') get();
            continue;
        }
        break;
    }
}
```

这里目前只处理了 `//` 单行注释，没有继续扩展到 `/* ... */` 多行注释。

---

## 运算符、分隔符和错误处理

运算符这块优先判断双字符运算符，避免把 `==` 这种情况拆成两个 `=`。

```cpp
Token readOpOrDelimOrError()
{
    int startLine = line, startCol = col;
    char c = get();

    auto op2 = string() + c + peek();
    if (op2 == "==" || op2 == "!=" || op2 == "<=" || op2 == ">=")
    {
        get();
        return Token{ TokenType::Operator, op2, startLine, startCol };
    }

    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>')
    {
        return Token{ TokenType::Operator, string(1, c), startLine, startCol };
    }

    if (c == ';' || c == ',' || c == '(' || c == ')' || c == '{' || c == '}')
    {
        return Token{ TokenType::Delimiter, string(1, c), startLine, startCol };
    }

    return Token{ TokenType::Error, string("Unexpected char: ") + c, startLine, startCol };
}
```

这部分虽然不长，但其实挺关键，因为很多 bug 都会出在“匹配顺序”上。

---

## 项目结构

```text
.
├── lab1.cpp
├── input.txt
├── output.txt
└── README.md
```

---

## 编译与运行

这个项目是标准 C++ 实现，没有额外依赖，直接用 `g++` 就能编译。

```bash
g++ lab1.cpp -o lexer -std=c++11
./lexer
```

运行后，程序会：

1. 从当前目录读取 `input.txt`
2. 扫描并生成 Token 序列
3. 将结果同时输出到终端和 `output.txt`

如果程序没有找到输入文件，会输出类似的提示：

```cpp
ifstream fin("input.txt");
if (!fin)
{
    cerr << "Cannot open input.txt. Put input.txt near the .exe (or project folder).\n";
    return 1;
}
```

---

## 输入示例

`input.txt`

```c
// test lexer
int main() {
    int a = 10;
    if (a == 10) return 0;
}
```

---

## 输出示例

`output.txt`

```text
KEYWORD	int	(line=2, col=1)
IDENT	main	(line=2, col=5)
DELIM	(	(line=2, col=9)
DELIM	)	(line=2, col=10)
DELIM	{	(line=2, col=12)
KEYWORD	int	(line=3, col=5)
IDENT	a	(line=3, col=9)
OP	=	(line=3, col=11)
INT	10	(line=3, col=13)
DELIM	;	(line=3, col=15)
...
```

---

## 已知限制

这个版本主要是为了完成课程实验，所以功能做得比较基础，目前还有一些限制：

- 只支持单行注释 `//`
- 不支持多行注释 `/* ... */`
- 只识别整数，不支持浮点数和科学计数法
- 不支持 `++`、`--` 这类复合运算符
- 关键字集合是固定的
- 输入默认从 `input.txt` 读取

这些地方后面如果继续扩展，其实就能慢慢往一个更完整的前端词法分析器靠。

---

## 我在这个实验里踩过的坑

这个项目写下来，比较容易出问题的地方主要有三个：

### 1. Token 位置记录不准确
如果直接用扫描结束时的 `line` 和 `col` 生成 Token，位置会偏到末尾。后来改成在每个识别函数开头先记录起始位置，这个问题就解决了。

### 2. 双字符运算符优先级不对
如果先判断单字符运算符，`==` 就会被拆开。后来改成先看双字符，再处理单字符。

### 3. 注释跳过不完整
如果注释刚好出现在文件末尾，处理不仔细的话可能会把后续逻辑带乱，所以这里用了 `peek() != '\0' && peek() != '\n'` 这种写法去保证边界安全。

---

## 这个项目能不能发 GitHub？

我觉得**可以，而且挺适合发**。

原因很简单：

- 这是一个完整的小项目，不是零碎代码片段
- 主题明确，和编译原理课程强相关
- 代码结构清楚，别人一眼能看懂你做了什么
- 既能展示基础的 C++ 实现能力，也能体现你对词法分析流程的理解

不过如果是准备发到 GitHub，我更建议这样放：

- `lab1.cpp`
- `README.md`
- 一个示例 `input.txt`
- 一份对应的 `output.txt`
- 如果想更完整一点，再单独加一个 `docs/` 文件夹放实验报告截图或 PDF

实验报告原文不一定非要直接放仓库首页，首页更适合写清楚“这个项目是干什么的、怎么跑、支持什么、有什么限制”。

---

## 后续可以继续完善的方向

如果后面还想继续改，这几个方向会比较自然：

- 支持多行注释
- 支持浮点数
- 支持字符串常量和字符常量
- 支持更多关键字和运算符
- 把错误信息写得更详细一点
- 把词法规则拆得更模块化，方便后续继续做语法分析

---

## 说明

这是我在**编译原理课程**中的一个实验项目，主要用于练习词法分析器的基本实现过程。  
如果你也在学编译原理，希望这个项目能提供一点参考，不过更重要的还是自己把状态转移逻辑真正写一遍。
