#include "httprequest.h"
using namespace std;

const unordered_map<string, string> HttpRequest::DEFAULT_POST_TAG{
    {"/api/register", "1"},
    {"/api/login", "2"},
};

const unordered_set<string> HttpRequest::DEFAULT_HTML{
    "/index",
    "/register",
    "/login",
    "/welcome",
    "/video",
    "/picture",
};

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/login.html", 1},
};

void HttpRequest::Init()
{
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const
{
    if (header_.count("Connection") == 1)
    {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

// 解析处理
bool HttpRequest::parse(Buffer &buff)
{
    const char CRLF[] = "\r\n"; // 行结束符标志
    if (buff.ReadableBytes() <= 0)
    { // 没有可读的字节
        return false;
    }
    // 复用连接要清除上次的json数据
    retjson_ = "";
    // 读取数据
    while (buff.ReadableBytes() && state_ != FINISH)
    {
        const char *lineEnd = search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.Peek(), lineEnd);

        // 有限状态机
        switch (state_)
        {
        case REQUEST_LINE:
            if (!ParseRequestLine_(line))
            {
                return false;
            }
            ParsePath_();
            break;
        case HEADERS:
            ParseHeader_(line);
            if (buff.ReadableBytes() <= 2)
            {
                state_ = FINISH;
            }
            break;
        case BODY:
            ParseBody_(line);
            break;
        default:
            break;
        }
        if (lineEnd == buff.BeginWrite())
        {
            break;
        }
        buff.RetrieveUntil(lineEnd + 2); // 跳过回车换行
    }
    /**
     * 因为HttpBody没有回车换行符，所以上述方法无法正确移动读指针。
     * 正确的操作应该是读取Content-Length数据来判断HttpBody的数据长度。
     * 这里直接解析Http完成之后清空缓冲区，但是可能有粘包的问题。
     * 但是有时候能正常，可能是因为作为新的连接新建了缓冲区。
     */
    buff.RetrieveAll();
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

// 解析路径
void HttpRequest::ParsePath_()
{
    if (path_ == "/")
    {
        path_ = "/index.html";
    }
    else
    {
        for (auto &item : DEFAULT_HTML)
        {
            if (item == path_)
            {
                path_ += ".html";
                break;
            }
        }
    }
}

// 解析请求行
bool HttpRequest::ParseRequestLine_(const string &line)
{
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten))
    {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS; // 状态转换为下一个状态

        // size_t pos = path_.find('?');
        // if (pos != std::string::npos)
        // {
        //     path_ = path_.substr(0, pos);   // 路径部分
        //     query_ = path_.substr(pos + 1); // 查询字符串（去掉 ?）
        // }
        // else
        // {
        //     path_ = path_;
        //     query_ = ""; // 没有查询字符串
        // }

        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

// 解析请求头
void HttpRequest::ParseHeader_(const string &line)
{
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten))
    {
        header_[subMatch[1]] = subMatch[2];
    }
    else
    {
        state_ = BODY; // 状态转换为下一个状态
    }
}

// 解析请求体
void HttpRequest::ParseBody_(const string &line)
{
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

// 16进制转换为10进制
int HttpRequest::ConverHex(char ch)
{
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}

// 处理Post请求
void HttpRequest::ParsePost_()
{
    if (method_ == "POST" &&
        (header_["Content-Type"] == "application/x-www-form-urlencoded" || header_["Content-Type"] == "application/x-www-form-urlencoded; charset=UTF-8"))
    {
        ParseFromUrlencoded_();

        // if (DEFAULT_HTML_TAG.count(path_))
        // {
        //     int tag = DEFAULT_HTML_TAG.find(path_)->second;
        //     LOG_DEBUG("Tag:%d", tag);
        //     if (tag == 0 || tag == 1)
        //     {
        //         path_ = "/error.html";

        //         bool isLogin = (tag == 1);
        //         if (UserVerify(post_["username"], post_["password"], isLogin))
        //         {
        //             path_ = "/welcome.html";
        //         }
        //         else
        //         {
        //             path_ = "/error.html";
        //         }
        //     }
        // }

        ProcessCGI_();
        path_ = "/welcome.html";
    }
}

void HttpRequest::ProcessCGI_()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        LOG_ERROR("pipe() error:%d", 4);
    }

    // 创建子进程
    pid_t pid = fork();

    /**
     * 创建子进程后，父进程继续执行，子进程会从下面的语句开始执行。两个程序都从fork的下一行开始执行
     * 如果pid为0，标识子进程，通常用execl函数执行一个新的程序替换到子进程
     * 新的程序如果成功执行就不会执行子进程接下来的语句了，如果执行失败，继续执行下面的语句，然后通常子进程通过exit退出自身
     */

    if (pid == 0) // 子进程
    {

        close(pipefd[0]); // 关闭读端
        // 将标准输出重定向到管道写端
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // dup2 后可以关闭原 fd

        /**
         * 子进程通过管道和父进程通信，定义了一个管道，
         * 父子进程各有一个副本，子进程关闭读管道并将标准输出重定向到管道写端，父进程关闭写管道并通过read监听
         * 子进程如果一直持有管道写端的引用，并且不显式关闭，父进程将一直阻塞在read方法，但可以使用超时机制
         */

        string authtype = DEFAULT_POST_TAG.find(path_)->second;

        execl("./resources_cgi/auth.cgi", "auth", post_["username"].c_str(), post_["password"].c_str(), authtype.c_str(), NULL);

        std::cout << R"({"status": "409","msg": "cgi run error"})" << std::endl;
        
        // exit(1); // 如果execl失败
        _exit(1); // execl会执行进程的清理工作 _exit直接系统调用关闭进程
    }
    else if (pid > 0) // 父进程
    {
        close(pipefd[1]); // 关闭写端

        char buffer[1024];
        std::string output;
        ssize_t bytesRead;

        // 从管道读取子进程的 stdout
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0'; // 添加字符串结束符
            output += buffer;
        }

        close(pipefd[0]); // 关闭读端

        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            LOG_DEBUG("CGI programe execute success:%s", output);
        }
        else
        {
            LOG_ERROR("CGI programe execute error:%d", WEXITSTATUS(status));
        }

        retjson_ = output;
    }
    else
    {
        LOG_ERROR("fork() error:%d", 3);
    }
}

