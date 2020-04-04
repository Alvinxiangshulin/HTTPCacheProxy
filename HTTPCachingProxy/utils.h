

#ifndef INC_568_HW2_GIVEUP_UTILS_H
#define INC_568_HW2_GIVEUP_UTILS_H

void send_message(const char * buffer, size_t len, int fd);

int pipe_communicate(int source_fd, int destination_fd);

std::string getClientIp(int cfd);
#endif //INC_568_HW2_GIVEUP_UTILS_H
