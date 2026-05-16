#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include <string>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpParser {
public:
    enum State { METHOD, PATH, VERSION, HEADER, BODY, DONE };

    HttpParser() : state_(METHOD) {}

    bool parse(const char* data, size_t len, HttpRequest& req) {
        buffer_.append(data, len);
        size_t pos;
        while (state_ != DONE) {
            switch (state_) {
            case METHOD:
                pos = buffer_.find(' ');
                if (pos == std::string::npos) return false;
                req.method = buffer_.substr(0, pos);
                buffer_.erase(0, pos + 1);
                state_ = PATH;
                break;
            case PATH:
                pos = buffer_.find(' ');
                if (pos == std::string::npos) return false;
                req.path = buffer_.substr(0, pos);
                buffer_.erase(0, pos + 1);
                state_ = VERSION;
                break;
            case VERSION:
                pos = buffer_.find("\r\n");
                if (pos == std::string::npos) return false;
                req.version = buffer_.substr(0, pos);
                buffer_.erase(0, pos + 2);
                state_ = HEADER;
                break;
            case HEADER:
                if (buffer_.substr(0, 2) == "\r\n") {
                    buffer_.erase(0, 2);
                    if (req.method == "POST") state_ = BODY;
                    else state_ = DONE;
                    break;
                }
                pos = buffer_.find("\r\n");
                if (pos == std::string::npos) return false;
                {
                    std::string line = buffer_.substr(0, pos);
                    buffer_.erase(0, pos + 2);
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string key = line.substr(0, colon);
                        std::string value = line.substr(colon + 1);
                        if (!value.empty() && value[0] == ' ') value.erase(0, 1);
                        req.headers[key] = value;
                    }
                }
                break;
            case BODY:
                {
                    auto it = req.headers.find("Content-Length");
                    if (it == req.headers.end()) { state_ = DONE; break; }
                    int bodyLen = std::stoi(it->second);
                    if (buffer_.size() < static_cast<size_t>(bodyLen)) return false;
                    req.body = buffer_.substr(0, bodyLen);
                    buffer_.erase(0, bodyLen);
                    state_ = DONE;
                }
                break;
            case DONE:
                break;
            }
        }
        return true;
    }

    void reset() {
        state_ = METHOD;
        buffer_.clear();
    }

private:
    State state_;
    std::string buffer_;
};

#endif