// 从Url中解析编码
void HttpRequest::ParseFromUrlencoded_()
{
    if (body_.size() == 0)
    {
        return;
    }

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for (; i < n; i++)
    {
        char ch = body_[i];
        switch (ch)
        {
        case '=':
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '%':
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if (post_.count(key) == 0 && j < i)
    {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

// bool HttpRequest::UserVerify(const string &name, const string &pwd, bool isLogin) {
//     if(name == "" || pwd == "") { return false; }
//     LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
//     MYSQL* sql;
//     SqlConnRAII(&sql,  SqlConnPool::Instance());
//     assert(sql);

//     bool flag = false;
//     unsigned int j = 0;
//     char order[256] = { 0 };
//     MYSQL_FIELD *fields = nullptr;
//     MYSQL_RES *res = nullptr;

//     if(!isLogin) { flag = true; }
//     /* 查询用户及密码 */
//     snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
//     LOG_DEBUG("%s", order);

//     if(mysql_query(sql, order)) {
//         mysql_free_result(res);
//         return false;
//     }
//     res = mysql_store_result(sql);
//     j = mysql_num_fields(res);
//     fields = mysql_fetch_fields(res);

//     while(MYSQL_ROW row = mysql_fetch_row(res)) {
//         LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
//         string password(row[1]);
//         /* 注册行为 且 用户名未被使用*/
//         if(isLogin) {
//             if(pwd == password) { flag = true; }
//             else {
//                 flag = false;
//                 LOG_DEBUG("pwd error!");
//             }
//         }
//         else {
//             flag = false;
//             LOG_DEBUG("user used!");
//         }
//     }
//     mysql_free_result(res);

//     /* 注册行为 且 用户名未被使用*/
//     if(!isLogin && flag == true) {
//         LOG_DEBUG("regirster!");
//         bzero(order, 256);
//         snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
//         LOG_DEBUG( "%s", order);
//         if(mysql_query(sql, order)) {
//             LOG_DEBUG( "Insert error!");
//             flag = false;
//         }
//         flag = true;
//     }
//     SqlConnPool::Instance()->FreeConn(sql);
//     LOG_DEBUG( "UserVerify success!!");
//     return flag;
// }

std::string &HttpRequest::retjson()
{
    return retjson_;
}

std::string HttpRequest::path() const
{
    return path_;
}

std::string &HttpRequest::path()
{
    return path_;
}
std::string HttpRequest::method() const
{
    return method_;
}

std::string HttpRequest::version() const
{
    return version_;
}

std::string HttpRequest::GetPost(const std::string &key) const
{
    assert(key != "");
    if (post_.count(key) == 1)
    {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char *key) const
{
    assert(key != nullptr);
    if (post_.count(key) == 1)
    {
        return post_.find(key)->second;
    }
    return "";
}