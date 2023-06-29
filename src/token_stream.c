#define _POSIX_C_SOURCE 200809L

#include "token_stream.h"
#include "token.h"
#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

char *current_input;
char *current_file;

static bool start_with(const char *p, const char *str)
{
	return strncmp(p, str, strlen(str)) == 0;
}

static bool is_ident1(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_ident2(char c)
{
	return is_ident1(c) || ('0' <= c && c <= '9');
}

static int read_punct(char *str)
{
	static char *key_char[] = {"==", "!=", "<=", ">=", "->"};
	for (int i = 0; i < sizeof(key_char) / sizeof(*key_char); ++i)
	{
		if (start_with(str, key_char[i]))
		{
			return strlen(key_char[i]);
		}
	}
	return ispunct(*str) ? 1 : 0;
}

static int read_escaped_char(char *p)
{
	switch (*p)
	{
	case 'a':	// 响铃（警报）
		return '\a'; 
	case 'b': // 退格
    return '\b';
  case 't': // 水平制表符，tab
    return '\t';
  case 'n': // 换行
    return '\n';
  case 'v': // 垂直制表符
    return '\v';
  case 'f': // 换页
    return '\f';
  case 'r': // 回车
    return '\r';
  // 属于GNU C拓展
  case 'e': // 转义符
    return 27;
	case '0':
		return 0;
  default: // 默认将原字符返回
    return *p;
	}
}

static char *str_literal_end(char *p)
{
	char *start = p;
	while (*p != '\"')
	{
		if (*p == '\n' || *p == '\0')
		{
			error_at(start, "unclose string literal");
		}
		if (*p == '\\')
		{
			p++;
		}
		p++;
	}
	return p;
}

static char *read_file(char *path)
{
	FILE *fp;
	if (strncmp(path, "-", 1) == 0)
	{
		fp = stdin;
	}
	else
	{
		fp = fopen(path, "r");
		if (!fp)
		{
			error_log("cannot open %s: %s", path, strerror(errno));
		}
	}
	char *buf;
	size_t buf_len;
	FILE *out = open_memstream(&buf, &buf_len);

	while (true)
	{
		char buf2[4096];
		int n = fread(buf2, 1, sizeof(buf2), fp);
		if (n == 0)
		{
			break;
		}
		fwrite(buf2, 1, n, out);
	}
	if (fp != stdin)
	{
		fclose(fp);
	}
	fflush(out);
	if (buf_len == 0 || buf[buf_len - 1] != '\n')
	{
		fputc('\n', out);
	}
	fputc('\0', out);
	fclose(out);
	return buf;
}

static token_t *read_str_literal(char *start)
{
	// 读取到字符串右边的 "
	char *end = str_literal_end(start + 1);
	// 定义一个与字符串字面量内字符数+1的Buf
  // 用来存储最大位数的字符串字面量
	char *buf = calloc(1, end - start);
  // 实际的字符位数，一个转义字符为1位
	int len = 0;
	char *p = start + 1;
	while (p < end)
	{
		if (*p == '\\')
		{
			buf[len++] = read_escaped_char(p+1);
			p = p + 2;
		}
		else
		{
			buf[len++] = *p++;
		}
	}
	// Token这里需要包含带双引号的字符串字面量
	token_t *tok = token_create(TK_STR, start, end + 1);
	tok->str = buf;
	return tok;
}

token_stream_t *token_stream_create()
{
    token_stream_t *ts = calloc(1, sizeof(token_stream_t));
    return ts;
}

void token_stream_add(token_stream_t *ts, token_t *tok)
{
    if (ts->token_count == 0)
    {
        ts->tokens = tok;
        ts->cur_token_pos = tok;
    }
    else
    {
        ts->cur_token_pos->next = tok;
        ts->cur_token_pos = ts->cur_token_pos->next;
    }
    ts->token_count++;
}

token_t *token_stream_get(token_stream_t *ts)
{
    if (ts->token_count == 0)
    {
        return NULL;
    }
    token_t *tok = ts->tokens;
    return tok;
}

void token_stream_advance(token_stream_t *ts)
{
    if (ts->token_count != 0)
    {
        ts->tokens = ts->tokens->next;
        ts->token_count--;
    }
    else
    {
        error_log("token已经处理完了, 无法获取");
    }
}

void token_stream_dump(token_stream_t *ts)
{
    token_t *head = ts->tokens;
    while (head != NULL) {
        switch (head->kind) {
            case TK_IDENT:
                printf("IDENT: \t%.*s\n", head->len, head->loc);
                break;
            case TK_PUNCT:
                printf("PUNCT: \t%.*s\n", head->len, head->loc);
                break;
            case TK_NUM:
                printf("NUM: \t%d\n", head->val);
                break;
            case TK_EOF:
                printf("EOF\t\n");
                break;
						case TK_STR:
								printf("STR: \t%s\n", head->str);
								break;
            default:
                printf("Unknown token kind.\n");
                break;
        }
        head = head->next;
    }
}

bool token_stream_consume(token_stream_t *ts, char *str)
{
	if (token_equal_str(token_stream_get(ts), str))
	{
		token_stream_advance(ts);
		return true;
	}
	return false;
}

token_t* token_stream_peek_next(token_stream_t *ts)
{
	if (ts->tokens && ts->tokens->next)
	{
		return ts->tokens->next;
	}
	return NULL;
}

token_stream_t *tokenize(char *p)
{
	current_input = p;
	token_stream_t *ts = token_stream_create();

	token_t *cur;
	long num;

	while (*p)
	{
		    // 跳过行注释
    if (start_with(p, "//")) 
		{
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    // 跳过块注释
    if (start_with(p, "/*")) 
		{
      // 查找第一个"*/"的位置
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "unclosed block comment");
      p = q + 2;
      continue;
    }
		if (isspace(*p))
		{
			p++;
			continue;
		}
		
		if (isdigit(*p))
		{
			const char *start = p;
			num = strtol(p, &p, 10);
			cur = token_create(TK_NUM, start, p);
			cur->val = num;
			token_stream_add(ts, cur);
			continue;
		}

		if (*p == '"')
		{
			cur = read_str_literal(p);
			token_stream_add(ts, cur);
			p += cur->len;
			continue;
		}

		if (is_ident1(*p))
		{
			char *start = p;
			do
			{
				++p;
			}while(is_ident2(*p));
			cur = token_create(TK_IDENT, start, p);
			token_stream_add(ts, cur);
			continue;
		}
		int punct_len = read_punct(p);
		if (punct_len)
		{
			cur = token_create(TK_PUNCT, p, p + punct_len);
			token_stream_add(ts, cur);
			p += punct_len;
			continue;
		}
		error_at(p, "invalid token: %c", *p);
	}
	cur = token_create(TK_EOF, p, p);
	token_stream_add(ts, cur);
	token_add_line_no(ts->tokens);
	return ts;
}

token_stream_t *tokenize_file(char *file_path)
{
	current_file = file_path;
	return tokenize(read_file(file_path));
}