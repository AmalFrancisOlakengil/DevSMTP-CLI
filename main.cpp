#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Include your custom configuration manager
#include "config.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define closesocket close
#endif

void sslSendAndReceive(SSL* ssl, const std::string& command) {
    if (!command.empty()) {
        SSL_write(ssl, command.c_str(), command.length());
    }
    char buffer[4096] = {0};
    SSL_read(ssl, buffer, sizeof(buffer) - 1);
}

std::string base64Encode(const std::string& in) {
    static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(lookup[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string extractFileLines(const std::string& filename, int startLine, int endLine) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "[Error: Could not open file " + filename + "]\n";
    }

    std::string line;
    std::string codeSnippet = "----- CODE SNIPPET (" + filename + ") -----\n";
    int currentLine = 1;

    while (std::getline(file, line)) {
        if (currentLine >= startLine && currentLine <= endLine) {
            codeSnippet += std::to_string(currentLine) + ": " + line + "\n";
        }
        if (currentLine > endLine) break;
        currentLine++;
    }
    codeSnippet += "-----------------------------------------\n";
    return codeSnippet;
}

int main(int argc, char* argv[]) {
    // 1. Resolve localized developer credentials using config.h
    UserConfig user = loadOrInitializeConfig();

    // 2. Streamlined Argument Validation
    if (argc < 5) {
        std::cerr << "Usage: send_mail <filename> <recipient_email> <start_line> <end_line>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string toEmail = argv[2];
    int startLine = std::stoi(argv[3]);
    int endLine = std::stoi(argv[4]);

    // 3. Extract target snippet block
    std::string codeSnippet = extractFileLines(filename, startLine, endLine);
    
    std::string subject, userBody;
    std::cout << "=== Developer Context Mailer ===\n";
    std::cout << "Subject: "; std::getline(std::cin, subject);
    std::cout << "Issue Description (Body): "; std::getline(std::cin, userBody);

    std::string finalBody = userBody + "\n\n" + codeSnippet;

    std::string smtpServer = "smtp.gmail.com"; 
    std::string port = "465"; 

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // 4. Secure OpenSSL Context Setup
    SSL_library_init();
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    
    struct addrinfo hints, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(smtpServer.c_str(), port.c_str(), &hints, &res);
    SOCKET serverSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(serverSock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, serverSock);
    if (SSL_connect(ssl) <= 0) {
        std::cerr << "SSL Connection failed." << std::endl;
        return 1;
    }

    // 5. Active Secured SMTP State Machine Execution
    sslSendAndReceive(ssl, ""); 
    sslSendAndReceive(ssl, "EHLO localhost\r\n");
    sslSendAndReceive(ssl, "AUTH LOGIN\r\n");
    sslSendAndReceive(ssl, base64Encode(user.email) + "\r\n");
    sslSendAndReceive(ssl, base64Encode(user.appPassword) + "\r\n");
    sslSendAndReceive(ssl, "MAIL FROM:<" + user.email + ">\r\n");
    sslSendAndReceive(ssl, "RCPT TO:<" + toEmail + ">\r\n");
    sslSendAndReceive(ssl, "DATA\r\n");

    std::string emailPayload = 
        "From: " + user.email + "\r\n" +
        "To: " + toEmail + "\r\n" +
        "Subject: [Code Issue] " + subject + "\r\n\r\n" + 
        finalBody + "\r\n.\r\n";

    sslSendAndReceive(ssl, emailPayload);
    sslSendAndReceive(ssl, "QUIT\r\n");

    std::cout << "\n Issue flagged! Contextual email sent successfully to " << toEmail << ".\n";

    // 6. Resource Disposal
    SSL_free(ssl);
    closesocket(serverSock);
    SSL_CTX_free(ctx);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}