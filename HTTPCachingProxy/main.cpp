#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <vector>
#include <mutex>
#include <thread>
#include <signal.h>

#include "utils.h"
#include "response.h"
#include "request.h"
#include "logger.h"
#include "proxyexceptions.h"
#include "cache.h"

std::mutex log_mtx;
std::mutex cache_mtx;

void action(int cfd, int id_count, Cache * cache){
    try{


        Logger logger;

        int yes = 1;
        int st = setsockopt(cfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(st < 0){
            throw MyException("setsockopt error");
        }
        //clie_addr_len = sizeof(clie_addr);

        std::string client_ip = getClientIp(cfd);

        Request request(cfd);

        std::string request_str = request.getRequestString();

        std::cout << "DEBUG: receive request: \n" << request_str << std::endl;
        std::string host = request.getHost();
        std::string port = request.getPort();
        std::string method = request.getMethod();
        request.setRequestId(id_count);

        log_mtx.lock();
        logger.logRequest(request, client_ip);
        log_mtx.unlock();

        int fd;
        int status;
        struct addrinfo hints;
        struct addrinfo *addrlist, *rm_it;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        status = getaddrinfo(host.c_str(), port.c_str(), &hints, &addrlist);
        if(status != 0){
            throw URLErrorException();
        }

        for (rm_it = addrlist; rm_it != NULL; rm_it = rm_it->ai_next) {
            fd = socket(rm_it->ai_family, rm_it->ai_socktype, rm_it->ai_protocol);
            if (fd == -1) {
                continue;
            }

            int yes = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                perror("setsockopt:");
                //return -1;
            }

            if (connect(fd, rm_it->ai_addr, rm_it->ai_addrlen) != -1) {
                break;
            }
            close(fd);
        }

        //Response response(fd);
        if (method == "GET") {

            std::string msg;

            cache_mtx.lock();
            cacheAction act = cache->receiveRequest(request, msg);
            cache_mtx.unlock();

            if(act == RE_VALID){
                request_str = request.revise_request_str();
            }
            else if(act == IN_CACHE){
                

                Response response_cache = cache->getCachedResponse(request);
                response_cache.sendCacheContent(cfd);

                std::string in_cache_log_respond = msg + response_cache.getLogResponding(request) + "\n";

                log_mtx.lock();
                logger.addContent(in_cache_log_respond);
                logger.writeContent();
                log_mtx.unlock();

                close(fd);
                close(cfd);
                return;
            }
            if(act == NOT_IN_CACHE){
                request_str = request.revise_request_str_no_cache();
            }
            send_message(request_str.c_str(), strlen(request_str.c_str()), fd);
            msg += request.getLog() + "\n";


            Response response(fd);
            response.setId(id_count);
            msg += response.getLog(request) + "\n";
            if(response.getStatusCode() == "304"){
                Response res_curr = cache->getCachedResponse(request);
                res_curr.sendClient(cfd, fd);
                msg += res_curr.getLogResponding(request) + "\n";


                log_mtx.lock();
                logger.addContent(msg);
                logger.writeContent();
                log_mtx.unlock();

                close(fd);
                close(cfd);
                return;
            }
            response.sendClient(cfd, fd);
            msg += response.getLogResponding(request) + "\n";

            cache_mtx.lock();
            msg += cache->receiveResponse(request, response);
            cache_mtx.unlock();

            log_mtx.lock();
            logger.addContent(msg);
            logger.writeContent();
            log_mtx.unlock();

            close(fd);
            close(cfd);
        }
        else if(method == "POST"){

            //for test
//            return;

            std::string body = request.getBody(cfd);
            std::string log_msg = request.getLog() + "\n";
            std::string msg = request_str + body;
            send_message(msg.c_str(), strlen(msg.c_str()), fd);
            Response response(fd);

            log_msg += response.getLog(request) + "\n";
            response.sendClient(cfd, fd);

            log_msg += response.getLogResponding(request) + "\n";

            log_mtx.lock();
            logger.addContent(log_msg);
            logger.writeContent();
            log_mtx.unlock();

            close(fd);
            close(cfd);
        }
        else if (method == "CONNECT"){
            std::string message = "HTTP/1.1 200 OK\r\n\r\n";
            status = send(cfd, message.c_str(), message.size(), 0);
            if(status ==-1){
                throw MyException("send error in CONNECT");
            }
            fd_set socket_set;
            int max_fd = std::max(fd, cfd);

            FD_ZERO(&socket_set);
            FD_SET(cfd, &socket_set);
            FD_SET(fd, &socket_set);

            fd_set reset = socket_set;

            while(true){
                int connect_status = 0;
                socket_set = reset;
                int n = select(max_fd + 1, &socket_set, NULL, NULL, NULL);
                if (n < 0) {
                    throw ConnectionErrorException();
                }
                if (FD_ISSET(fd, &socket_set)) {
                    connect_status = pipe_communicate(fd, cfd);
                    
                }
                if (FD_ISSET(cfd, &socket_set)) {
                    //check the pipe_communication status????? and return????
                    connect_status = pipe_communicate(cfd, fd);
                    
                }

                if (connect_status < 0){
                    throw ConnectionErrorException();
                }

                if(connect_status == 0){

                    std::string connect_msg = request.connectLog();
                    log_mtx.lock();
                    logger.addContent(connect_msg);
                    logger.writeContent();
                    log_mtx.unlock();

                    
                    close(fd);
                    close(cfd);
                    break;
                }
            }
        }
    }
    catch (URLErrorException & e){
        std::cout<<"ERROR: "<<e.what()<<std::endl;
        const char * BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send_message(BAD_REQUEST, strlen(BAD_REQUEST), cfd);
        close(cfd);
    }
    catch (ConnectionErrorException & e){
        std::cout<<"ERROR: "<<e.what()<<std::endl;
        close(cfd);
    }
    catch (CacheErrorException & e){
        std::cout<<"ERROR: "<<e.what()<<std::endl;
        close(cfd);
    }
    catch  (CorruptRequestException & e){
        const char * BAD_GATEWAY = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
        send_message(BAD_GATEWAY, strlen(BAD_GATEWAY), cfd);
        close(cfd);
    }
    catch(MyException& e){
      std::cout<<e.what()<<std::endl;
    }
    /*catch(...){
        pthread_t t =pthread_self();
        std::cout<<(size_t)t<<" exit\n";
    }
    pthread_t t =pthread_self();
    std::cout<<(size_t)t<<" exit\n";*/
};


int main() {
    signal(SIGPIPE, SIG_IGN);
    int id_count = 0;
    struct sockaddr_in serv_addr;
    int lfd, cfd;
    char buf[BUFSIZ];
    memset(buf, '0', sizeof(buf));

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12345);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int b = bind(lfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(lfd, 500);

    Cache cache;

    while (1) {
//        try{
            struct sockaddr_in clie_addr;
            socklen_t clie_addr_len;
            int cfd = accept(lfd, (struct sockaddr *) &clie_addr, &clie_addr_len);
//            try{
                 //action(cfd, id_count, &cache);
                std::thread thread(action, cfd, id_count, &cache);
                thread.detach();
                id_count++;
//            }
//            catch(...){
//                continue;
//                std::cout<<"create new thread error"<<std::endl;
//            }
//        }
//        catch (...){
//            continue;
//        }
    }
    return EXIT_SUCCESS;
}
