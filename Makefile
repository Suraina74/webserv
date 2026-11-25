NAME = webserv
CC = c++
FLAGS = -Wall -Wextra -Werror -g -std=c++11

SRC = src/main.cpp
SRC += src/serverProcess/ServerInfo.cpp
SRC += src/serverProcess/eventLoop.cpp
SRC += src/serverProcess/Client.cpp
SRC += src/serverProcess/router.cpp
SRC += src/serverProcess/routerUtils.cpp
SRC += src/serverProcess/CGI.cpp
SRC += src/HttpParser/Request.cpp
SRC += src/HttpParser/parseRL.cpp
SRC += src/HttpParser/parseHeaders.cpp
SRC += src/HttpParser/parseBody.cpp
SRC += src/HttpParser/fileOperations.cpp
SRC += src/HttpParser/Response.cpp
SRC += src/configParser/Config.cpp
SRC += src/configParser/ConfigLocationAttri.cpp
SRC += src/configParser/Location.cpp
SRC += src/configParser/ConfigUtils.cpp
SRC += src/configParser/ServerConfig.cpp
SRC += src/configParser/ConfigServerAttri.cpp

OBJ = $(SRC:%.cpp=objs/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

objs/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) -rf $(NAME) objs
	@if [ -d LogDir ]; then find LogDir -type f -name '*.log' -delete; fi
	@if [ -d LogDir ]; then find LogDir -type f -name  '*.data' -delete; fi
	@if [ -d LogDir ]; then find LogDir -type f -name  '*.expected' -delete; fi
	@if [ -d LogDir ]; then find LogDir -type f -name  '*.conf' -delete; fi

re: fclean all

.PHONY: all clean fclean re
