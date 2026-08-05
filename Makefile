NAME = webserv
CC = c++
FLAGS = -Wall -Wextra -Werror
SRC = src/main.cpp src/EventLoop.cpp src/Server.cpp #src/Config.cpp
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