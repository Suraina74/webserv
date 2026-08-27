# NAME = webserv
# CC = c++
# FLAGS = -Wall -Wextra -Werror -std=c++11
# SRC =	main.cpp Server.cpp EventLoop.cpp \
# 		configParser/Config.cpp configParser/ConfigUtils.cpp \
# 		configParser/ServerConfig.cpp configParser/ConfigServerAttri.cpp

# OBJ = $(SRC:%.cpp=objs/%.o)

# all: $(NAME)

# $(NAME) : $(OBJ)
# 	$(CC) $(FLAGS) $(OBJ) -o $(NAME) || $(MAKE) fclean

# objs/%.o: %.cpp
# 	@mkdir -p $(dir $@)
# 	$(CC) $(FLAGS) -c $< -o $@ || $(MAKE) fclean

# clean:
# 	$(RM) $(OBJ)

# fclean: clean
# 	$(RM) -rf $(NAME) objs

# re: fclean all

# .PHONY: clean fclean re

NAME = webserv
CC = c++
FLAGS = -Wall -Wextra -Werror -g -std=c++11

SRC =	src/main.cpp \
		src/Request.cpp \
		src/Response.cpp \
		src/EventLoop.cpp \
		src/configParser/Config.cpp \
		src/configParser/ConfigLocationAttri.cpp \
		src/configParser/Location.cpp \
		src/configParser/ConfigUtils.cpp \
		src/configParser/ServerConfig.cpp \
		src/configParser/ConfigServerAttri.cpp \

OBJ = $(SRC:%.cpp=objs/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME) || { $(MAKE) fclean; exit 1; }

objs/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@ || { $(MAKE) fclean; exit 1; }

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) -rf $(NAME) objs

re: fclean all

.PHONY: all clean fclean re