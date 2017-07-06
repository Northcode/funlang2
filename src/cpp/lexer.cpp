#include "lexer.hpp"

lexer::lexer(const std::string input, arena* arr)
  : _input(input)
  , begin(_input.begin())
  , end(_input.end())
  , iter(begin)
  , _arena(arr)
  , tokens{}
{
    state = HALT;
}

const char&
lexer::current() const
{
    return *iter;
}

const char&
lexer::peek() const
{
    auto next = iter;
    next++;
    return *next;
}

const char&
lexer::step()
{
    auto cur = *iter;
    if (cur == '\n') {
        curline++;
    }
    return *(++iter);
}

void
lexer::scan_all()
{
    init_scan();
    while (state != HALT) {
        scan_char();
    }
}

void
lexer::init_scan()
{
    state = INIT;
    iter = begin;
    curline = 1;
}

void
lexer::scan_char()
{

    switch (state) {
        case INIT:
            assert(iter != end);

            buffer = _arena->alloc_str(0);
            buf_type = t_none;

            state = READ;
            break;
        case READ:
            assert(iter != end);

            if (isdigit(current())) {
                state = DIGIT;
            } else if (current() == '"') {
                step();
                state = STR;
            } else if (isalpha(current()) ||
                       valid_ident_chars.count(current())) {
                state = IDENT;
            } else if (current() == '?') {
                state = CHAR;
            } else if (current() == ':') {
                step();
                state = KEYWORD;
            } else if (valid_symbol_chars.count(current())) {
                state = SYMBOL;
            } else if (isspace(current()) || current() == ',') {
                step();
                state = READ;
            } else {
                error("unexpected character", distance(begin, iter));
                state = HALT;
            }
            break;
        case SYMBOL:
            assert(iter != end);

            switch (current()) {
                case '(':
                    buf_type = t_par_open;
                    break;
                case ')':
                    buf_type = t_par_close;
                    break;
                case '{':
                    buf_type = t_map_open;
                    break;
                case '}':
                    buf_type = t_map_close;
                    break;
                case '[':
                    buf_type = t_vec_open;
                    break;
                case ']':
                    buf_type = t_vec_close;
                    break;
                case '#':
                    buf_type = t_hash;
                    break;
                default:
                    buf_type = t_none;
                    break;
            }

            _arena->append_char(&buffer, current());
            step();

            state = PUSH;

            break;
        case CHAR:
            assert(iter != end);

            step();
            _arena->append_char(&buffer, current());

            step();

            buf_type = t_chr;
            state = PUSH;

            break;
        case IDENT:
            assert(iter != end);
            {

                _arena->append_char(&buffer, current());
                step();

                if (!(isalnum(current()) ||
                      valid_ident_chars.count(current()))) {
                    std::string tester{ buffer.data };

                    if (valid_literals.count(tester)) {
                        state = LITERAL;
                        break;
                    } else {
                        buf_type = t_ident;
                        state = PUSH;
                    }
                }
            }
            break;
        case LITERAL:
            assert(iter != end);
            {
                std::string test{ buffer.data };

                // @TODO: maybe make this faster:

                if (test == "true") {
                    buf_type = t_true;
                } else if (test == "false") {
                    buf_type = t_false;
                } else if (test == "def") {
                    buf_type = t_def;
                } else if (test == "let") {
                    buf_type = t_let;
                } else if (test == "fn") {
                    buf_type = t_fn;
                } else if (test == "if") {
                    buf_type = t_if;
                } else if (test == "nil") {
                    buf_type = t_nil;
                }

                state = PUSH;
            }

            break;
        case KEYWORD:
            assert(iter != end);

            _arena->append_char(&buffer, current());
            step();

            buf_type = t_keyword;

            if (isspace(current())) {
                state = PUSH;
            }

            break;
        case DIGIT:
            assert(iter != end);

            _arena->append_char(&buffer, current());
            step();

            buf_type = t_integer;

            if (isdigit(current())) {

            } else if (current() == '.') {
                state = DECIMAL;
            } else if (current() == '/') {
                state = RATIO;
            } else {
                state = PUSH;
            }
            break;
        case RATIO:
            assert(iter != end);

            _arena->append_char(&buffer, current());
            step();

            buf_type = t_ratio;

            if (!isdigit(current())) {
                state = PUSH;
            }

            break;
        case DECIMAL:
            assert(iter != end);
            _arena->append_char(&buffer, current());
            step();

            buf_type = t_decimal;

            if (!isdigit(current())) {
                state = PUSH;
            }

            break;
        case STR:
            assert(iter != end);

            _arena->append_char(&buffer, current());
            step();

            buf_type = t_str;

            if (current() == '\\') {
                state = ESCAPED_STR;
            } else if (current() == '"') {
                step();
                state = PUSH;
            }
            break;
        case ESCAPED_STR:
            assert(iter != end);
            step();

            {
                char to_append = '\0';
                switch (current()) {
                    case 'n':
                        to_append = '\n';
                        break;
                    case 't':
                        to_append = '\t';
                        break;
                    case 'r':
                        to_append = '\r';
                        break;
                    case '\\':
                        to_append = '\\';
                        break;
                    case 'b':
                        to_append = '\b';
                        break;
                    default:
                        to_append = current();
                        break;
                }
                _arena->append_char(&buffer, to_append);
                step();
            }

            if (current() == '"') {
                state = PUSH;
            } else {
                state = STR;
            }

            break;
        case PUSH: {
            token cur;

            // std::cout << "read: " << buffer << "\n";

            cur.type = buf_type;
            _arena->append_char(&buffer, '\0');

            _arena->realloc_head(&buffer, buffer.len - 1);

            switch (buf_type) {
                case t_none:
                case t_nil:
                case t_true:
                case t_false:
                case t_def:
                case t_let:
                case t_fn:
                case t_if:
                case t_par_open:
                case t_par_close:
                case t_map_open:
                case t_map_close:
                case t_vec_open:
                case t_vec_close:
                case t_hash:
                    cur.ts = ts_char;
                    cur.data_char = buffer.data[0];
                    break;
                case t_integer:
                    cur.ts = ts_int;
                    _arena->make_null_term(&buffer);
                    cur.data_int = atoi(buffer.data);
                    break;
                case t_decimal:
                    cur.ts = ts_dec;
                    _arena->make_null_term(&buffer);
                    cur.data_decimal = atof(buffer.data);
                    break;
                case t_ratio: {
                    ratio r{ 0, 0 };

                    _arena->append_char(&buffer, '\0');

                    char* div_char = strchr(buffer.data, '/');
                    int div_index = std::distance(buffer.data, div_char);

                    { // head is temp_buf
                        mystr temp_buf = _arena->alloc_str(div_index);

                        strncpy(temp_buf.data, buffer.data, temp_buf.len);
                        _arena->append_char(&temp_buf, '\0');
                        r.counter = atoi(temp_buf.data);
                        _arena->discard_head();
                    } // head is now buffer again

                    { // head is temp_buf
                        mystr temp_buf =
                          _arena->alloc_str(buffer.len - div_index - 1);
                        strncpy(temp_buf.data,
                                buffer.data + div_index + 1,
                                temp_buf.len);
                        _arena->append_char(&temp_buf, '\0');
                        r.divider = atoi(temp_buf.data);
                        _arena->discard_head();
                    } // head is now buffer again

                    _arena->realloc_head(&buffer, buffer.len - 1);

                    cur.ts = ts_rat;
                    cur.data_rat = r;
                } break;
                case t_chr:
                    cur.ts = ts_char;
                    cur.data_char = buffer.data[0];
                    break;
                case t_str:
                case t_ident:
                    cur.ts = ts_str;
                    cur.data_str = buffer;
                    break;
                case t_keyword:
                    // @TODO: atom tableify keywords
                    cur.ts = ts_str;
                    cur.data_str = buffer;
                    break;
            }

            switch (buf_type) {
                case t_str:
                case t_ident:
                case t_keyword:
                    break;
                default:
                    _arena->discard_head();
                    break;
            }

            tokens.push_back(cur);

            if (iter == end) {
                state = HALT;
            } else {
                state = INIT;
            }
        } break;
        case HALT:
            return;
    }
}

void
lexer::error(const std::string message, int chr)
{
    std::cout << "error on line: " << curline << "," << chr << ": " << message
              << std::endl;
}
