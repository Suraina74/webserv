NAME = webserv
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++11
SRC = src/main.cpp src/Config.cpp src/ConfigUtils.cpp src/ServerConfig.cpp src/ConfigServerAttri.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME) : $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) -rf $(NAME)

re: fclean all

.PHONY: clean fclean re